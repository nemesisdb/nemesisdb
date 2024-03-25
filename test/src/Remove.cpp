#include "useful/TestCommon.h"

using namespace nemesis::test;


TEST_F(NemesisTest, Remove)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData {  .request = R"({ "KV_SET":{ "keys":{"string1":"asda", "string2":"tesco"} }})"_json,
											.expected = {R"({ "KV_SET_RSP":{ "st":1 }})"_json}});

	tc.test({TestData { .request = R"({ "KV_RMV":{"keys":["string1"] }})"_json,
											.expected = {R"({ "KV_RMV_RSP":{ "st":1 } })"_json} }});
}


TEST_F(NemesisTest, KeyNotExist)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test({TestData { .request = R"({ "KV_RMV":{"keys":["stringNotHere"] }})"_json,	.expected = {R"({ "KV_RMV_RSP":{ "st":1 } })"_json} }});
}




int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
