#include "useful/TestCommon.h"

using namespace nemesis::test;


// TEST_F(NemesisTest, Rename)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

// 	tc.test({TestData { .request = R"({ "KV_SET":{"string1":"billybob"}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"string1" } })"_json} }});
// 	tc.test({TestData { .request = R"({ "KV_RNM":{"string1":"string2"}})"_json,	.expected = {R"({ "KV_RNM_RSP":{ "st":1, "string1":"string2" } })"_json} }});
// }



// TEST_F(NemesisTest, KeyNotExist)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KV_RNM":{"stringNotHere":"string2"}})"_json,	.expected = {R"({ "KV_RNM_RSP":{ "st":22, "k":"string1" } })"_json} }});
// }


// TEST_F(NemesisTest, KeyShort)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

// 	tc.test({TestData { .request = R"({ "KV_RNM":{"str":"string2"}})"_json,	.expected = {R"({ "KV_RNM_RSP":{ "st":25, "k":"str" } })"_json} }});
// }


// TEST_F(NemesisTest, IncorrectCommandType)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

// 	tc.test(TestData { .request = R"({ "KV_RNM":[""] })"_json,	.expected = {R"({ "KV_RNM_RSP":{ "st":12, "k":"" } })"_json} });
// }


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
