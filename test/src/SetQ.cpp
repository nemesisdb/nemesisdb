#include "useful/TestCommon.h"

using namespace fusion::test;

//
// Limited on what we can do because SETQ doesn't respond unless there's an error.
//


TEST_F(FusionTest, SetQ)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_SETQ":{"string1":"asda"}})"_json }},
		{TestData { .request = R"({ "KV_SETQ":{"integer1":1}})"_json }},
		{TestData { .request = R"({ "KV_SETQ":{"decimal1":1.5}})"_json }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);

	tc.test(TestData {.request = R"({ "KV_COUNT":{} })"_json, .expected = {R"({ "KV_COUNT_RSP":{"cnt":3, "st":1} } )"_json} });
}


TEST_F(FusionTest, KeyShort)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_SETQ":{"short":["a", "b"] } })"_json,	.expected = {R"({ "KV_SETQ_RSP":{ "st":25, "k":"short"} } )"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, IncorrectCommandType)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_SETQ":[""] })"_json,	.expected = {R"({ "KV_SETQ_RSP":{ "st":12, "k":"" } })"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
