#include "useful/TestCommon.h"

using namespace fusion::test;

// Error cases

TEST_F(FusionTest, NoOperator)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"/a" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":61, "k":"" } })"_json} }});
}


TEST_F(FusionTest, NoOpNoPath)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "x":"asda", "y":"asda" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":13, "k":"" } })"_json} }});
}


TEST_F(FusionTest, EmptyPath)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"", ">":5 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":62, "k":"" } })"_json} }});
}


TEST_F(FusionTest, PathNotString)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":[], ">":5 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":62, "k":"" } })"_json} }});
}


TEST_F(FusionTest, InvalidOp)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"/a", "&":5 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":61, "k":"" } })"_json} }});
}


TEST_F(FusionTest, IncorrectCommandType)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData { .request = R"({ "KV_FIND":[] })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":12, "k":"" } })"_json} });
}


// Happy cases

TEST_F(FusionTest, NoData)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/a", "==":"dave" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":[] } })"_json} });
}


TEST_F(FusionTest, RootStringEq)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
  tc.test(TestData { .request = R"({ "KV_SET":{ "user:1234":"dave" } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:1234" } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "==":"dave" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:1234"] } })"_json} });
}


TEST_F(FusionTest, RootObjectEq)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
  tc.test(TestData { .request = R"({ "KV_SET":{ "user:1234": {"name":"dave"} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:1234" } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "==":{"name":"dave"} } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:1234"] } })"_json} });
}


TEST_F(FusionTest, RootArrayEq)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
  tc.test(TestData { .request = R"({ "KV_SET":{"user:1234": ["dave", 123]} })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:1234" } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "==":["dave", 123] } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:1234"] } })"_json} });
}


TEST_F(FusionTest, RootDecimalEq)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
  tc.test(TestData { .request = R"({ "KV_SET":{"user:1234":123.123} })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:1234" } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "==":123.123 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:1234"] } })"_json} });
}


// NOTE: the order of keys in the "k" array is important. Make sure they are in the correct order for the purposes of the test

TEST_F(FusionTest, RootIntegerLt)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
  tc.test(TestData { .request = R"({ "KV_SET":{"user:1234":10} })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:1234" } })"_json} });
  tc.test(TestData { .request = R"({ "KV_SET":{"user:12345":5} })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:12345" } })"_json} });
  tc.test(TestData { .request = R"({ "KV_SET":{"user:123456":3} })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:123456" } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "<":5 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:123456"]} })"_json} });
  tc.test(TestData { .request = R"({ "KV_FIND":{ "<=":5 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:12345", "user:123456"]} })"_json} });
  tc.test(TestData { .request = R"({ "KV_FIND":{ "<=":10 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:1234", "user:12345", "user:123456"]} })"_json} });
}


TEST_F(FusionTest, NestedStringEq)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
  tc.test(TestData { .request = R"({ "KV_SET":{ "user:1234": {"name":"dave"} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:1234" } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/name", "==":"dave" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:1234"] } })"_json} });
}


TEST_F(FusionTest, NestedIntEq)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
  tc.test(TestData { .request = R"({ "KV_SET":{ "user:1234": {"age":25} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:1234" } })"_json} });
  tc.test(TestData { .request = R"({ "KV_SET":{ "user:12345": {"age":30} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:12345" } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/age", "==":25 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:1234"] } })"_json} });
}


TEST_F(FusionTest, NestedIntLt)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
  tc.test(TestData { .request = R"({ "KV_SET":{ "user:1234": {"age":25} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:1234" } })"_json} });
  tc.test(TestData { .request = R"({ "KV_SET":{ "user:12345": {"age":30} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:12345" } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/age", "<":30 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:1234"] } })"_json} });
  tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/age", "<=":30 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:1234", "user:12345"] } })"_json} });
}


TEST_F(FusionTest, NestedArray)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
  tc.test(TestData { .request = R"({ "KV_SET":{ "user:1234": {"ages":[25,30]} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:1234" } })"_json} });
  tc.test(TestData { .request = R"({ "KV_SET":{ "user:12345": {"ages":[35,40]} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:12345" } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/ages", "==":[35,40] } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:12345"] } })"_json} });
  tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/ages", "==":[25,30] } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:1234"] } })"_json} });
}


TEST_F(FusionTest, NestedObject)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
  tc.test(TestData { .request = R"({ "KV_SET":{ "user:1234": {"info":{"a":"1", "b":"2"}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:1234" } })"_json} });
  tc.test(TestData { .request = R"({ "KV_SET":{ "user:12345": {"info":{"a":"1", "c":"3"}}  } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:12345" } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/info", "==":{"a":"1", "b":"2"} } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:1234"] } })"_json} });
  tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/info", "==":{"a":"1", "c":"3"} } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:12345"] } })"_json} });
}


TEST_F(FusionTest, DeepNestedString)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
  tc.test(TestData { .request = R"({ "KV_SET":{"user:1234":{"address":{"city":"London"} }  } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:1234" } })"_json} });
  tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/address/city", "==":"London" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:1234"] } })"_json} });
}


TEST_F(FusionTest, DeepNestedIntGt)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
  tc.test(TestData { .request = R"({ "KV_SET":{"user:1234":{"address":{"city":"London", "year":1982} }  } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:1234" } })"_json} });
  tc.test(TestData { .request = R"({ "KV_SET":{"user:12345":{"address":{"city":"London", "year":1985} }  } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:12345" } })"_json} });
  tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/address/year", ">":1984 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:12345"] } })"_json} });
  tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/address/year", ">=":1982 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:1234", "user:12345"] } })"_json} });
}



int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}

