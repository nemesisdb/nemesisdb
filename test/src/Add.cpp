#include "useful/TestCommon.h"

using namespace nemesis::test;


TEST_F(NemesisTest, AddNew)
{
	std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_ADD":{"keys":{"string1":"asda"}}} )"_json,	.expected = {R"({ "KV_ADD_RSP":{ "keys":{"string1":20} } })"_json} }},
		{TestData { .request = R"({ "KV_ADD":{"keys":{"integer1":1}}} )"_json,			.expected = {R"({ "KV_ADD_RSP":{ "keys":{"integer1":20} } })"_json} }},
		{TestData { .request = R"({ "KV_ADD":{"keys":{"decimal1":1.5}}} )"_json,		.expected = {R"({ "KV_ADD_RSP":{ "keys":{"decimal1":20} } })"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


TEST_F(NemesisTest, AddToExisting)
{
	std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_ADD":{"keys":{"string1":"asda"}}} )"_json,	.expected = {R"({ "KV_ADD_RSP":{ "keys":{"string1":20} } })"_json} }},
		{TestData { .request = R"({ "KV_ADD":{"keys":{"integer1":1}}} )"_json,			.expected = {R"({ "KV_ADD_RSP":{ "keys":{"integer1":20} } })"_json} }},
		{TestData { .request = R"({ "KV_ADD":{"keys":{"decimal1":1.5}}} )"_json,		.expected = {R"({ "KV_ADD_RSP":{ "keys":{"decimal1":20} } })"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);

	// now attempt to overwrite
	std::vector<TestData> update = 
	{
		{TestData { .request = R"({ "KV_ADD":{"keys":{"string1":"asda"}}} )"_json,	.expected = {R"({ "KV_ADD_RSP":{ "keys":{"string1":23} } })"_json} }},
		{TestData { .request = R"({ "KV_ADD":{"keys":{"integer1":1}}} )"_json,			.expected = {R"({ "KV_ADD_RSP":{ "keys":{"integer1":23} } })"_json} }},
		{TestData { .request = R"({ "KV_ADD":{"keys":{"decimal1":1.5}}} )"_json,		.expected = {R"({ "KV_ADD_RSP":{ "keys":{"decimal1":23} } })"_json} }}
	};

	for(auto& d : update)
		tc.test(d);
}


TEST_F(NemesisTest, NoKeys)
{
	std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_ADD":{"keys":{ }}})"_json,	.expected = {R"({ "KV_ADD_RSP":{ "keys":{} } })"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


TEST_F(NemesisTest, IncorrectCommandType)
{
	std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_ADD":[] })"_json,	.expected = {R"({"KV_ADD_RSP":{"m":"","st":12}})"_json} }}
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
