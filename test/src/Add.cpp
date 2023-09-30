#include "useful/TestCommon.h"

using namespace fusion::test;


TEST_F(FusionTest, AddNew)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_ADD":{"string1":"asda"}} )"_json,	.expected = {R"({ "KV_ADD_RSP":{ "st":20, "k":"string1" } })"_json} }},
		{TestData { .request = R"({ "KV_ADD":{"integer1":1}} )"_json,			.expected = {R"({ "KV_ADD_RSP":{ "st":20, "k":"integer1" } })"_json} }},
		{TestData { .request = R"({ "KV_ADD":{"decimal1":1.5}} )"_json,		.expected = {R"({ "KV_ADD_RSP":{ "st":20, "k":"decimal1" } })"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, AddToExisting)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_ADD":{"string1":"asda"}} )"_json,	.expected = {R"({ "KV_ADD_RSP":{ "st":20, "k":"string1" } })"_json} }},
		{TestData { .request = R"({ "KV_ADD":{"integer1":1}} )"_json,			.expected = {R"({ "KV_ADD_RSP":{ "st":20, "k":"integer1" } })"_json} }},
		{TestData { .request = R"({ "KV_ADD":{"decimal1":1.5}} )"_json,		.expected = {R"({ "KV_ADD_RSP":{ "st":20, "k":"decimal1" } })"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);

	// now attempt to overwrite
	const std::vector<TestData> update = 
	{
		{TestData { .request = R"({ "KV_ADD":{"string1":"asda"}} )"_json,	.expected = {R"({ "KV_ADD_RSP":{ "st":23, "k":"string1" } })"_json} }},
		{TestData { .request = R"({ "KV_ADD":{"integer1":1}} )"_json,			.expected = {R"({ "KV_ADD_RSP":{ "st":23, "k":"integer1" } })"_json} }},
		{TestData { .request = R"({ "KV_ADD":{"decimal1":1.5}} )"_json,		.expected = {R"({ "KV_ADD_RSP":{ "st":23, "k":"decimal1" } })"_json} }}
	};

	for(auto& d : update)
		tc.test(d);
}


TEST_F(FusionTest, KeyShort)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_ADD":{"short":["a", "b"] } })"_json,	.expected = {R"({ "KV_ADD_RSP":{ "st":25, "k":"short" } })"_json} }}
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
		{TestData { .request = R"({ "KV_ADD":[] })"_json,	.expected = {R"({ "KV_ADD_RSP":{ "st":12, "k":"" } })"_json} }}
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
