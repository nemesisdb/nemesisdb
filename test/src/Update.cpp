#include "useful/TestCommon.h"

using namespace nemesis::test;


static const auto set = R"({
														"KV_SET":
														{
															"keys":
															{
																"loginsValid":
																[
																	{"timestamp":1234, "location":"London"},
																	{"timestamp":1235, "location":"London"},
																	{"timestamp":1236, "location":"Paris"},
																	{"timestamp":1238, "location":"London"}
																],
																"loginsFailed":
																[
																	{"timestamp":1235, "location":"New York"},
																	{"timestamp":1235, "location":"London"},
																	{"timestamp":1237, "location":"New York"}
																],
                                "profile":
                                {
                                  "username":"Dave",
                                  "address":
                                  {
                                    "city":"London"
                                  }
                                }
															}
														}
													})"_json;


static const auto setRsp = R"({ "KV_SET_RSP":{ "keys":{"loginsValid":20, "loginsFailed":20, "profile":20} } } )"_json;


TEST_F(NemesisTest, NoData)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_UPDATE":{"key":"loginsFailed","path":"$[?(@.location == 'London')].location", "value":"Manc"} })"_json,	.expected = {R"({ "KV_UPDATE_RSP":{ "st":22, "cnt":0 } })"_json} }});
}


TEST_F(NemesisTest, MissingParams)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	{
		TestData td {MakeTestData(R"({ "KV_UPDATE":{"key":"loginsFailed", "value":"Manc"} })"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["KV_UPDATE_RSP"]["st"] == RequestStatus::ParamMissing); 
	}

	{
		TestData td {MakeTestData(R"({ "KV_UPDATE":{"key":"loginsFailed","path":"$abc"} })"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["KV_UPDATE_RSP"]["st"] == RequestStatus::ParamMissing); 
	}

	{
		TestData td {MakeTestData(R"({ "KV_UPDATE":{"path":"$abc", "value":"Manc"} })"_json)};
		tc.test(td);
		ASSERT_TRUE(td.actual[0]["KV_UPDATE_RSP"]["st"] == RequestStatus::ParamMissing); 
	}
}


// Happy
TEST_F(NemesisTest, UpdateObjectRoot)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

  tc.test(TestData { .request = set, .expected = {setRsp} });
	
  tc.test({TestData { .request = R"({ "KV_UPDATE":{"key":"profile","path":"$.username", "value":"Bob"} })"_json,	.expected = {R"({ "KV_UPDATE_RSP":{ "st":1, "cnt":1 } })"_json} }});
  tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["profile"]} })"_json, .expected = {R"({ "KV_GET_RSP":{ "keys":{"profile":{"username":"Bob", "address":{"city":"London"}} } }})"_json}});
}


TEST_F(NemesisTest, UpdateObjectNested)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

  tc.test(TestData { .request = set, .expected = {setRsp} });
	
  tc.test({TestData { .request = R"({ "KV_UPDATE":{"key":"profile","path":"$.address.city", "value":"Manc"} })"_json,	.expected = {R"({ "KV_UPDATE_RSP":{ "st":1, "cnt":1 } })"_json} }});
  tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["profile"]} })"_json, .expected = {R"({ "KV_GET_RSP":{ "keys":{"profile":{"username":"Dave", "address":{"city":"Manc"}} } }})"_json}});
}


TEST_F(NemesisTest, UpdateObjectWithObject)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

  tc.test(TestData { .request = set, .expected = {setRsp} });
	
  tc.test({TestData { .request = R"({ "KV_UPDATE":{"key":"profile","path":"$.address", "value":{ "city":"Paris"}} })"_json,	.expected = {R"({ "KV_UPDATE_RSP":{ "st":1, "cnt":1 } })"_json} }});
  tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["profile"]} })"_json, .expected = {R"({ "KV_GET_RSP":{ "keys":{"profile":{"username":"Dave", "address":{"city":"Paris"}} } }})"_json}});
}


TEST_F(NemesisTest, UpdateArrayObjectElement)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

  tc.test(TestData { .request = set, .expected = {setRsp} });
	
  tc.test({TestData { .request = R"({ "KV_UPDATE":{"key":"loginsFailed","path":"$[?(@.location == 'London')].location", "value":"Manc"} })"_json,	.expected = {R"({ "KV_UPDATE_RSP":{ "st":1, "cnt":1 } })"_json} }});
  tc.test(TestData { 	.request = R"({ "KV_GET":{ "keys":["loginsFailed"]} })"_json, .expected = {R"({ "KV_GET_RSP":{ "keys":{"loginsFailed":[ {"timestamp":1235, "location":"New York"},
                                                                                                                                              {"timestamp":1235, "location":"Manc"},
                                                                                                                                              {"timestamp":1237, "location":"New York"}]} }})"_json}});
}





int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
