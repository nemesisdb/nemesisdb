#include "useful/TestCommon.h"

using namespace nemesis::test;


TEST_F(NemesisTest, NoData)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_KEYS":{} })"_json,	.expected = {R"({ "KV_KEYS_RSP":{ "st":1, "keys":[] } })"_json} }});
}


TEST_F(NemesisTest, Data)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"asda":"a", "tesco":"b"}}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":1 } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_KEYS":{} })"_json,	.expected = {R"({ "KV_KEYS_RSP":{ "st":1, "keys":["asda", "tesco"] } })"_json} }});
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
