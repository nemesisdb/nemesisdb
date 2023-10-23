#include "useful/TestCommon.h"
#include <core/NemesisCommon.h>

using namespace fusion::test;


// TEST_F(FusionTest, FirstQuery)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	std::string rsp {R"({ "KV_SERVER_INFO_RSP":{"st":1, "qryCnt":1, "version":")" + std::string{nemesis::core::FUSION_VERSION} + "\"}}"};

// 	// qryCnt is 1 because it includes the server_info account itself
// 	tc.test({TestData { .request = R"({ "KV_SERVER_INFO":{} })"_json,	.expected = {json::parse(rsp)} }});
// }


// TEST_F(FusionTest, AfterQueries)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());
	
// 	std::string rsp {R"({ "KV_SERVER_INFO_RSP":{"st":1, "qryCnt":3, "version":")" + std::string{nemesis::core::FUSION_VERSION} + "\"}}"};

// 	// qryCnt is 3: the two sets plus the server_info
// 	tc.test({TestData { .request = R"({ "KV_SET":{"string1":"billybob"}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"string1" } })"_json} }});
// 	tc.test({TestData { .request = R"({ "KV_SET":{"anotherkey":"billybob"}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"anotherkey" } })"_json} }});
// 	tc.test({TestData { .request = R"({ "KV_SERVER_INFO":{} })"_json,	.expected = {json::parse(rsp)} }});
// }


// TEST_F(FusionTest, IncorrectCommandType)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

// 	tc.test(TestData { .request = R"({ "KV_SERVER_INFO":[] })"_json,	.expected = {R"({ "KV_SERVER_INFO_RSP":{ "st":12, "k":"" } })"_json} });
// }


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
