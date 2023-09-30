#include "useful/TestCommon.h"

using namespace fusion::test;


TEST_F(FusionTest, Scalar)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_SET":{"string1":"asda"}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"string1" } })"_json} }},
		{TestData { .request = R"({ "KV_SET":{"integer1":1}})"_json,			.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"integer1" } })"_json} }},
		{TestData { .request = R"({ "KV_SET":{"decimal1":1.5}})"_json,		.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"decimal1" } })"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, Object)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_SET":{"object1": { "a":"a", "b":2 } }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"object1" } })"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, Array)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_SET":{"array1":["a", "b"] } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"array1" } })"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, MultipleVarious)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_SET":{"array1":["a", "b"], "array2":[1.5], "myinta":5, "someobject":{"user":"toad"} } })"_json,
								.expected = {
															R"({ "KV_SET_RSP":{ "st":20, "k":"array1" } })"_json,
														 	R"({ "KV_SET_RSP":{ "st":20, "k":"array2" } })"_json,
															R"({ "KV_SET_RSP":{ "st":20, "k":"myinta" } })"_json,
															R"({ "KV_SET_RSP":{ "st":20, "k":"someobject"} })"_json
														}
							}
		}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, Update)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_SET":{"string1":"asda"}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"string1" } })"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);


	// update
	const std::vector<TestData> update = 
	{
		{TestData { .request = R"({ "KV_SET":{"string1":"asda2"}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":21, "k":"string1" } })"_json} }}
	};

	for(auto& d : update)
		tc.test(d);
}


TEST_F(FusionTest, KeyShort)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_SET":{"short":["a", "b"] } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":25, "k":"short" } })"_json} }}
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
		{TestData { .request = R"({ "KV_SET":[""] })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":12, "k":"" } })"_json} }}
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
