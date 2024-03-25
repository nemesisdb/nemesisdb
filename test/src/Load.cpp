#include <filesystem>
#include <fstream>
#include "useful/TestCommon.h"

// useful/TestCommon.h defines NDB_UNIT_TEST which changes normal behaviour:
//	- no monitor
//  - no "duration" in SH_SAVE_RSP

using namespace nemesis::test;

namespace fs = std::filesystem;

SessionToken startupLoadToken ;


void setData (TestClient& tc)
{
	tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"key1":"v1", "key2":"v2"}}})"_json,
                      .expected = {R"({ "KV_SET_RSP":{ "st":1 } })"_json} }});
}



TEST_F(NemesisTest, NameInvalid)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	{
		TestData td {MakeTestData(R"({ "SH_LOAD":{} })"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["SH_LOAD_RSP"]["st"] == RequestStatus::ParamMissing); 
	}

	{
		TestData td {MakeTestData(R"({ "SH_LOAD":{ "name":2} })"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["SH_LOAD_RSP"]["st"] == RequestStatus::ValueTypeInvalid); 
	}

	{
		TestData td {MakeTestData(R"({ "SH_LOAD":{ "name":"_idontexist"} })"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["SH_LOAD_RSP"]["st"] == RequestStatus::LoadError); 
	}
}


TEST_F(NemesisTestSaveEnable, Data)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
  SessionToken tkn = tc.token["tkn"];

	setData(tc);
	
	// save data
	{
		auto td = MakeTestData(R"({ "SH_SAVE":{"name":"Data"} })"_json, 2);
		tc.test(td);
		
		ASSERT_EQ(td.actual[0]["SH_SAVE_RSP"]["name"], "Data");
		ASSERT_EQ(td.actual[0]["SH_SAVE_RSP"]["st"], 120); // save start

		ASSERT_EQ(td.actual[1]["SH_SAVE_RSP"]["name"], "Data");
		ASSERT_EQ(td.actual[1]["SH_SAVE_RSP"]["st"], 121); // save complete

	}

  // delete session and re-load (easier than restarting server)
	{
		testjson end;
		end["SH_END"]["tkn"] = tkn;
		
		tc.test({TestData { .request = end,
												.expected = {R"({ "SH_END_RSP":{ "st":1 } })"_json} }});
	}
	
	// load and check
	{
		tc.test({TestData { .request = R"({ "SH_LOAD":{"name":"Data"} })"_json,
												.expected = {R"({ "SH_LOAD_RSP":{ "name":"Data", "st":141, "sessions":1, "keys":2 } })"_json} }});


		tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["key1", "key2"]} })"_json,
												.expected = {R"({ "KV_GET_RSP":{ "st":1, "keys":{"key1":"v1", "key2":"v2" } }})"_json}});
	}
	
}


TEST_F(NemesisTestSaveEnable, PrepareStartupLoad)
{
	// This saves data for StartupLoad test
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
  startupLoadToken = tc.token["tkn"];	// grab the token for StartupLoad test

	setData(tc);

	const testjson save  = {{"SH_SAVE", {{"name", LoadName}}}};

	auto td = MakeTestData(save , 2);
	tc.test(td);

	ASSERT_EQ(td.actual[0]["SH_SAVE_RSP"]["name"], LoadName);
	ASSERT_EQ(td.actual[0]["SH_SAVE_RSP"]["st"], 120); // save start

	ASSERT_EQ(td.actual[1]["SH_SAVE_RSP"]["name"], LoadName);
	ASSERT_EQ(td.actual[1]["SH_SAVE_RSP"]["st"], 121); // save complete

	// now we have data to load on startup in StartupLoad test
}


TEST_F(NemesisTestLoadOnStartup, StartupLoad)
{
	// NOTE this must run after PrepareStartupLoad
	TestClient tc;

	ASSERT_TRUE(tc.openNoSession());

	tc.test(TestData { 	.request = R"({ "SH_INFO_ALL":{} })"_json,
											.expected = {R"({ "SH_INFO_ALL_RSP":{ "st":1, "totalSessions":1, "totalKeys":2 } })"_json}});

	tc.token["tkn"] = startupLoadToken;

	tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["key1", "key2"]} })"_json,
											.expected = {R"({ "KV_GET_RSP":{ "st":1, "keys":{"key1":"v1", "key2":"v2" } }})"_json}});
}



int main (int argc, char ** argv)
{
	auto workingDir = fs::path{argv[0]}.parent_path();

	fs::remove_all(workingDir / "data");
  fs::create_directory(workingDir / "data");

	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
