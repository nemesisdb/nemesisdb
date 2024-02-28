#include "useful/TestCommon.h"

using namespace nemesis::test;

static const auto set = R"({
														"KV_SET":
														{
															"keys":
															{
																"loginsValid":
																[
																	{"timestamp":1234, "location":"London"},
																	{"timestamp":1235, "location":"London"},
																	{"timestamp":1236, "location":"Paris"},
																	{"timestamp":1238, "location":"London"}
																],
																"loginsFailed":
																[
																	{"timestamp":1235, "location":"New York"},
																	{"timestamp":1235, "location":"London"},
																	{"timestamp":1237, "location":"New York"}
																]
															}
														}
													})"_json;


// Error cases

TEST_F(NemesisTest, NoPathRspKeys)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "a":"a", "b":"b", "c":"c" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":26, "m":"Missing parameter" } })"_json} }});
}


TEST_F(NemesisTest, NoRspInvalid)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "keys":["a"], "path":"$", "rsp":"duff" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":13, "m":"'rsp' invalid value" } })"_json} }});
}


TEST_F(NemesisTest, EmptyPath)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "keys":["a"], "path":"", "rsp":"kv" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":42, "m":"'path' is empty" } })"_json} }});
}


TEST_F(NemesisTest, PathNotString)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "keys":["a"], "path":2, "rsp":"kv" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":41, "m":"Param type incorrect" } })"_json} }});
}


TEST_F(NemesisTest, KeysNotArray)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "keys":"a", "path":"a", "rsp":"kv" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":41, "m":"Param type incorrect" } })"_json} }});
}


// TEST_F(NemesisTest, PathInvalid)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open()); 

// 	//note the space at the end of the path value to avoid ending the raw literal 
// 	tc.test({TestData { .request = R"({ "KV_FIND":{ "keys":["a"], "path":"$.(@ == 'a') ", "rsp":"kv" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":4, "m":"" } })"_json} }});
// }


//Happy cases

TEST_F(NemesisTest, NoData)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"$[?(@.location == 'London')]", "rsp":"kv" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "keys":{} } })"_json} }});
}


TEST_F(NemesisTest, KvOneResult)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
 	tc.test(TestData { .request = set,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"loginsValid":20, "loginsFailed":20} } })"_json} });
	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"$[?(@.location == 'London')]", "rsp":"kv", "keys":["loginsValid"] } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "keys":{"loginsValid":[{"timestamp":1234, "location":"London"}, {"timestamp":1235, "location":"London"}, {"timestamp":1236, "location":"Paris"},{"timestamp":1238, "location":"London"}]} } })"_json} }});
}


TEST_F(NemesisTest, KeysOneResult)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
 	tc.test(TestData { .request = set,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"loginsValid":20, "loginsFailed":20} } })"_json} });
	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"$[?(@.location == 'London')]", "rsp":"keys", "keys":["loginsValid"] } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "keys":["loginsValid"] } })"_json} }});
}


TEST_F(NemesisTest, PathsOneResult)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
 	tc.test(TestData { .request = set,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"loginsValid":20, "loginsFailed":20} } })"_json} });
	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"$[?(@.location == 'London')]", "rsp":"paths", "keys":["loginsValid"] } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "paths":["$[0]", "$[1]", "$[3]"] } })"_json} }});
}


TEST_F(NemesisTest, KeysTwoResult)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
 	tc.test(TestData { .request = set,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"loginsValid":20, "loginsFailed":20} } })"_json} });
	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"$[?(@.location == 'London')]", "rsp":"keys", "keys":["loginsValid", "loginsFailed"] } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "keys":["loginsValid", "loginsFailed"] } })"_json} }});
}



int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}

