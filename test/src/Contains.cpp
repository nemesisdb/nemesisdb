#include "useful/TestCommon.h"

using namespace nemesis::test;


TEST_F(NemesisTest, NoResult)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_CONTAINS":{"keys":["somekey"]} })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "contains":[], "st":1 } })"_json} }});
}


TEST_F(NemesisTest, OneResult)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test({TestData { .request = R"({ "KV_CONTAINS":{"keys":["somekey"]} })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "contains":[], "st":1 } })"_json} }});
  tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"somekey":"billybob"}}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":1 } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_CONTAINS":{"keys":["somekey"]} })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "contains":["somekey"], "st":1 } })"_json} }});
}


TEST_F(NemesisTest, MultipleResults)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test({TestData { .request = R"({ "KV_CONTAINS":{"keys":["somekey","anotherkey"]} })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "contains":[], "st":1 } })"_json }}});
  tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"somekey":"tesco", "anotherkey":"asda"}}})"_json,	.expected = {R"({ "KV_SET_RSP":{"st":1} })"_json} }});
	tc.test({TestData { .request = R"({ "KV_CONTAINS":{"keys":["somekey","anotherkey"]} })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "contains":["somekey", "anotherkey"], "st":1 } })"_json }}});
}



int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
