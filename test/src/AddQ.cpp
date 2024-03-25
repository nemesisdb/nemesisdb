#include "useful/TestCommon.h"

using namespace nemesis::test;


//
// Limited on what we can do because ADDQ doesn't respond unless there's an error.
//


TEST_F(NemesisTest, AddNew)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData { .request = R"({ "KV_ADDQ":{"keys":{"string1":"asda"}}} )"_json});
	tc.test(TestData { .request = R"({ "KV_ADDQ":{"keys":{"integer1":1}}} )"_json});
	tc.test(TestData { .request = R"({ "KV_ADDQ":{"keys":{"decimal1":1.5}}} )"_json});

	
	TestData count {.request = R"({ "KV_COUNT":{} })"_json, .expected = {R"({ "KV_COUNT_RSP":{"cnt":3, "st":1} } )"_json} };
	tc.test(count);
}


TEST_F(NemesisTest, AddToExisting)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData { .request = R"({ "KV_ADDQ":{"keys":{"string1":"asda"}}} )"_json});
	tc.test(TestData { .request = R"({ "KV_ADDQ":{"keys":{"integer1":1}}} )"_json});
	tc.test(TestData { .request = R"({ "KV_ADDQ":{"keys":{"decimal1":1.5}}} )"_json});

	// attempt to overwrite
	tc.test(TestData { .request = R"({ "KV_ADDQ":{"keys":{"string1":"asda"}}} )"_json });
	tc.test(TestData { .request = R"({ "KV_ADDQ":{"keys":{"integer1":1}}} )"_json });
	tc.test(TestData { .request = R"({ "KV_ADDQ":{"keys":{"decimal1":1.5}}} )"_json });
}


TEST_F(NemesisTest, IncorrectCommandType)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());

	tc.test(TestData { .request = R"({ "KV_ADDQ":[] })"_json,	.expected = {R"({"KV_ADDQ_RSP":{"m":"","st":12}})"_json} });
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
