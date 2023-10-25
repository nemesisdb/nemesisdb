#include "useful/TestCommon.h"

using namespace nemesis::test;


TEST_F(NemesisTest, String)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"string1":"asda"}}})"_json,			.expected = {R"({ "KV_SET_RSP":{ "keys":{"string1":20} } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_APPEND":{"keys":{"string1":"_tesco"}}})"_json,					.expected = {R"({ "KV_APPEND_RSP":{ "keys":{"string1":1} } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_GET":{"keys":["string1"]} })"_json,						.expected = {R"({ "KV_GET_RSP":{ "keys":{"string1":"asda_tesco"} } })"_json} }});
}


TEST_F(NemesisTest, Array)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"myarray":["asda", 1, 2]}}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"myarray":20} } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_APPEND":{"keys":{"myarray":[3, 4, "tesco"]}}})"_json,			.expected = {R"({ "KV_APPEND_RSP":{ "keys":{"myarray":1} } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_GET":{"keys":["myarray"]} })"_json,								.expected = {R"({ "KV_GET_RSP":{ "keys":{"myarray":["asda", 1, 2, 3, 4, "tesco"]} } })"_json} }});
}


TEST_F(NemesisTest, Object)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_SET":{"keys":{"myobject":{"s":"string", "i":5}}}})"_json,							.expected = {R"({ "KV_SET_RSP":{ "keys":{"myobject":20} } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_APPEND":{"keys":{"myobject":{"d":5.5, "o":{"os":"str"}, "a":[0,1]}} }})"_json,	.expected = {R"({ "KV_APPEND_RSP":{ "keys":{"myobject":1} } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_GET":{"keys":["myobject"]} })"_json,																	.expected = {R"({ "KV_GET_RSP":{ "keys":{"myobject":{"s":"string", "i":5, "d":5.5, "o":{"os":"str"}, "a":[0,1]}} } })"_json} }});
}


TEST_F(NemesisTest, KeyNotExist)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_APPEND":{"keys":{"x":"y"}}})"_json,	.expected = {R"({ "KV_APPEND_RSP":{ "keys":{"x":22} } })"_json} }});

}



int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
