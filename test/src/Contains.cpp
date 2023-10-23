#include "useful/TestCommon.h"

using namespace nemesis::test;


TEST_F(NemesisTest, NoResult)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_CONTAINS":{"keys":["somekey"]} })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "keys":{"somekey":false}, "st":1 } })"_json} }});
}


TEST_F(NemesisTest, OneResult)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test({TestData { .request = R"({ "KV_CONTAINS":{"keys":["somekey"]} })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "keys":{"somekey":false}, "st":1 } })"_json} }});
  tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"somekey":"billybob"}}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"somekey":20} } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_CONTAINS":{"keys":["somekey"]} })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "keys":{"somekey":true}, "st":1 } })"_json} }});
}


TEST_F(NemesisTest, MultipleResults)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test({TestData { .request = R"({ "KV_CONTAINS":{"keys":["somekey","anotherkey"]} })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "keys":{"somekey":false, "anotherkey":false}, "st":1 } })"_json }}});
  tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"somekey":"tesco", "anotherkey":"asda"}}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"somekey":20, "anotherkey":20} } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_CONTAINS":{"keys":["somekey","anotherkey"]} })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "keys":{"somekey":true, "anotherkey":true}, "st":1 } })"_json }}});
}



int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
