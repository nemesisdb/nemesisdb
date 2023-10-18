#include "useful/TestCommon.h"

using namespace fusion::test;


//
// Limited on what we can do because ADDQ doesn't respond unless there's an error.
//


TEST_F(FusionTest, AddNew)
{
	std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_ADDQ":{"string1":"asda"}} )"_json}},
		{TestData { .request = R"({ "KV_ADDQ":{"integer1":1}} )"_json}},
		{TestData { .request = R"({ "KV_ADDQ":{"decimal1":1.5}} )"_json}}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
	
	TestData count {.request = R"({ "KV_COUNT":{} })"_json, .expected = {R"({ "KV_COUNT_RSP":{"cnt":3, "st":1} } )"_json} };
	tc.test(count);
}


TEST_F(FusionTest, AddToExisting)
{
	std::vector<TestData> data = 
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
	std::vector<TestData> update = 
	{
		{TestData { .request = R"({ "KV_ADD":{"string1":"asda"}} )"_json,	.expected = {R"({ "KV_ADD_RSP":{ "keys":{"string1":23} } })"_json} }},
		{TestData { .request = R"({ "KV_ADD":{"integer1":1}} )"_json,			.expected = {R"({ "KV_ADD_RSP":{ "keys":{"integer1":23} } })"_json} }},
		{TestData { .request = R"({ "KV_ADD":{"decimal1":1.5}} )"_json,		.expected = {R"({ "KV_ADD_RSP":{ "keys":{"decimal1":23} } })"_json} }}
	};

	for(auto& d : update)
		tc.test(d);
}


TEST_F(FusionTest, NoKeys)
{
	std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_ADDQ":{ } })"_json,	.expected = {R"({ "KV_ADDQ_RSP":{ "keys":{} } })"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, IncorrectCommandType)
{
	std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_ADDQ":[] })"_json,	.expected = {R"({"KV_ADDQ_RSP":{"m":"","st":12}})"_json} }}
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
