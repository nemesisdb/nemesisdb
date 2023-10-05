#include "useful/TestCommon.h"

using namespace fusion::test;


TEST_F(FusionTest, NoResult)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_CONTAINS":["somekey"] })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "st":22, "k":"somekey" } })"_json} }});
}


TEST_F(FusionTest, OneResult)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test({TestData { .request = R"({ "KV_CONTAINS":["somekey"] })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "st":22, "k":"somekey" } })"_json} }});
  tc.test({TestData { .request = R"({ "KV_SET":{"somekey":"billybob"}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"somekey" } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_CONTAINS":["somekey"] })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "st":23, "k":"somekey" } })"_json} }});
}


TEST_F(FusionTest, MultipleResults)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test({TestData { .request = R"({ "KV_CONTAINS":["somekey", "anotherkey"] })"_json,	.expected = { R"({ "KV_CONTAINS_RSP":{"st":22, "k":"somekey"} })"_json,
                                                                                                      R"({ "KV_CONTAINS_RSP":{"st":22, "k":"anotherkey"} })"_json} }});

  tc.test({TestData { .request = R"({ "KV_SET":{"somekey":"billybob", "anotherkey":"blah"}})"_json,	.expected = { R"({ "KV_SET_RSP":{ "st":20, "k":"somekey" } })"_json,
                                                                                                                    R"({ "KV_SET_RSP":{ "st":20, "k":"anotherkey" } })"_json} }});

	tc.test({TestData { .request = R"({ "KV_CONTAINS":["somekey", "anotherkey"] })"_json,	.expected = { R"({ "KV_CONTAINS_RSP":{ "st":23, "k":"somekey" } })"_json,
                                                                                                      R"({ "KV_CONTAINS_RSP":{ "st":23, "k":"anotherkey" } })"_json} }});
}


TEST_F(FusionTest, ShortKey)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

  tc.test({TestData { .request = R"({ "KV_CONTAINS":["short"] })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "st":25, "k":"short" } })"_json} }});
}


TEST_F(FusionTest, IncorrectCommandType)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData { .request = R"({ "KV_CONTAINS":{"a":"b"} })"_json,	.expected = {R"({ "KV_CONTAINS_RSP":{ "st":12, "k":"" } })"_json} });
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
