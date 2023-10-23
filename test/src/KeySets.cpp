#include "useful/TestCommon.h"
#include <core/NemesisCommon.h>

using namespace nemesis::test;

// create
// TEST_F(NemesisTest, KsCreate_NameErrors)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CREATE":{} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":40, "m":"name" } })"_json} }});
// 	tc.test({TestData { .request = R"({ "KS_CREATE":{"a":""} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":40, "m":"name" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":""}})"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":83, "m":"name" } })"_json} }});
// }


// TEST_F(NemesisTest, KsCreate_Ok)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
// }


// TEST_F(NemesisTest, KsCreate_Duplicate)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":81, "name":"ks1" } })"_json} }});
// }


// // list
// TEST_F(NemesisTest, KsList_Ok)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
// 	 tc.test({TestData { .request = R"({ "KS_LIST":{} })"_json,	.expected = {R"({ "KS_LIST_RSP":{ "st":1, "list":["ks1"] } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks2"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks2" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_LIST":{} })"_json,	.expected = {R"({ "KS_LIST_RSP":{ "st":1, "list":["ks1", "ks2"] } })"_json} }});
// }


// TEST_F(NemesisTest, KsList_Empty)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_LIST":{} })"_json,	.expected = {R"({ "KS_LIST_RSP":{ "st":1, "list":[] } })"_json} }});
// }


// //add
// TEST_F(NemesisTest, KsAddKey_NotArray)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1", "k":{}} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":41, "m":"k" } })"_json} }});
// }


// TEST_F(NemesisTest, KsAddKey_MissingValues)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"k":["ks1"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":13, "m":"" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1"} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":13, "m":"" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"tesco":"ks1", "asda":["ks1"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":40, "m":"" } })"_json} }});
// }


// TEST_F(NemesisTest, KsAddKey_NoKeys)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1", "k":[]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":1, "ks":"ks1" } })"_json} }});
// }



// //get
// TEST_F(NemesisTest, KsGet_NotArray)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_GET":{} })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":12, "m":"" } })"_json} }});
// }


// TEST_F(NemesisTest, KsGet_EmptyKs)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_GET":[] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":40, "m":"" } })"_json} }});
// }


// TEST_F(NemesisTest, KsGet_KsNotString)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_GET":[1] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1 } })"_json} }});
//  tc.test({TestData { .request = R"({ "KS_GET":[2, 1] })"_json,	.expected = { R"({ "KS_GET_RSP":{ "st":1 } })"_json}}});
// }


// TEST_F(NemesisTest, KsGet_KsNotExist)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	tc.test({TestData { .request = R"({ "KS_GET":["ks1"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":[] } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_GET":["ks1", "ks2"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":[], "ks2":[] } })"_json} }});
  
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_GET":["ks1", "ks2"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":[], "ks2":[] } })"_json} }});  
// }


// TEST_F(NemesisTest, KsGet_KsExist)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_GET":["ks1"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":[] } })"_json} }});  

//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks2"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks2" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_GET":["ks2"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks2":[] } })"_json} }});  

//   tc.test({TestData { .request = R"({ "KS_GET":["ks1", "ks2"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":[], "ks2":[] } })"_json} }});  
// }


// TEST_F(NemesisTest, KsGet_OneKs)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});

//   tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1", "k":["user:10"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":1, "ks":"ks1" } })"_json} }});  
//   tc.test({TestData { .request = R"({ "KS_GET":["ks1"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":["user:10"] } })"_json} }}); 

//   tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1", "k":["user:11"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":1, "ks":"ks1" } })"_json} }});  
//   tc.test({TestData { .request = R"({ "KS_GET":["ks1"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":["user:10", "user:11"] } })"_json} }}); 
// }


// TEST_F(NemesisTest, KsGet_TwoKs)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks2"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks2" } })"_json} }});

//   tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1", "k":["user:10", "user:11"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":1, "ks":"ks1" } })"_json} }});  
//   tc.test({TestData { .request = R"({ "KS_GET":["ks1"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":["user:10", "user:11"] } })"_json} }});

//   tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks2", "k":["user2:10", "user2:11"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":1, "ks":"ks2" } })"_json} }});  
//   tc.test({TestData { .request = R"({ "KS_GET":["ks2"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks2":["user2:10", "user2:11"] } })"_json} }});

  
//   tc.test({TestData { .request = R"({ "KS_GET":["ks1", "ks2"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":["user:10", "user:11"], "ks2":["user2:10", "user2:11"] } })"_json} }});
// }

// // remove key
// TEST_F(NemesisTest, KsRmvKey_MissingValue)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_RMV_KEY":{} })"_json,	.expected = {R"({ "KS_RMV_KEY_RSP":{ "st":40, "m":"" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_RMV_KEY":{"ks":""} })"_json,	.expected = {R"({ "KS_RMV_KEY_RSP":{ "st":40, "m":"" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_RMV_KEY":{"k":[]} })"_json,	.expected = {R"({ "KS_RMV_KEY_RSP":{ "st":40, "m":"" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_RMV_KEY":{"asda":"", "dfspk":""} })"_json,	.expected = {R"({ "KS_RMV_KEY_RSP":{ "st":40, "m":"" } })"_json} }});
// }


// TEST_F(NemesisTest, KsRmvKey_InvalidTypes)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_RMV_KEY":{"ks":5, "k":""} })"_json,	.expected = {R"({ "KS_RMV_KEY_RSP":{ "st":41, "m":"" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_RMV_KEY":{"ks":"", "k":[]} })"_json,	.expected = {R"({ "KS_RMV_KEY_RSP":{ "st":41, "m":"" } })"_json} }});
// }


// TEST_F(NemesisTest, KsRmvKey_KsNotExist)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_RMV_KEY":{"ks":"ks1", "k":"user:1"} })"_json,	.expected = {R"({ "KS_RMV_KEY_RSP":{ "st":82, "ks":"ks1", "k":"user:1" } })"_json} }});
// }


// TEST_F(NemesisTest, KsRmvKey_KeyNotExist)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_RMV_KEY":{"ks":"ks1", "k":"user:1"} })"_json,	.expected = {R"({ "KS_RMV_KEY_RSP":{ "st":22, "ks":"ks1", "k":"user:1" } })"_json} }});
  
//   tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1", "k":["user:1"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":1, "ks":"ks1" } })"_json} }});  
//   tc.test({TestData { .request = R"({ "KS_RMV_KEY":{"ks":"ks1", "k":"user:2"} })"_json,	.expected = {R"({ "KS_RMV_KEY_RSP":{ "st":22, "ks":"ks1", "k":"user:2" } })"_json} }});
// }


// TEST_F(NemesisTest, KsRmvKey_Remove)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
  
//   tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1", "k":["user:1", "user:2"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":1, "ks":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_GET":["ks1"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":["user:1", "user:2"] } })"_json} }}); 

//   tc.test({TestData { .request = R"({ "KS_RMV_KEY":{"ks":"ks1", "k":"user:1"} })"_json,	.expected = {R"({ "KS_RMV_KEY_RSP":{ "st":1, "ks":"ks1", "k":"user:1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_GET":["ks1"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":["user:2"] } })"_json} }}); 
// }


// clear set
// TEST_F(NemesisTest, KsClearSet_Invalid)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CLEAR_SET":{} })"_json,	.expected = {R"({ "KS_CLEAR_SET_RSP":{ "st":40, "m":""} })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_CLEAR_SET":{ "ks":123 } })"_json,	.expected = {R"({ "KS_CLEAR_SET_RSP":{ "st":41, "m":"" } })"_json} }});
// }


// TEST_F(NemesisTest, KsClearSet_SetNotExist)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CLEAR_SET":{ "ks":"notexist" } })"_json,	.expected = {R"({ "KS_CLEAR_SET_RSP":{ "st":82, "ks":"notexist" } })"_json} }});
// }


// TEST_F(NemesisTest, KsClearSet_Ok)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
  
//   tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1", "k":["user:1", "user:2"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":1, "ks":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_GET":["ks1"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":["user:1", "user:2"] } })"_json} }}); 

//   tc.test({TestData { .request = R"({ "KS_CLEAR_SET":{ "ks":"ks1" } })"_json,	.expected = {R"({ "KS_CLEAR_SET_RSP":{ "st":1, "ks":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_GET":["ks1"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":[] } })"_json} }}); 
// }


// delete set
// TEST_F(NemesisTest, KsDeleteSet_Invalid)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_DELETE_SET":{} })"_json,	.expected = {R"({ "KS_DELETE_SET_RSP":{ "st":40, "m":""} })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_DELETE_SET":{ "ks":123 } })"_json,	.expected = {R"({ "KS_DELETE_SET_RSP":{ "st":41, "m":"" } })"_json} }});
// }


// TEST_F(NemesisTest, KsDeleteSet_SetNotExist)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_DELETE_SET":{ "ks":"ks1" } })"_json,	.expected = {R"({ "KS_DELETE_SET_RSP":{ "st":82, "ks":"ks1" } })"_json} }});
// }


// TEST_F(NemesisTest, KsDeleteSet_Ok)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_LIST":{} })"_json,	.expected = {R"({ "KS_LIST_RSP":{ "st":1, "list":["ks1"] } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_DELETE_SET":{ "ks":"ks1" } })"_json,	.expected = {R"({ "KS_DELETE_SET_RSP":{ "st":1, "ks":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_LIST":{} })"_json,	.expected = {R"({ "KS_LIST_RSP":{ "st":1, "list":[] } })"_json} }});
// }


// TEST_F(NemesisTest, KsDeleteSet_Invalid)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_DELETE_ALL":{ "notallowed":123 } })"_json,	.expected = {R"({ "KS_DELETE_ALL_RSP":{ "st":13, "m":"" } })"_json} }});
// }


// TEST_F(NemesisTest, KsDeleteSet_Ok)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_DELETE_ALL":{ } })"_json,	.expected = {R"({ "KS_DELETE_ALL_RSP":{ "st":1 } })"_json} }});

//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks2"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks2" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_LIST":{} })"_json,	.expected = {R"({ "KS_LIST_RSP":{ "st":1, "list":["ks1", "ks2"] } })"_json} }});

//   tc.test({TestData { .request = R"({ "KS_DELETE_ALL":{ } })"_json,	.expected = {R"({ "KS_DELETE_ALL_RSP":{ "st":1 } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_LIST":{} })"_json,	.expected = {R"({ "KS_LIST_RSP":{ "st":1, "list":[] } })"_json} }});
// }


// set exists
// TEST_F(NemesisTest, KsSetExists_Invalid)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_SET_EXISTS":{} })"_json,	.expected = {R"({ "KS_SET_EXISTS_RSP":{ "st":40, "m":""} })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_SET_EXISTS":{ "ks":123 } })"_json,	.expected = {R"({ "KS_SET_EXISTS_RSP":{ "st":41, "m":"" } })"_json} }});
// }


// TEST_F(NemesisTest, KsSetExists_NotExist)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_SET_EXISTS":{ "ks":"ks1" } })"_json,	.expected = {R"({ "KS_SET_EXISTS_RSP":{ "st":82, "ks":"ks1" } })"_json} }});
// }


// TEST_F(NemesisTest, KsSetExists_Exist)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_SET_EXISTS":{ "ks":"ks1" } })"_json,	.expected = {R"({ "KS_SET_EXISTS_RSP":{ "st":82, "ks":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_SET_EXISTS":{ "ks":"ks1" } })"_json,	.expected = {R"({ "KS_SET_EXISTS_RSP":{ "st":81, "ks":"ks1" } })"_json} }});
// }


// key exists
// TEST_F(NemesisTest, KsSetExists_Invalid)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_KEY_EXISTS":{} })"_json,	.expected = {R"({ "KS_KEY_EXISTS_RSP":{ "st":40, "m":""} })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_KEY_EXISTS":{ "ks":"" } })"_json,	.expected = {R"({ "KS_KEY_EXISTS_RSP":{ "st":40, "m":"" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_KEY_EXISTS":{ "k":"" } })"_json,	.expected = {R"({ "KS_KEY_EXISTS_RSP":{ "st":40, "m":"" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_KEY_EXISTS":{ "ks":123, "k":"" } })"_json,	.expected = {R"({ "KS_KEY_EXISTS_RSP":{ "st":41, "m":"" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_KEY_EXISTS":{ "k":[], "ks":""} })"_json,	.expected = {R"({ "KS_KEY_EXISTS_RSP":{ "st":41, "m":"" } })"_json} }});
// }


// TEST_F(NemesisTest, KsSetExists_KsNotExists)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_KEY_EXISTS":{ "ks":"ks1", "k":"user:1"} })"_json,	.expected = {R"({ "KS_KEY_EXISTS_RSP":{ "st":82, "ks":"ks1", "k":"user:1" } })"_json} }});
// }


// TEST_F(NemesisTest, KsSetExists_KeyNotExists)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_KEY_EXISTS":{ "ks":"ks1", "k":"user:1"} })"_json,	.expected = {R"({ "KS_KEY_EXISTS_RSP":{ "st":22, "ks":"ks1", "k":"user:1" } })"_json} }});
// }


// TEST_F(NemesisTest, KsSetExists_KeyExists)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks2"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks2" } })"_json} }});
  
//   tc.test({TestData { .request = R"({ "KS_KEY_EXISTS":{ "ks":"ks1", "k":"user:1"} })"_json,	.expected = {R"({ "KS_KEY_EXISTS_RSP":{ "st":22, "ks":"ks1", "k":"user:1" } })"_json} }});
  
//   tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1", "k":["user:1"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":1, "ks":"ks1" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_KEY_EXISTS":{ "ks":"ks1", "k":"user:1"} })"_json,	.expected = {R"({ "KS_KEY_EXISTS_RSP":{ "st":23, "ks":"ks1", "k":"user:1" } })"_json} }});
  
//   tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks2", "k":["user:2"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":1, "ks":"ks2" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KS_KEY_EXISTS":{ "ks":"ks2", "k":"user:2"} })"_json,	.expected = {R"({ "KS_KEY_EXISTS_RSP":{ "st":23, "ks":"ks2", "k":"user:2" } })"_json} }});
// }


// move key
TEST_F(NemesisTest, KsMove_Invalid)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
  tc.test({TestData { .request = R"({ "KS_MOVE_KEY":{ } })"_json,	.expected = {R"({ "KS_MOVE_KEY_RSP":{ "st":40, "m":""} })"_json} }});
  tc.test({TestData { .request = R"({ "KS_MOVE_KEY":{ "sourceKs":"", "targetKs":"" } })"_json,	.expected = {R"({ "KS_MOVE_KEY_RSP":{ "st":40, "m":"" } })"_json} }});
  tc.test({TestData { .request = R"({ "KS_MOVE_KEY":{ "targetKs":"", "k":"" } })"_json,	.expected = {R"({ "KS_MOVE_KEY_RSP":{ "st":40, "m":"" } })"_json} }});
  tc.test({TestData { .request = R"({ "KS_MOVE_KEY":{ "k":"", "sourceKs":"" } })"_json,	.expected = {R"({ "KS_MOVE_KEY_RSP":{ "st":40, "m":"" } })"_json} }});
  
  tc.test({TestData { .request = R"({ "KS_MOVE_KEY":{ "sourceKs":2, "targetKs":"", "k":"" } })"_json,	.expected = {R"({ "KS_MOVE_KEY_RSP":{ "st":41, "m":"" } })"_json} }});
  tc.test({TestData { .request = R"({ "KS_MOVE_KEY":{ "sourceKs":"", "targetKs":2, "k":"" } })"_json,	.expected = {R"({ "KS_MOVE_KEY_RSP":{ "st":41, "m":"" } })"_json} }});
  tc.test({TestData { .request = R"({ "KS_MOVE_KEY":{ "sourceKs":"", "targetKs":"", "k":2 } })"_json,	.expected = {R"({ "KS_MOVE_KEY_RSP":{ "st":41, "m":"" } })"_json} }});
}


TEST_F(NemesisTest, KsMove_EmptyValues)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
  tc.test({TestData { .request = R"({ "KS_MOVE_KEY":{ "sourceKs":"", "targetKs":"", "k":"" } })"_json,	.expected = {R"({ "KS_MOVE_KEY_RSP":{ "st":82, "sourceKs":"", "targetKs":"", "k":"" } })"_json} }});
}


TEST_F(NemesisTest, KsMove_SourceNotExist)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
  tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
  tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks2"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks2" } })"_json} }});
  
  tc.test({TestData { .request = R"({ "KS_MOVE_KEY":{ "sourceKs":"asda", "targetKs":"ks2", "k":"user:1" } })"_json,	.expected = {R"({ "KS_MOVE_KEY_RSP":{ "st":82, "sourceKs":"asda", "targetKs":"ks2", "k":"user:1" } })"_json} }});
}


TEST_F(NemesisTest, KsMove_TargetNotExist)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
  tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
  tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks2"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks2" } })"_json} }});
  
  tc.test({TestData { .request = R"({ "KS_MOVE_KEY":{ "sourceKs":"ks1", "targetKs":"asda", "k":"user:1" } })"_json,	.expected = {R"({ "KS_MOVE_KEY_RSP":{ "st":82, "sourceKs":"ks1", "targetKs":"asda", "k":"user:1" } })"_json} }});
}


TEST_F(NemesisTest, KsMove_KeyNotExist)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
  tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
  tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks2"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks2" } })"_json} }});
  
  tc.test({TestData { .request = R"({ "KS_MOVE_KEY":{ "sourceKs":"ks1", "targetKs":"ks2", "k":"user:1" } })"_json,	.expected = {R"({ "KS_MOVE_KEY_RSP":{ "st":22, "sourceKs":"ks1", "targetKs":"ks2", "k":"user:1" } })"_json} }});
}


TEST_F(NemesisTest, KsMove_KeyMoved)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
  tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks1"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks1" } })"_json} }});
  tc.test({TestData { .request = R"({ "KS_CREATE":{"name":"ks2"} })"_json,	.expected = {R"({ "KS_CREATE_RSP":{ "st":80, "name":"ks2" } })"_json} }});
  
  tc.test({TestData { .request = R"({ "KS_ADD_KEY":{"ks":"ks1", "k":["user:1"]} })"_json,	.expected = {R"({ "KS_ADD_KEY_RSP":{ "st":1, "ks":"ks1" } })"_json} }});
  
  tc.test({TestData { .request = R"({ "KS_MOVE_KEY":{ "sourceKs":"ks1", "targetKs":"ks2", "k":"user:1" } })"_json,	.expected = {R"({ "KS_MOVE_KEY_RSP":{ "st":1, "sourceKs":"ks1", "targetKs":"ks2", "k":"user:1" } })"_json} }});
  tc.test({TestData { .request = R"({ "KS_GET":["ks1", "ks2"] })"_json,	.expected = {R"({ "KS_GET_RSP":{ "st":1, "ks1":[], "ks2":["user:1"] } })"_json} }}); 
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
