#include "useful/TestCommon.h"

using namespace fusion::test;


TEST_F(FusionTest, String)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_SET":{"string1":"asda"}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"string1" } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_APPEND":{"string1":"_tesco"}})"_json,	.expected = {R"({ "KV_APPEND_RSP":{ "st":1, "k":"string1" } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_GET":["string1"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "string1":"asda_tesco" } })"_json} }});
}


TEST_F(FusionTest, Array)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_SET":{"myarray":["asda", 1, 2]}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"myarray" } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_APPEND":{"myarray":[3, 4, "tesco"]}})"_json,	.expected = {R"({ "KV_APPEND_RSP":{ "st":1, "k":"myarray" } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_GET":["myarray"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "myarray":["asda", 1, 2, 3, 4, "tesco"] } })"_json} }});
}


TEST_F(FusionTest, Object)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_SET":{"myobject":{"s":"string", "i":5}}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"myobject" } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_APPEND":{"myobject":{"d":5.5, "o":{"os":"str"}, "a":[0,1]} }})"_json,	.expected = {R"({ "KV_APPEND_RSP":{ "st":1, "k":"myobject" } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_GET":["myobject"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "myobject":{"s":"string", "i":5, "d":5.5, "o":{"os":"str"}, "a":[0,1]} } })"_json} }});
}


TEST_F(FusionTest, KeyNotExist)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test(TestData { .request = R"({ "KV_APPEND":{"imnothere":["a", "b"] } })"_json,	.expected = {R"({ "KV_APPEND_RSP":{ "st":22, "k":"imnothere" } })"_json} });
}


TEST_F(FusionTest, KeyShort)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData { .request = R"({ "KV_APPEND":{"short":["a", "b"] } })"_json,	.expected = {R"({ "KV_APPEND_RSP":{ "st":25, "k":"short" } })"_json} });
}


TEST_F(FusionTest, IncorrectCommandType)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData { .request = R"({ "KV_APPEND":[""] })"_json,	.expected = {R"({ "KV_APPEND_RSP":{ "st":12, "k":"" } })"_json} });
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
