#define FC_UNIT_TEST_NOMONITOR

#include "useful/TestCommon.h"

using namespace nemesis::test;

// errors

TEST_F(NemesisTest, New_NoName)
{
	TestClient tc;

	ASSERT_TRUE(tc.openNoSession());

  {
		TestData td {MakeTestData(R"({ "SH_NEW":{ }})"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["SH_NEW_RSP"]["st"] == RequestStatus::ParamMissing); 
    ASSERT_TRUE(td.actual[0]["SH_NEW_RSP"]["tkn"].is_null());
	}
}


TEST_F(NemesisTest, New_NameEmpty)
{
	TestClient tc;

	ASSERT_TRUE(tc.openNoSession());

  {
		TestData td {MakeTestData(R"({ "SH_NEW":{ "name":"" }})"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["SH_NEW_RSP"]["st"] == RequestStatus::ValueSize); 
    ASSERT_TRUE(td.actual[0]["SH_NEW_RSP"]["tkn"].is_null());
	}
}


TEST_F(NemesisTest, New_NameNotString)
{
	TestClient tc;

	ASSERT_TRUE(tc.openNoSession());

  {
		TestData td {MakeTestData(R"({ "SH_NEW":{ "name":523423 }})"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["SH_NEW_RSP"]["st"] == RequestStatus::ValueTypeInvalid); 
    ASSERT_TRUE(td.actual[0]["SH_NEW_RSP"]["tkn"].is_null());
	}
}


TEST_F(NemesisTest, New_SharedWrongType)
{
	TestClient tc;

	ASSERT_TRUE(tc.openNoSession());
  
  {
		TestData td {MakeTestData(R"({ "SH_NEW":{ "name":"sesh1", "shared":"a" }})"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["SH_NEW_RSP"]["st"] == RequestStatus::ValueTypeInvalid); 
    ASSERT_TRUE(td.actual[0]["SH_NEW_RSP"]["tkn"].is_null());
	}
}


TEST_F(NemesisTest, New_ExpiryWrongType)
{
	TestClient tc;

	ASSERT_TRUE(tc.openNoSession());

  {
		TestData td {MakeTestData(R"({ "SH_NEW":{ "name":"sesh1", "expiry":"aye" }})"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["SH_NEW_RSP"]["st"] == RequestStatus::ValueTypeInvalid); 
    ASSERT_TRUE(td.actual[0]["SH_NEW_RSP"]["tkn"].is_null());
	}
}


TEST_F(NemesisTest, New_ExpiryClearNotSet)
{
	TestClient tc;

	ASSERT_TRUE(tc.openNoSession());

  {
		TestData td {MakeTestData(R"({ "SH_NEW":{ "name":"sesh1", "shared":false, "expiry":{"duration":10} }})"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["SH_NEW_RSP"]["st"] == RequestStatus::ParamMissing); 
    ASSERT_TRUE(td.actual[0]["SH_NEW_RSP"]["tkn"].is_null());
	}
}


//happy
TEST_F(NemesisTest, New_SharedNotSet)
{
	TestClient tc;

	ASSERT_TRUE(tc.openNoSession());

  TestData sesh1 {  .request =  R"({ "SH_NEW":{ "name":"sesh1" }})"_json,
									  .expected = {R"({ "SH_NEW_RSP":{"st":1, "name":"sesh1"}})"_json}};

  tc.token = sesh1.token;

  TestData sesh1Check {   .request =  R"({ "SH_INFO":{ }})"_json,
									        .expected = {R"({ "SH_INFO_RSP":{"st":1, "shared":false, "keyCnt":0, "expiry":{"expires":false, "time":0}}})"_json}};
  
  ASSERT_EQ(sesh1.token, sesh1Check.token);

	tc.test(sesh1);
}


TEST_F(NemesisTest, New_SharedSet)
{
	TestClient tc;

	ASSERT_TRUE(tc.openNoSession());

  // not shared
  TestData sesh1 {  .request =  R"({ "SH_NEW":{ "name":"sesh1", "shared":false }})"_json,
									  .expected = {R"({ "SH_NEW_RSP":{"st":1, "name":"sesh1"}})"_json}};

  tc.token = sesh1.token;

  TestData sesh1Check {   .request =  R"({ "SH_INFO":{ }})"_json,
									        .expected = {R"({ "SH_INFO_RSP":{"st":1, "shared":false, "keyCnt":0, "expiry":{"expires":false, "time":0, "duration":0, "deleteSession":true}}})"_json}};
  
  ASSERT_EQ(sesh1.token, sesh1Check.token);

  // shared
  TestData sesh2 {  .request =  R"({ "SH_NEW":{ "name":"sesh2", "shared":true }})"_json,
									  .expected = {R"({ "SH_NEW_RSP":{"st":1, "name":"sesh2"}})"_json}};

  tc.token = sesh2.token;

  TestData sesh2Check {   .request =  R"({ "SH_INFO":{ }})"_json,
									        .expected = {R"({ "SH_INFO_RSP":{"st":1, "shared":true, "keyCnt":0, "expiry":{"expires":false, "time":0, "duration":0, "deleteSession":true}}})"_json}};
  
  ASSERT_EQ(sesh2.token, sesh2Check.token);

	tc.test(sesh2);
}


TEST_F(NemesisTest, New_ExpirySet)
{
	TestClient tc;

	ASSERT_TRUE(tc.openNoSession());

  // not shared
  TestData sesh1 {  .request =  R"({ "SH_NEW":{ "name":"sesh1", "shared":false, "expiry":{"duration":10, "deleteSession":true} }})"_json,
									   .nResponses = 1,
                    .checkResponses = false};

  tc.test(sesh1);

  tc.token = sesh1.token;

  TestData sesh1Info {.request =  R"({ "SH_INFO":{ }})"_json,
									    .nResponses = 1,
                      .checkResponses = false};
  
  tc.test(sesh1Info);

  ASSERT_EQ(sesh1.token, sesh1Info.token);
  ASSERT_FALSE(sesh1Info.actual.empty());
  ASSERT_TRUE(sesh1Info.actual[0]["SH_INFO_RSP"].contains("expiry"));
  ASSERT_TRUE(sesh1Info.actual[0]["SH_INFO_RSP"]["expiry"].contains("duration"));
  ASSERT_EQ(sesh1Info.actual[0]["SH_INFO_RSP"]["expiry"]["duration"], 10);
  ASSERT_EQ(sesh1Info.actual[0]["SH_INFO_RSP"]["expiry"]["deleteSession"], true);
}



TEST_F(NemesisTest, New_TwoSesh)
{
	TestClient tc1, tc2;

	ASSERT_TRUE(tc1.openNoSession());
  ASSERT_TRUE(tc2.openNoSession());

  TestData sesh1 {  .request =  R"({ "SH_NEW":{ "name":"sesh1", "shared":true }})"_json,
									  .nResponses = 1,
                    .checkResponses = false};

  TestData sesh2 {  .request =  R"({ "SH_NEW":{ "name":"sesh2", "shared":true }})"_json,
									  .nResponses = 1,
                    .checkResponses = false};


  tc1.test(sesh1);
  tc2.test(sesh2);

  tc1.token = sesh1.token;
  tc2.token = sesh2.token;

  
  TestData sesh1Info {.request =  R"({ "SH_INFO":{ }})"_json,
									    .nResponses = 1,
                      .checkResponses = false};
  
  TestData sesh2Info {.request =  R"({ "SH_INFO":{ }})"_json,
                      .nResponses = 1,
									    .checkResponses = false};

  tc1.test(sesh1Info);
  tc2.test(sesh2Info);

  ASSERT_EQ(sesh1Info.actual[0]["SH_INFO_RSP"]["tkn"], tc1.token["tkn"]);
  ASSERT_EQ(sesh2Info.actual[0]["SH_INFO_RSP"]["tkn"], tc2.token["tkn"]);


  // store in sesh1
  tc1.test(TestData { .request = R"({ "KV_SETQ":{ "keys":{"string":"string", "int":5} }})"_json});
  tc1.test(sesh1Info);
  
  ASSERT_EQ(sesh1Info.actual[0]["SH_INFO_RSP"]["tkn"], tc1.token["tkn"]);
  ASSERT_EQ(sesh1Info.actual[0]["SH_INFO_RSP"]["keyCnt"], 2);


  tc2.test(sesh2Info);
  ASSERT_EQ(sesh2Info.actual[0]["SH_INFO_RSP"]["tkn"], tc2.token["tkn"]);
  ASSERT_EQ(sesh2Info.actual[0]["SH_INFO_RSP"]["keyCnt"], 0);

  
  // store in sesh2
  tc2.test(TestData { .request = R"({ "KV_SETQ":{ "keys":{"string2":"string2", "int2":25, "int3":3} }})"_json});
  tc2.test(sesh2Info);
  
  ASSERT_EQ(sesh2Info.actual[0]["SH_INFO_RSP"]["tkn"], tc2.token["tkn"]);
  ASSERT_EQ(sesh2Info.actual[0]["SH_INFO_RSP"]["keyCnt"], 3);

  
  // empty sesh1
  tc1.test(TestData { .request = R"({ "KV_CLEAR":{} })"_json,
                      .expected = {R"({ "KV_CLEAR_RSP":{ "st":1, "cnt":2 } })"_json}});

  tc1.test(sesh1Info);
  ASSERT_EQ(sesh1Info.actual[0]["SH_INFO_RSP"]["keyCnt"], 0);

  tc2.test(sesh2Info);
  ASSERT_EQ(sesh2Info.actual[0]["SH_INFO_RSP"]["keyCnt"], 3);
}


TEST_F(NemesisTest, New_ExpiryClearOnly)
{
	TestClient tc;

	ASSERT_TRUE(tc.openNoSession());

  TestData sesh1 {  .request =  R"({ "SH_NEW":{ "name":"sesh1", "shared":false, "expiry":{"duration":10, "deleteSession":false} }})"_json,
									  .nResponses = 1,
                    .checkResponses = false};

  tc.test(sesh1);

  tc.token = sesh1.token;

  TestData sesh1Info {.request =  R"({ "SH_INFO":{ }})"_json,
									    .nResponses = 1,
                      .checkResponses = false};
  
  tc.test(sesh1Info);

  ASSERT_EQ(sesh1.token, sesh1Info.token);
  ASSERT_FALSE(sesh1Info.actual.empty());
  ASSERT_TRUE(sesh1Info.actual[0]["SH_INFO_RSP"].contains("expiry"));
  ASSERT_TRUE(sesh1Info.actual[0]["SH_INFO_RSP"]["expiry"].contains("duration"));
  ASSERT_EQ(sesh1Info.actual[0]["SH_INFO_RSP"]["expiry"]["duration"], 10);
  ASSERT_EQ(sesh1Info.actual[0]["SH_INFO_RSP"]["expiry"]["deleteSession"], false);
}


TEST_F(NemesisTest, EndAll)
{
   // tc3 for SH_INFO_ALL because it can't have a 'tkn' which is added if open() is used
  TestClient tc1, tc2, tc3;

  ASSERT_TRUE(tc1.open());
  ASSERT_TRUE(tc2.open());
  ASSERT_TRUE(tc3.openNoSession());

  tc1.test(TestData { .request = R"({ "KV_SETQ":{ "keys":{"string":"string", "int":5} }})"_json});
  tc2.test(TestData { .request = R"({ "KV_SETQ":{ "keys":{"string":"string", "int":5} }})"_json});

  tc3.test(TestData { .request = R"({ "SH_INFO_ALL":{} })"_json, 
                      .expected = { R"({ "SH_INFO_ALL_RSP":{"st":1, "totalSessions":2, "totalKeys":4} } )"_json }});

  tc1.test(TestData { .request = R"({ "SH_END_ALL":{} } )"_json,
                      .expected = { R"({ "SH_END_ALL_RSP":{"st":1, "cnt":2} } )"_json }});

  tc3.test(TestData { .request = R"({ "SH_INFO_ALL":{} })"_json, 
                      .expected = { R"({ "SH_INFO_ALL_RSP":{"st":1, "totalSessions":0, "totalKeys":0} } )"_json }});
}


TEST_F(NemesisTest, ShExists)
{
  TestClient tc1, tc2, tc3;

  ASSERT_TRUE(tc1.open());
  ASSERT_TRUE(tc2.open());
  ASSERT_TRUE(tc3.openNoSession());

  tc3.test(TestData { .request = R"({ "SH_INFO_ALL":{} })"_json, 
                      .expected = { R"({ "SH_INFO_ALL_RSP":{"st":1, "totalSessions":2, "totalKeys":0} } )"_json }});

  auto tc1Tkn = tc1.token["tkn"].get<SessionToken>();
  auto tc2Tkn = tc2.token["tkn"].get<SessionToken>();
  auto notExistTkn = 1234;

  testjson exists;
  exists["SH_EXISTS"]["tkns"] = {tc1Tkn, tc2Tkn, notExistTkn};

  testjson rsp;
  rsp["SH_EXISTS_RSP"]["st"] = (int)nemesis::core::RequestStatus::Ok;
  rsp["SH_EXISTS_RSP"]["exist"] = {tc1Tkn, tc2Tkn};

  tc1.test(TestData { .request = exists, 
                      .expected = { rsp }});

  // end all and check again
  {
    tc1.test(TestData { .request = R"({ "SH_END_ALL":{} } )"_json,
                        .expected = { R"({ "SH_END_ALL_RSP":{"st":1, "cnt":2} } )"_json }});
    
    tc1.test(TestData { .request = exists, 
                        .expected = { R"({ "SH_EXISTS_RSP": { "st":1, "exist":[] }})"_json }});
  }
  
}



int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}