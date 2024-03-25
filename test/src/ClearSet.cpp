#include "useful/TestCommon.h"

using namespace nemesis::test;


TEST_F(NemesisTest, Invalid)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	{
		TestData td {MakeTestData(R"({ "KV_CLEAR_SET":{} })"_json)};
		tc.test(td);

		ASSERT_TRUE(td.actual[0]["KV_CLEAR_SET_RSP"]["st"] == RequestStatus::ParamMissing); 
	}	


	{
		TestData td {MakeTestData(R"({ "KV_CLEAR_SET":{"keys":""} })"_json)};
		tc.test(td);

		ASSERT_TRUE(td.actual[0]["KV_CLEAR_SET_RSP"]["st"] == RequestStatus::ValueTypeInvalid); 
	}
}


TEST_F(NemesisTest, KeysEmpty)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test({TestData { .request = R"({ "KV_CLEAR_SET":{"keys":{}} })"_json,	.expected = {R"({ "KV_CLEAR_SET_RSP":{ "st":1, "cnt":0 } })"_json} }});
}


TEST_F(NemesisTest, Valid)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test({TestData { .request = R"({ "KV_SETQ":{"keys":{"k1":"billybob"}}})"_json}});
	tc.test({TestData { .request = R"({ "KV_SETQ":{"keys":{"k2":"billybob"}}})"_json}});

	tc.test({TestData { .request = R"({ "KV_CLEAR_SET":{"keys":{"k3":"a", "k4":"b"}} })"_json,
											.expected = {R"({ "KV_CLEAR_SET_RSP":{ "st":1, "cnt":2 } })"_json} }});
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
