#include <filesystem>
#include <fstream>
#include "useful/TestCommon.h"

// useful/TestCommon.h defines NDB_UNIT_TEST which changes normal behaviour:
//	- no monitor
//  - no "duration" in SH_SAVE_RSP

using namespace nemesis::test;

namespace fs = std::filesystem;


void setData (TestClient& tc)
{
	tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"key1":"v1", "key2":"v2"}}})"_json,
                      .expected = {R"({ "KV_SET_RSP":{ "keys":{"key1":20, "key2":20} } })"_json} }});
}



TEST_F(NemesisTestSaveEnable, NameInvalid)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "SH_LOAD":{} })"_json,	.expected = {R"({ "SH_LOAD_RSP":{ "st":13, "m":"" } })"_json} }});
	tc.test({TestData { .request = R"({ "SH_LOAD":{ "names":2} })"_json,	.expected = {R"({ "SH_LOAD_RSP":{ "st":13, "m":"" } })"_json} }});
  tc.test({TestData { .request = R"({ "SH_LOAD":{ "names":[2]} })"_json,	.expected = {R"({ "SH_LOAD_RSP":{ "st":142, "m":"name not string, does not exist or empty" } })"_json} }});
}



TEST_F(NemesisTestSaveEnable, Data)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
  SessionToken tkn = tc.token["tkn"];

	setData(tc);

	tc.test({TestData { .request = R"({ "SH_SAVE":{"name":"Data"} })"_json,
											.expected = {R"({ "SH_SAVE_RSP":{ "name":"Data", "st":120 } })"_json,
																	 R"({ "SH_SAVE_RSP":{ "name":"Data", "st":121 } })"_json} }});

  // delete session and re-load (easier than restarting server)
  testjson end;
  end["SH_END"]["tkn"] = tkn;
  
  tc.test({TestData { .request = end,
											.expected = {R"({ "SH_END_RSP":{ "st":1 } })"_json} }});

  // load
	tc.test({TestData { .request = R"({ "SH_LOAD":{"names":["Data"]} })"_json,
											.expected = {R"({ "SH_LOAD_RSP":{ "Data":{"st":141, "sessions":1, "keys":2} } })"_json} }});

  tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["key1", "key2"]} })"_json,
											.expected = {R"({ "KV_GET_RSP":{ "keys":{"key1":"v1", "key2":"v2" } }})"_json}});
}



int main (int argc, char ** argv)
{
	auto workingDir = fs::path{argv[0]}.parent_path();

	fs::remove_all(workingDir / "data");
  fs::create_directory(workingDir / "data");

	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
