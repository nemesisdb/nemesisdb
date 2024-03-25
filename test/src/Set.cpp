#include "useful/TestCommon.h"

using namespace nemesis::test;




TEST_F(NemesisTest, SetVarious)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData {  .request = R"({ "KV_SET":{ "keys":{"myarray":["a", "b"], "mystring":"string", "myinteger":5, "mydecimal":5.5, "myobject":{"user":"toad"}} }})"_json,
											.expected = {R"({ "KV_SET_RSP":{ "st":1 }})"_json}});
	
}


TEST_F(NemesisTest, Overwrite)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData {  .request = R"({ "KV_SET":{ "keys":{"myarray":["a", "b"], "mystring":"string", "myinteger":5, "mydecimal":5.5, "myobject":{"user":"toad"}} }})"_json,
											.expected = {R"({ "KV_SET_RSP":{ "st":1 }})"_json}});

	tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["myarray", "myobject"]} })"_json,
											.expected = {R"({ "KV_GET_RSP":{ "st":1, "keys":{"myarray":["a", "b"], "myobject":{"user":"toad"}} }})"_json}});

	tc.test(TestData {  .request = R"({ "KV_SET":{ "keys":{"myarray":["a", "b", "c"], "mystring":"string2", "myinteger":10, "mydecimal":5.521, "myobject":{"user":"toad2"}} }})"_json,
											.expected = {R"({ "KV_SET_RSP":{ "st":1 }})"_json}});

	tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["myarray", "myobject", "mystring", "mydecimal", "dontExist"]} })"_json,
											.expected = {R"({ "KV_GET_RSP":{ "st":1, "keys":{"myarray":["a", "b", "c"], "myobject":{"user":"toad2"}, "mydecimal":5.521, "mystring":"string2"} }})"_json}});

}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
