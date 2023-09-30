#include "useful/TestCommon.h"

using namespace fusion::test;


TEST_F(FusionTest, Remove)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_SET":{"string1":"billybob"}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"string1" } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_RMV":["string1"] })"_json,	.expected = {R"({ "KV_RMV_RSP":{ "st":24, "k":"string1" } })"_json} }});
}


TEST_F(FusionTest, KeyNotExist)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test({TestData { .request = R"({ "KV_RMV":["stringNotHere"] })"_json,	.expected = {R"({ "KV_RMV_RSP":{ "st":22, "k":"stringNotHere" } })"_json} }});
}


TEST_F(FusionTest, KeyShort)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_RMV":["str"] })"_json,	.expected = {R"({ "KV_RMV_RSP":{ "st":25, "k":"str" } })"_json} }});
}


TEST_F(FusionTest, IncorrectCommandType)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData { .request = R"({ "KV_RMV":{"a":"b"} })"_json,	.expected = {R"({ "KV_RMV_RSP":{ "st":12, "k":"" } })"_json} });
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
