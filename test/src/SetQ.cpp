#include "useful/TestCommon.h"

using namespace nemesis::test;

//
// Limited on what we can do because SETQ doesn't respond unless there's an error.
//


TEST_F(NemesisTest, SetQ)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData {  .request = R"({ "KV_SET":{ "keys":{"myarray":["a", "b"], "mystring":"string", "myinteger":5, "mydecimal":5.5, "myobject":{"user":"toad"}} }})"_json,
											.expected = {R"({ "KV_SET_RSP":{ "keys":{"myarray":20, "mystring":20, "myinteger":20, "mydecimal":20, "myobject":20} }})"_json}});

	tc.test(TestData {.request = R"({ "KV_COUNT":{} })"_json, .expected = {R"({ "KV_COUNT_RSP":{"cnt":5, "st":1} } )"_json} });
}



int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
