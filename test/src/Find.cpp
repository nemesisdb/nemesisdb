#include "useful/TestCommon.h"

using namespace fusion::test;

// Error cases

TEST_F(FusionTest, NoOperator)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"/a" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":13, "m":"" } })"_json} }});
}


TEST_F(FusionTest, NoOpNoPath)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "x":"asda", "y":"asda" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":60, "m":"" } })"_json} }});
}


TEST_F(FusionTest, EmptyPath)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"", ">":5 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "keys":[] } })"_json} }});
}


TEST_F(FusionTest, PathNotString)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":[], ">":5 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":41, "m":"path" } })"_json} }});
}


TEST_F(FusionTest, SwapOrder)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"/a", ">":5 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "keys":[] } })"_json} }});
	tc.test({TestData { .request = R"({ "KV_FIND":{ ">":5, "path":"/a" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "keys":[] } })"_json} }});
}


TEST_F(FusionTest, InvalidOp)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_FIND":{ "path":"/a", "&":5 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":62, "m":"" } })"_json} }});
}


/*
//regex errors


// TEST_F(FusionTest, RegexNoOp)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

// 	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/a", "keyrgx":"k?y" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":61, "k":"" } })"_json} });
// }


// TEST_F(FusionTest, RegexEmpty)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

// 	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/a", "==":"x", "keyrgx":"" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":63, "k":"" } })"_json} });
// }
*/


//Happy cases

TEST_F(FusionTest, NoData)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/a", "==":"dave" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "keys":[] } })"_json} });
}


TEST_F(FusionTest, RootStringEq)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
 tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"user:1234":"dave"} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"user:1234":20} } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"", "==":"dave" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "keys":["user:1234"] } })"_json} });
}


TEST_F(FusionTest, RootObjectEq)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
 tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"user:1234": {"name":"dave"}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"user:1234":20} } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/name", "==":"dave"} })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "keys":["user:1234"] } })"_json} });
}


TEST_F(FusionTest, RootArrayEq)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
 tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"user:1234":["dave", 123]} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"user:1234":20} } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"", "==":["dave", 123] } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "keys":["user:1234"] } })"_json} });
}


TEST_F(FusionTest, RootDecimalEq)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
 tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"x":123.123} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"x":20} } })"_json} });
 tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"", "==":123.123 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "keys":["x"] } })"_json} });
}


// NOTE: the order of keys in the "k" array is important. Make sure they are in the correct order for the purposes of the test

TEST_F(FusionTest, RootIntegerLt)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
  tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"x":10, "y":5, "z":3} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"x":20, "y":20, "z":20} } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"", "<":5 } })"_json,		.expected = {R"({ "KV_FIND_RSP":{ "keys":["z"] }})"_json} });
  tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"", "<=":5 } })"_json,		.expected = {R"({ "KV_FIND_RSP":{ "keys":["y", "z"] }})"_json} });
  tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"", "<=":10 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "keys":["x", "y", "z"] }})"_json} });
}


TEST_F(FusionTest, NestedStringEq)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

  tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"user:1234": {"name":"dave"}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"user:1234":20} } })"_json} });
 	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/name", "==":"dave"} })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "keys":["user:1234"] } })"_json} });
}


TEST_F(FusionTest, NestedIntEq)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
  tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"x": {"age":25}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"x":20} } })"_json} });
	tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"y": {"age":30}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"y":20} } })"_json} });
	tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"z": {"age":25}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"z":20} } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/age", "==":25 } })"_json,		.expected = {R"({ "KV_FIND_RSP":{ "keys":["x", "z"] } })"_json} });
}


TEST_F(FusionTest, NestedIntLt)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
	tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"x": {"age":25}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"x":20} } })"_json} });
	tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"y": {"age":30}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"y":20} } })"_json} });
	tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"z": {"age":20}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"z":20} } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/age", "<":25 } })"_json,		.expected = {R"({ "KV_FIND_RSP":{ "keys":["z"] } })"_json} });
}


TEST_F(FusionTest, NestedArrayEq)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
	tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"x": {"age":[25,30]}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"x":20} } })"_json} });
	tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"y": {"age":[12,13]}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"y":20} } })"_json} });
	tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"z": {"age":[10,13]}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"z":20} } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/age", "==":[25,30] } })"_json,		.expected = {R"({ "KV_FIND_RSP":{ "keys":["x"] } })"_json} });
}


TEST_F(FusionTest, DeepNestedString)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
	tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"x":{"address":{"city":"London"}}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"x":20} } })"_json} });
  tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/address/city", "==":"London" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "keys":["x"] } })"_json} });
}


TEST_F(FusionTest, DeepNestedIntGt)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
  
	tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"x":{"address":{"city":"London", "year":1983}}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"x":20} } })"_json} });
	tc.test(TestData { .request = R"({ "KV_SET":{ "keys":{"y":{"address":{"city":"London", "year":1985}}} } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"y":20} } })"_json} });

  tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/address/year", ">":1984 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "keys":["y"] } })"_json} });
	tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/address/year", ">":1982 } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "keys":["x", "y"] } })"_json} });
}


/*
// TEST_F(FusionTest, RegExNoPath)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
  
//   tc.test(TestData { .request = R"({ "KV_SET":{"user:a:1":"London"} })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:a:1" } })"_json} });
//   tc.test(TestData { .request = R"({ "KV_SET":{"user:b:1":"London"} })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:b:1" } })"_json} });
//   tc.test(TestData { .request = R"({ "KV_SET":{"user:b:2":"London"} })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:b:2" } })"_json} });
//   tc.test(TestData { .request = R"({ "KV_FIND":{ "keyrgx":"user:b:[0-9]+", "==":"London" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:b:1", "user:b:2"] } })"_json} });
// }


// TEST_F(FusionTest, RegExAndPath)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
  
//   tc.test(TestData { .request = R"({ "KV_SET":{"user:a:1":{"address":{"city":"London"} }  } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:a:1" } })"_json} });
//   tc.test(TestData { .request = R"({ "KV_SET":{"user:b:1":{"address":{"city":"London"} }  } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:b:1" } })"_json} });
//   tc.test(TestData { .request = R"({ "KV_SET":{"user:b:2":{"address":{"city":"London"} }  } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"user:b:2" } })"_json} });
//   tc.test(TestData { .request = R"({ "KV_FIND":{ "path":"/address/city", "keyrgx":"user:b:[0-9]+", "==":"London" } })"_json,	.expected = {R"({ "KV_FIND_RSP":{ "st":1, "k":["user:b:1", "user:b:2"] } })"_json} });
// }
*/


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}

