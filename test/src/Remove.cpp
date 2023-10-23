#include "useful/TestCommon.h"

using namespace nemesis::test;


TEST_F(NemesisTest, Remove)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData {  .request = R"({ "KV_SET":{ "keys":{"string1":"asda", "string2":"tesco"} }})"_json,
											.expected = {R"({ "KV_SET_RSP":{ "keys":{"string1":20, "string2":20} }})"_json}});

	tc.test({TestData { .request = R"({ "KV_RMV":{"keys":["string1"] }})"_json,
											.expected = {R"({ "KV_RMV_RSP":{ "string1":24 } })"_json} }});
}


TEST_F(NemesisTest, KeyNotExist)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test({TestData { .request = R"({ "KV_RMV":{"keys":["stringNotHere"] }})"_json,	.expected = {R"({ "KV_RMV_RSP":{ "stringNotHere":22 } })"_json} }});
}




int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
