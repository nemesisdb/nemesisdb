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
	
	tc.test({TestData { .request = R"({ "KV_SET":{"string1":"billybob"}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"string1" } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_SET":{"anotherkey":"billybob"}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"anotherkey" } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_COUNT":{} })"_json,	.expected = {R"({ "KV_COUNT_RSP":{ "st":1, "cnt":2 } })"_json} }});
}


TEST_F(FusionTest, IncorrectCommandType)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData { .request = R"({ "KV_COUNT":[] })"_json,	.expected = {R"({ "KV_COUNT_RSP":{ "st":12, "k":"" } })"_json} });
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
