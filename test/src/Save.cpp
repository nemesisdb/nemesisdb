#include <filesystem>
#include <fstream>
#include "useful/TestCommon.h"

// This tests assumes the save path is "./data".
// It deletes save path on startup and creates it in SavePathNotExist
// useful/TestCommon.h defines NDB_UNIT_TEST which changes normal behaviour:
//	- no monitor
//  - no "duration" in SH_SAVE_RSP

using namespace nemesis::test;

namespace fs = std::filesystem;

static const fs::path DataDir = "data";
static const fs::path MetaDataDir = "md";

fs::path workingDir, savePath;


fs::path getSubDir (const fs::path& parent)
{
	auto it = fs::directory_iterator{parent};
	return it == fs::directory_iterator{} ? "" : it->path();
}


void setData (TestClient& tc)
{
	tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"key1":"v1", "key2":"v2"}}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":1 } })"_json} }});
}


testjson readSeshFile (const fs::path& f)
{
	if (std::ifstream stream{f}; stream.good())
		return testjson::parse(stream);
	else
		return testjson{};
}


TEST_F(NemesisTest, SaveDisabled)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "SH_SAVE":{} })"_json,	.expected = {R"({ "SH_SAVE_RSP":{ "st":14, "m":"" } })"_json} }});
}


TEST_F(NemesisTestSaveEnablePathNotExist, SavePathNotExist)
{
	// NemesisTestSaveEnablePathNotExist::SetUp() confirms the server fails to start because
	// save path does not exist
	
	// now we can create the data directory for subsequent tests
	fs::create_directory(savePath);
}


TEST_F(NemesisTestSaveEnable, NameInvalid)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	{
		TestData td {MakeTestData(R"({ "SH_SAVE":{} })"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["SH_SAVE_RSP"]["st"] == RequestStatus::ParamMissing); 
	}

	{
		TestData td {MakeTestData(R"({ "SH_SAVE":{ "name":2} })"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["SH_SAVE_RSP"]["st"] == RequestStatus::ValueTypeInvalid); 
	}
}


// All Sessions
TEST_F(NemesisTestSaveEnable, NoSessions)
{
	TestClient tc;

	ASSERT_TRUE(tc.openNoSession());

	TestData td {MakeTestData(R"({ "SH_SAVE":{"name":"NoSessions"} })"_json, 2)};
	tc.test(td);

	ASSERT_TRUE(td.actual[0]["SH_SAVE_RSP"]["st"] == RequestStatus::SaveStart); 
	ASSERT_TRUE(td.actual[1]["SH_SAVE_RSP"]["st"] == RequestStatus::SaveComplete); 

	ASSERT_TRUE(fs::exists(savePath / "NoSessions"));
	ASSERT_EQ(countFiles(savePath / "NoSessions"), 1);

	auto dsRoot = getSubDir(savePath / "NoSessions");

	ASSERT_EQ(countFiles(dsRoot), 2);	// 'md' and 'data' dirs
	ASSERT_TRUE(fs::exists(dsRoot / MetaDataDir));
	ASSERT_TRUE(fs::exists(dsRoot / DataDir));

	ASSERT_TRUE(fs::exists(dsRoot / MetaDataDir / "md.json"));
	ASSERT_EQ(countFiles(dsRoot / DataDir), 0); // no session data files
}


TEST_F(NemesisTestSaveEnable, Data)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	setData(tc);

	TestData td {MakeTestData(R"({ "SH_SAVE":{"name":"Data"} })"_json, 2)};
	tc.test(td);

	ASSERT_TRUE(td.actual[0]["SH_SAVE_RSP"]["st"] == RequestStatus::SaveStart); 
	ASSERT_TRUE(td.actual[1]["SH_SAVE_RSP"]["st"] == RequestStatus::SaveComplete); 

	ASSERT_TRUE(fs::exists(savePath / "Data"));
	ASSERT_EQ(countFiles(savePath / "Data"), 1);

	auto dsRoot = getSubDir(savePath / "Data");
	ASSERT_EQ(countFiles(dsRoot / DataDir), 1); // assumes 1 pool
	
	auto sesh0 = readSeshFile(dsRoot / DataDir / "0");
	
	ASSERT_FALSE(sesh0.is_null());
	ASSERT_TRUE(sesh0.is_array());
	ASSERT_EQ(sesh0.size(), 1);
	
	ASSERT_TRUE(sesh0[0].contains("sh"));
	ASSERT_TRUE(sesh0[0].contains("keys"));

	ASSERT_TRUE(sesh0[0]["sh"].contains("tkn"));
	ASSERT_TRUE(sesh0[0]["sh"].contains("shared"));
	ASSERT_TRUE(sesh0[0]["sh"].contains("expiry"));
	ASSERT_TRUE(sesh0[0]["sh"]["expiry"].is_object());
	ASSERT_TRUE(sesh0[0]["sh"]["expiry"].contains("duration"));
	ASSERT_TRUE(sesh0[0]["sh"]["expiry"].contains("deleteSession"));

	
	ASSERT_TRUE(sesh0[0]["keys"].contains("key1"));
	ASSERT_TRUE(sesh0[0]["keys"].contains("key2"));
}


// This test requires the 'data' directory exists, which is created by the 'Data' test
TEST_F(NemesisTestSaveEnable, TokensInvalid)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	// tkn data type
	{
		TestData td {MakeTestData(R"({ "SH_SAVE":{ "name":"TokensInvalid", "tkns":""} })"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["SH_SAVE_RSP"]["st"] == RequestStatus::ValueTypeInvalid); 
	}

	// empty
	{
		TestData td {MakeTestData(R"({ "SH_SAVE":{ "name":"TokensInvalid", "tkns":[]} })"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["SH_SAVE_RSP"]["st"] == RequestStatus::ValueSize); 
	}

	// item data type
	{
		TestData td {MakeTestData(R"({ "SH_SAVE":{ "name":"TokensInvalid", "tkns":[123, "a"]} })"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["SH_SAVE_RSP"]["st"] == RequestStatus::ValueTypeInvalid); 
	}
}


// Save selected Sessions
TEST_F(NemesisTestSaveEnable, SaveSessions)
{
	// testing framework not well suited to this: each TestClient creates one session
	// so we create two clients here for two sessions
	TestClient tc1, tc2;

	ASSERT_TRUE(tc1.open());
	ASSERT_TRUE(tc2.open());

	SessionToken tkn1 = tc1.token["tkn"];
	SessionToken tkn2 = tc2.token["tkn"];
	
	testjson save = R"({ "SH_SAVE":{"name":"SaveSessions"} })"_json ;
	save["SH_SAVE"]["tkns"] = {tkn1, tkn2};

	setData(tc1);
	setData(tc2);

	TestData td {MakeTestData(R"({ "SH_SAVE":{"name":"SaveSessions"} })"_json, 2)};
	tc1.test(td);

	ASSERT_TRUE(td.actual[0]["SH_SAVE_RSP"]["st"] == RequestStatus::SaveStart); 
	ASSERT_TRUE(td.actual[1]["SH_SAVE_RSP"]["st"] == RequestStatus::SaveComplete); 


	auto dsRoot = getSubDir(savePath / "SaveSessions");
	ASSERT_EQ(countFiles(dsRoot / DataDir), 1);
	
	auto seshData = readSeshFile(dsRoot / DataDir / "0");

	ASSERT_FALSE(seshData.is_null());
	ASSERT_EQ(seshData.size(), 2);	// 2 sessions

	ASSERT_TRUE(seshData[0]["keys"].contains("key1"));
	ASSERT_TRUE(seshData[0]["keys"].contains("key2"));
	ASSERT_TRUE(seshData[1]["keys"].contains("key1"));
	ASSERT_TRUE(seshData[1]["keys"].contains("key2"));
}


int main (int argc, char ** argv)
{
	workingDir = fs::path{argv[0]}.parent_path();
	savePath = workingDir / "data";

	// a bit hacky: remove "./data" dir before tests start
	fs::remove_all(savePath);

	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
