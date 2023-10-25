#include "useful/TestCommon.h"

using namespace nemesis::test;



TEST_F(NemesisTest, GetScalar)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData {  .request = R"({ "KV_SET":{ "keys":{"myarray":["a", "b"], "mystring":"string", "myinteger":5, "mydecimal":5.5, "myobject":{"user":"toad"}} }})"_json,
											.expected = {R"({ "KV_SET_RSP":{ "keys":{"myarray":20, "mystring":20, "myinteger":20, "mydecimal":20, "myobject":20} }})"_json}});
	
	tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["myinteger"]} })"_json,
											.expected = {R"({ "KV_GET_RSP":{ "keys":{"myinteger":5} }})"_json}});
}


TEST_F(NemesisTest, GetStructured)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData {  .request = R"({ "KV_SET":{ "keys":{"myarray":["a", "b"], "mystring":"string", "myinteger":5, "mydecimal":5.5, "myobject":{"user":"toad"}} }})"_json,
											.expected = {R"({ "KV_SET_RSP":{ "keys":{"myarray":20, "mystring":20, "myinteger":20, "mydecimal":20, "myobject":20} }})"_json}});
	
	tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["myarray", "myobject"]} })"_json,
											.expected = {R"({ "KV_GET_RSP":{ "keys":{"myarray":["a", "b"], "myobject":{"user":"toad"}} }})"_json}});
}



TEST_F(NemesisTest, KeyInvalidType)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData {  .request = R"({ "KV_SET":{ "keys":{"myarray":["a", "b"], "mystring":"string", "myinteger":5, "mydecimal":5.5, "myobject":{"user":"toad"}} }})"_json,
											.expected = {R"({ "KV_SET_RSP":{ "keys":{"myarray":20, "mystring":20, "myinteger":20, "mydecimal":20, "myobject":20} }})"_json}});

	tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["myarray", 5]} })"_json,
											.expected = {R"({ "KV_GET_RSP":{ "keys":{"myarray":["a", "b"]} }})"_json}});
}


TEST_F(NemesisTest, KeyNotExist)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["dontexist"]} })"_json,
											.expected = {R"({ "KV_GET_RSP":{ "keys":{"dontexist":null} }})"_json}});
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}