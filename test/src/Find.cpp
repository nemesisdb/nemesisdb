#include "useful/TestCommon.h"

using namespace fusion::test;


TEST_F(FusionTest, InvalidCommand)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ ">":5 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":13, "k":"" } })"_json} }});
}


TEST_F(FusionTest, NoOperator)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"/a" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":13, "k":"" } })"_json} }});
}


TEST_F(FusionTest, NoOpNoPath)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "x":"asda", "y":"asda" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":60, "k":"" } })"_json} }});
}


TEST_F(FusionTest, EmptyPath)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"", ">":5 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":61, "k":"" } })"_json} }});
}


TEST_F(FusionTest, PathNotString)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":[], ">":5 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":61, "k":"" } })"_json} }});
}


TEST_F(FusionTest, InvalidOp)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"/a", "&":5 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":62, "k":"" } })"_json} }});
}


TEST_F(FusionTest, IncorrectCommandType)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData { .request = R"({ "KV_FIND":[] })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":12, "k":"" } })"_json} });
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
