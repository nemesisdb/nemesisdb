#include "useful/TestCommon.h"

using namespace fusion::test;


//
// Limited on what we can do because ADDQ doesn't respond unless there's an error.
//


TEST_F(FusionTest, AddNew)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_ADDQ":{"string1":"asda"}} )"_json}},
		{TestData { .request = R"({ "KV_ADDQ":{"integer1":1}} )"_json}},
		{TestData { .request = R"({ "KV_ADDQ":{"decimal1":1.5}} )"_json}}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);

	tc.test(TestData {.request = R"({ "KV_COUNT":{} })"_json, .expected = {R"({ "KV_COUNT_RSP":{"cnt":3, "st":1} } )"_json} });
}


TEST_F(FusionTest, AddToExisting)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_ADDQ":{"string1":"asda"}} )"_json}},
		{TestData { .request = R"({ "KV_ADDQ":{"integer1":1}} )"_json}},
		{TestData { .request = R"({ "KV_ADDQ":{"decimal1":1.5}} )"_json}}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);

	// now attempt to overwrite
	const std::vector<TestData> update = 
	{
		{TestData { .request = R"({ "KV_ADDQ":{"string1":"asda"}} )"_json,	.expected = {R"({ "KV_ADDQ_RSP":{ "st":23, "k":"string1" } })"_json} }},
		{TestData { .request = R"({ "KV_ADDQ":{"integer1":1}} )"_json,			.expected = {R"({ "KV_ADDQ_RSP":{ "st":23, "k":"integer1" } })"_json} }},
		{TestData { .request = R"({ "KV_ADDQ":{"decimal1":1.5}} )"_json,		.expected = {R"({ "KV_ADDQ_RSP":{ "st":23, "k":"decimal1" } })"_json} }}
	};

	for(auto& d : update)
		tc.test(d);
}


TEST_F(FusionTest, KeyShort)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_ADDQ":{"short":["a", "b"] } })"_json,	.expected = {R"({ "KV_ADDQ_RSP":{ "st":25, "k":"short" } })"_json} }}
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
		{TestData { .request = R"({ "KV_ADDQ":[] })"_json,	.expected = {R"({ "KV_ADDQ_RSP":{ "st":12, "k":"" } })"_json} }}
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
