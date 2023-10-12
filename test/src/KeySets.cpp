#include "useful/TestCommon.h"
#include <core/FusionCommon.h>

using namespace fusion::test;

// create
// TEST_F(FusionTest, KsNameErrors)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CREATE":{} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":40, "m":"name" } })"_json} }});
// 	tc.test({TestData { .request = R"({ "KS_CREATE":{"a":""} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":40, "m":"name" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":""}})"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":83, "m":"name" } })"_json} }});
// }


// TEST_F(FusionTest, KsCreate)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
// }


// TEST_F(FusionTest, KsCreateDuplicate)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":81, "name":"ks1" } })"_json} }});
// }


// // list
// TEST_F(FusionTest, KsList)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
// 	 tc.test({TestData { .request = R"({ "KS_LIST":{} })"_json,	.expected = {R"({ "KS_LIST_RSP":{ "st":1, "list":["ks1"] } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks2"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks2" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_LIST":{} })"_json,	.expected = {R"({ "KS_LIST_RSP":{ "st":1, "list":["ks1", "ks2"] } })"_json} }});
// }


// TEST_F(FusionTest, KsListEmpty)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_LIST":{} })"_json,	.expected = {R"({ "KS_LIST_RSP":{ "st":1, "list":[] } })"_json} }});
// }


// add
// TEST_F(FusionTest, KsAddKey_NotArray)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1", "k":{}} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":41, "m":"k" } })"_json} }});
// }


// TEST_F(FusionTest, KsAddKey_MissingValues)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"k":["ks1"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":13, "m":"" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1"} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":13, "m":"" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"tesco":"ks1", "asda":["ks1"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":40, "m":"" } })"_json} }});
// }


// TEST_F(FusionTest, KsAddKey_NoKeys)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1", "k":[]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":1, "ks":"ks1" } })"_json} }});
// }



// get
// TEST_F(FusionTest, KsGet_NotArray)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_GET":{} })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":12, "m":"" } })"_json} }});
// }


// TEST_F(FusionTest, KsGet_EmptyKs)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_GET":[] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":40, "m":"" } })"_json} }});
// }


// TEST_F(FusionTest, KsGet_KsNotString)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_GET":[1] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1 } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_GET":[2, 1] })"_json,	.expected = { R"({ "KS_GET_RSP":{ "st":1 } })"_json}}});
// }


// TEST_F(FusionTest, KsGet_KsNotExist)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_GET":["ks1"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":[] } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_GET":["ks1", "ks2"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":[], "ks2":[] } })"_json} }});
  
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_GET":["ks1", "ks2"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":[], "ks2":[] } })"_json} }});  
// }


// TEST_F(FusionTest, KsGet_KsExist)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_GET":["ks1"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":[] } })"_json} }});  

//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks2"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_GET":["ks2"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks2":[] } })"_json} }});  

//   tc.test({TestData { .request = R"({ "KS_GET":["ks1", "ks2"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":[], "ks2":[] } })"_json} }});  
// }


TEST_F(FusionTest, KsGet_OneKs)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
  tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});

  tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1", "k":["user:10"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":1, "ks":"ks1" } })"_json} }});  
  tc.test({TestData { .request = R"({ "KS_GET":["ks1"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":["user:10"] } })"_json} }}); 

  tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1", "k":["user:11"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":1, "ks":"ks1" } })"_json} }});  
  tc.test({TestData { .request = R"({ "KS_GET":["ks1"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":["user:10", "user:11"] } })"_json} }}); 
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
