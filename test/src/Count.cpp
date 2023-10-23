#include "useful/TestCommon.h"

using namespace fusion::test;


TEST_F(FusionTest, NoData)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_COUNT":{} })"_json,	.expected = {R"({ "KV_COUNT_RSP":{ "st":1, "cnt":0 } })"_json} }});
}


TEST_F(FusionTest, Data)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"asda":"a", "tesco":"b"}}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"asda":20, "tesco":20} } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_COUNT":{} })"_json,	.expected = {R"({ "KV_COUNT_RSP":{ "st":1, "cnt":2 } })"_json} }});
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
