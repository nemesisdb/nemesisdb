#include "useful/TestCommon.h"

using namespace nemesis::test;



TEST_F(NemesisTest, GetScalar)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData {  .request = R"({ "KV_SET":{ "keys":{"myarray":["a", "b"], "mystring":"string", "myinteger":5, "mydecimal":5.5, "myobject":{"user":"toad"}} }})"_json,
											.expected = {R"({ "KV_SET_RSP":{ "st":1 }})"_json}});
	
	tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["myinteger"]} })"_json,
											.expected = {R"({ "KV_GET_RSP":{ "st":1, "keys":{"myinteger":5} }})"_json}});
}


TEST_F(NemesisTest, GetStructured)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData {  .request = R"({ "KV_SET":{ "keys":{"myarray":["a", "b"], "mystring":"string", "myinteger":5, "mydecimal":5.5, "myobject":{"user":"toad"}} }})"_json,
											.expected = {R"({ "KV_SET_RSP":{ "st":1 }})"_json}});
	
	tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["myarray", "myobject"]} })"_json,
											.expected = {R"({ "KV_GET_RSP":{ "st":1, "keys":{"myarray":["a", "b"], "myobject":{"user":"toad"}} }})"_json}});
}



TEST_F(NemesisTest, KeyInvalidType)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData {  .request = R"({ "KV_SET":{ "keys":{"myarray":["a", "b"], "mystring":"string", "myinteger":5, "mydecimal":5.5, "myobject":{"user":"toad"}} }})"_json,
											.expected = {R"({ "KV_SET_RSP":{ "st":1 }})"_json}});

	tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["myarray", 5]} })"_json,
											.expected = {R"({ "KV_GET_RSP":{ "st":1, "keys":{"myarray":["a", "b"]} }})"_json}});
}


TEST_F(NemesisTest, KeyNotExist)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["dontexist"]} })"_json,
											.expected = {R"({ "KV_GET_RSP":{ "st":1, "keys":{} }})"_json}});
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}