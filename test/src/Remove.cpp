#include "useful/TestCommon.h"

using namespace fusion::test;


TEST_F(FusionTest, Remove)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData {  .request = R"({ "KV_SET":{ "keys":{"string1":"asda"} }})"_json,
											.expected = {R"({ "KV_SET_RSP":{ "keys":{"myarray":20, "mystring":20, "myinteger":20, "mydecimal":20, "myobject":20} }})"_json}});

	tc.test({TestData { .request = R"({ "KV_RMV":{"keys":["string1"]} })"_json,	.expected = {R"({ "KV_RMV_RSP":{ "keys":{"string1":24} } })"_json} }});
}


TEST_F(FusionTest, KeyNotExist)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test({TestData { .request = R"({ "KV_RMV":["stringNotHere"] })"_json,	.expected = {R"({ "KV_RMV_RSP":{ "stringNotHere":22 } })"_json} }});
}




int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
