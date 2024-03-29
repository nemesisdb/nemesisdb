#include "useful/TestCommon.h"

using namespace nemesis::test;


// TEST_F(NemesisTest, KeyNotExist)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"keys":{"usergroups":[1,0]}} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "usergroups":22 } })"_json} }});
// }


// TEST_F(NemesisTest, EmptyArrayTwoPositions)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":[] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "keys":{"usergroups":20} } })"_json} }});
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[1,0]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "usergroups":43 } })"_json} }});
// }


// TEST_F(NemesisTest, EmptyArrayOnePositions)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":[] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }});
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[0]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":43, "k":"usergroups" } })"_json} }});
// }


// TEST_F(NemesisTest, OneItem)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":["a"] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }});
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[1,0]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":43, "k":"usergroups" } })"_json} }});
// }


// TEST_F(NemesisTest, TwoItemsPositionsEqual)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":["a", "b"] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }}); 
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[0,0]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":1, "k":"usergroups" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KV_GET":["usergroups"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "usergroups":["a", "b"] } })"_json} }});   
// }


// TEST_F(NemesisTest, TwoItemsMoveFirst)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":["a", "b"] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }}); 
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[0,1]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":1, "k":"usergroups" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KV_GET":["usergroups"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "usergroups":["a", "b"] } })"_json} }});
// }


// TEST_F(NemesisTest, TwoItemsMoveLastToStart)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":["a", "b"] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }}); 
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[1,0]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":1, "k":"usergroups" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KV_GET":["usergroups"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "usergroups":["b", "a"] } })"_json} }});
// }


// TEST_F(NemesisTest, ThreeItemsMoveSecondToFirst)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":["a", "b", "c"] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }}); 
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[1,0]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":1, "k":"usergroups" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KV_GET":["usergroups"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "usergroups":["b", "a", "c"] } })"_json} }});
// }


// TEST_F(NemesisTest, ThreeItemsMoveLastToFirst)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":["a", "b", "c"] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }}); 
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[2,0]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":1, "k":"usergroups" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KV_GET":["usergroups"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "usergroups":["c", "a", "b"] } })"_json} }});
// }


// TEST_F(NemesisTest, ThreeItemsMoveLastToSecond)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":["a", "b", "c"] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }}); 
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[2,1]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":1, "k":"usergroups" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KV_GET":["usergroups"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "usergroups":["a", "c", "b"] } })"_json} }});
// }


// TEST_F(NemesisTest, OneItemNoNewMove)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":["a"] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }}); 
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[0]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":1, "k":"usergroups" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KV_GET":["usergroups"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "usergroups":["a"] } })"_json} }});
// }


// TEST_F(NemesisTest, TwoItemsNoNewMoveFirst)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":["a", "b"] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }}); 
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[0]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":1, "k":"usergroups" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KV_GET":["usergroups"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "usergroups":["b", "a"] } })"_json} }});
// }


// TEST_F(NemesisTest, TwoItemsNoNewMoveLast)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":["a", "b"] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }}); 
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[1]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":1, "k":"usergroups" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KV_GET":["usergroups"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "usergroups":["a", "b"] } })"_json} }});
// }


// TEST_F(NemesisTest, MoreItemsNoNewMoveFirst)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":["a", "b", "c", "d", "e", "f"] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }}); 
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[0]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":1, "k":"usergroups" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KV_GET":["usergroups"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "usergroups":["b", "c", "d", "e", "f", "a"] } })"_json} }});
// }


// TEST_F(NemesisTest, MoreItemsNoNewMoveMid)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":["a", "b", "c", "d", "e", "f"] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }}); 
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[2]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":1, "k":"usergroups" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KV_GET":["usergroups"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "usergroups":["a", "b", "d", "e", "f", "c"] } })"_json} }});
// }


// TEST_F(NemesisTest, ObjectItemsMove)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":["a", {"x":"y"}, "c"] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }}); 
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[1]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":1, "k":"usergroups" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KV_GET":["usergroups"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":1, "usergroups":["a", "c", {"x":"y"}] } })"_json} }});
// }


// TEST_F(NemesisTest, MultipleKeysObjectItemsMove)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups1":["a", "b", "c"], "usergroups2":["d", "e", "f"] }})"_json,
//                       .expected = { R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups1" } })"_json,
//                                     R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups2" } })"_json} }});

// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups1":[1], "usergroups2":[0]} })"_json,
//                       .expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":1, "k":"usergroups1" } })"_json,
//                                    R"({ "KV_ARRAY_MOVE_RSP":{ "st":1, "k":"usergroups2" } })"_json } }});

//   tc.test({TestData { .request = R"({ "KV_GET":["usergroups1", "usergroups2"] })"_json,
//                       .expected = { R"({ "KV_GET_RSP":{ "st":1, "usergroups1":["a", "c", "b"] } })"_json,
//                                     R"({ "KV_GET_RSP":{ "st":1, "usergroups2":["e", "f", "d"] } })"_json} }});
// }


// TEST_F(NemesisTest, InvalidPositionType)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

//   tc.test({TestData { .request = R"({ "KV_SET":{ "usergroups":["a", "b"] }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"usergroups" } })"_json} }}); 
// 	tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":["1", 0]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":41, "k":"usergroups" } })"_json} }});
//   tc.test({TestData { .request = R"({ "KV_ARRAY_MOVE":{"usergroups":[0, "1"]} })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":41, "k":"usergroups" } })"_json} }});
// }


// TEST_F(NemesisTest, IncorrectCommandType)
// {
// 	TestClient tc;

// 	ASSERT_TRUE(tc.open());

// 	tc.test(TestData { .request = R"({ "KV_ARRAY_MOVE":[0.0] })"_json,	.expected = {R"({ "KV_ARRAY_MOVE_RSP":{ "st":12, "k":"" } })"_json} });
// }


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
