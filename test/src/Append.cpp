#include "useful/TestCommon.h"

using namespace fusion::test;


TEST_F(FusionTest, String)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"string1":"asda"}}})"_json,			.expected = {R"({ "KV_SET_RSP":{ "keys":{"string1":20} } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_APPEND":{"string1":"_tesco"}})"_json,					.expected = {R"({ "KV_APPEND_RSP":{ "st":1, "k":"string1" } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_GET":{"keys":["string1"]} })"_json,						.expected = {R"({ "KV_GET_RSP":{ "string1":"asda_tesco" } })"_json} }});
}


TEST_F(FusionTest, Array)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"myarray":["asda", 1, 2]}}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"myarray":20} } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_APPEND":{"myarray":[3, 4, "tesco"]}})"_json,			.expected = {R"({ "KV_APPEND_RSP":{ "st":1, "k":"myarray" } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_GET":{"keys":["myarray"]} })"_json,								.expected = {R"({ "KV_GET_RSP":{ "myarray":["asda", 1, 2, 3, 4, "tesco"] } })"_json} }});
}


TEST_F(FusionTest, Object)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"myobject":{"s":"string", "i":5}}}})"_json,							.expected = {R"({ "KV_SET_RSP":{ "keys":{"myobject":20} } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_APPEND":{"myobject":{"d":5.5, "o":{"os":"str"}, "a":[0,1]} }})"_json,	.expected = {R"({ "KV_APPEND_RSP":{ "st":1, "k":"myobject" } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_GET":{"keys":["myobject"]} })"_json,																	.expected = {R"({ "KV_GET_RSP":{ "myobject":{"s":"string", "i":5, "d":5.5, "o":{"os":"str"}, "a":[0,1]} } })"_json} }});
}


TEST_F(FusionTest, KeyNotExist)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_GET":{"keys":["string1"]} })"_json,	.expected = {R"({ "KV_GET_RSP":{ "string1":null } })"_json} }});
}



int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
