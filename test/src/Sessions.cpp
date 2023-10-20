#define FC_UNIT_TEST_NOMONITOR

#include "useful/TestCommon.h"

using namespace fusion::test;

// errors

// TEST_F(FusionTest, New_NoName)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.openNoSession());

// 	tc.test(TestData {  .request =  R"({ "SH_NEW":{ }})"_json,
// 											.expected = {R"({ "SH_NEW_RSP":{"st":13, "m":"", "tkn":null}})"_json},
//                       .checkToken = true});
// }


// TEST_F(FusionTest, New_NameEmpty)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.openNoSession());

// 	tc.test(TestData {  .request =  R"({ "SH_NEW":{ "name":"" }})"_json,
// 											.expected = {R"({ "SH_NEW_RSP":{"st":42, "m":"name", "tkn":null}})"_json},
//                       .checkToken = true});
// }


// TEST_F(FusionTest, New_NameNotString)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.openNoSession());

// 	tc.test(TestData {  .request =  R"({ "SH_NEW":{ "name":523423 }})"_json,
// 											.expected = {R"({ "SH_NEW_RSP":{"st":41, "m":"name", "tkn":null}})"_json},
//                       .checkToken = true});
// }


// TEST_F(FusionTest, New_SharedWrongType)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.openNoSession());

// 	tc.test(TestData {  .request =  R"({ "SH_NEW":{ "name":"sesh1", "shared":"a" }})"_json,
// 											.expected = {R"({ "SH_NEW_RSP":{"st":13, "m":"shared", "tkn":null}})"_json},
//                       .checkToken = true});
// }


// TEST_F(FusionTest, New_ExpiryWrongType)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.openNoSession());

// 	tc.test(TestData {  .request =  R"({ "SH_NEW":{ "name":"sesh1", "expiry":"aye" }})"_json,
// 											.expected = {R"({ "SH_NEW_RSP":{"st":13, "m":"expiry", "tkn":null}})"_json},
//                       .checkToken = true});
// }


// happy
// TEST_F(FusionTest, New_SharedNotSet)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.openNoSession());

//   TestData sesh1 {  .request =  R"({ "SH_NEW":{ "name":"sesh1" }})"_json,
// 									  .expected = {R"({ "SH_NEW_RSP":{"st":1, "name":"sesh1"}})"_json}};

//   tc.token = sesh1.token;

//   TestData sesh1Check {   .request =  R"({ "SH_INFO":{ }})"_json,
// 									        .expected = {R"({ "SH_INFO_RSP":{"st":1, "shared":false, "expiry":{"expires":false, "time":0},"keyCnt":0}})"_json}};
  
//   ASSERT_EQ(sesh1.token, sesh1Check.token);

// 	tc.test(sesh1);
// }


TEST_F(FusionTest, New_SharedSet)
{
	TestClient tc;

	ASSERT_TRUE(tc.openNoSession());

  // not shared
  TestData sesh1 {  .request =  R"({ "SH_NEW":{ "name":"sesh1", "shared":false }})"_json,
									  .expected = {R"({ "SH_NEW_RSP":{"st":1, "name":"sesh1"}})"_json}};

  tc.token = sesh1.token;

  TestData sesh1Check {   .request =  R"({ "SH_INFO":{ }})"_json,
									        .expected = {R"({ "SH_INFO_RSP":{"st":1, "shared":false, "expiry":{"expires":false, "time":0}, "keyCnt":0}})"_json}};
  
  ASSERT_EQ(sesh1.token, sesh1Check.token);

  // shared
  TestData sesh2 {  .request =  R"({ "SH_NEW":{ "name":"sesh2", "shared":true }})"_json,
									  .expected = {R"({ "SH_NEW_RSP":{"st":1, "name":"sesh2"}})"_json}};

  tc.token = sesh2.token;

  TestData sesh2Check {   .request =  R"({ "SH_INFO":{ }})"_json,
									        .expected = {R"({ "SH_INFO_RSP":{"st":1, "shared":true, "expiry":{"expires":false, "time":0}, "keyCnt":0}})"_json}};
  
  ASSERT_EQ(sesh2.token, sesh2Check.token);

	tc.test(sesh2);
}


// TODO: - can't skip "expiry" which is awkward when duration is not 0 because expiry::time is unknown
//       - add "ignorePath" to the TestData:  .ignore = "/expiry"  will copy and delete that path so it
//         it can be checked here
// TEST_F(FusionTest, New_ExpirySet)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.openNoSession());

//   // not shared
//   TestData sesh1 {  .request =  R"({ "SH_NEW":{ "name":"sesh1", "shared":false, "expiry":{"duration":10} }})"_json,
// 									  .expected = {R"({ "SH_NEW_RSP":{"st":1, "name":"sesh1"}})"_json}};

//   tc.token = sesh1.token;

//   TestData sesh1Check {   .request =  R"({ "SH_INFO":{ }})"_json,
// 									        .expected = {R"({ "SH_INFO_RSP":{"st":1, "shared":false, "expiry":{"expires":false, "time":0}, "keyCnt":0}})"_json}};
  
//   ASSERT_EQ(sesh1.token, sesh1Check.token);

//   // shared
//   TestData sesh2 {  .request =  R"({ "SH_NEW":{ "name":"sesh2", "shared":true }})"_json,
// 									  .expected = {R"({ "SH_NEW_RSP":{"st":1, "name":"sesh2"}})"_json}};

//   tc.token = sesh2.token;

//   TestData sesh2Check {   .request =  R"({ "SH_INFO":{ }})"_json,
// 									        .expected = {R"({ "SH_INFO_RSP":{"st":1, "shared":true, "expiry":{"expires":false, "time":0}, "keyCnt":0}})"_json}};
  
//   ASSERT_EQ(sesh2.token, sesh2Check.token);

// 	tc.test(sesh2);
// }


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}