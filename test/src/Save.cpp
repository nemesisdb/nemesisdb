#include <filesystem>
#include "useful/TestCommon.h"

// This tests assumes the save path is "./data".
// It deletes save path on startup and creates it in SavePathNotExist
// It assumes one worker pool (correct way is to get number of pools from md/md.json)

using namespace nemesis::test;

namespace fs = std::filesystem;

static const fs::path DataDir = "data";
static const fs::path MetaDataDir = "md";
static std::size_t NumPools = 1U;


fs::path workingDir, savePath;

fs::path getSubDir (const fs::path& parent)
{
	auto it = fs::directory_iterator{parent};
	return it == fs::directory_iterator{} ? "" : it->path();
}


// TEST_F(NemesisTest, SaveDisabled)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

// 	tc.test({TestData { .request = R"({ "SH_SAVE":{} })"_json,	.expected = {R"({ "SH_SAVE_RSP":{ "st":14, "m":"" } })"_json} }});
// }



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

	tc.test({TestData { .request = R"({ "SH_SAVE":{} })"_json,	.expected = {R"({ "SH_SAVE_RSP":{ "st":13, "m":"" } })"_json} }});
	tc.test({TestData { .request = R"({ "SH_SAVE":{ "name":2} })"_json,	.expected = {R"({ "SH_SAVE_RSP":{ "st":13, "m":"" } })"_json} }});
}


// All Sessions
TEST_F(NemesisTestSaveEnable, NoSessions)
{
	TestClient tc;

	ASSERT_TRUE(tc.openNoSession());

	tc.test({TestData { .request = R"({ "SH_SAVE":{"name":"NoSessions"} })"_json,
											.expected = {R"({ "SH_SAVE_RSP":{ "name":"NoSessions", "st":120 } })"_json} }});

	//ASSERT_TRUE(fs::exists(savePath / "NoSessions"));
	//ASSERT_EQ(countFiles(savePath / "NoSessions"), 1);

	// auto dsRoot = getSubDir(savePath / "NoSessions");

	// ASSERT_EQ(countFiles(dsRoot), 2);	// 'md' and 'data' dirs
	// ASSERT_TRUE(fs::exists(dsRoot / MetaDataDir));
	// ASSERT_TRUE(fs::exists(dsRoot / DataDir));

	// ASSERT_TRUE(fs::exists(dsRoot / MetaDataDir / "md.json"));
	// ASSERT_EQ(countFiles(dsRoot / DataDir), NumPools);

	std::cout << "here\n";
}


// TEST_F(NemesisTestSaveEnable, Data)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"string1":"billybob"}}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"string1":20} } })"_json} }});
// 	tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"anotherkey":"billybob"}}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"anotherkey":20} } })"_json} }});
// 	tc.test({TestData { .request = R"({ "KV_COUNT":{} })"_json,	.expected = {R"({ "KV_COUNT_RSP":{ "st":1, "cnt":2 } })"_json} }});
// 	tc.test({TestData { .request = R"({ "KV_CLEAR":{} })"_json,	.expected = {R"({ "KV_CLEAR_RSP":{ "st":1, "cnt":2 } })"_json} }});
// 	tc.test({TestData { .request = R"({ "KV_COUNT":{} })"_json,	.expected = {R"({ "KV_COUNT_RSP":{ "st":1, "cnt":0 } })"_json} }});
// }


// Select Sessions






int main (int argc, char ** argv)
{
	workingDir = fs::path{argv[0]}.parent_path();
	savePath = workingDir / "data";

	// a bit hacky: remove "./data" dir before tests start
	fs::remove_all(savePath);

	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
