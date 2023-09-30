#include "useful/TestCommon.h"

using namespace fusion::test;


static void setData()
{
  const std::vector<TestData> data = 
	{
		{TestData {   .request = R"({ "KV_SET":{"myarray":["a", "b"], "mystring":"string", "myinteger":5, "mydecimal":5.5, "myobject":{"user":"toad"} } })"_json,
                  .expected = {
                                R"({ "KV_SET_RSP":{ "st":20, "k":"myarray" } })"_json,
                                R"({ "KV_SET_RSP":{ "st":20, "k":"mystring" } })"_json,
                                R"({ "KV_SET_RSP":{ "st":20, "k":"myinteger" } })"_json,
                                R"({ "KV_SET_RSP":{ "st":20, "k":"mydecimal"} })"_json,
                                R"({ "KV_SET_RSP":{ "st":20, "k":"myobject"} })"_json
                              }
                }
      }
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, GetScalar)
{
  setData();
  
  const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_GET":["mystring", "myinteger", "mydecimal"] })"_json,
								.expected = {
															R"({ "KV_GET_RSP":{ "st":1, "mystring":"string" } })"_json,
														 	R"({ "KV_GET_RSP":{ "st":1, "myinteger":5 } })"_json,
															R"({ "KV_GET_RSP":{ "st":1, "mydecimal":5.5 } })"_json
														}
							}
		}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, GetStructured)
{
  const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_GET":["myarray", "myobject"] })"_json,
								.expected = {
															R"({ "KV_GET_RSP":{ "st":1, "myarray":["a", "b"] } })"_json,
														 	R"({ "KV_GET_RSP":{ "st":1, "myobject":{"user":"toad"} } })"_json
														}
							}
		}
	};

  setData();
  
  TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, ShortKey)
{
  const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_GET":["myinteger", "short"] })"_json,
								.expected = {
															R"({ "KV_GET_RSP":{ "st":1, "myinteger":5 } })"_json,  
														 	R"({ "KV_GET_RSP":{ "st":25, "k":"short" } })"_json
														}
							}
		}
	};

  setData();

	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, KeyInvalidType)
{
  const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_GET":["myinteger", 123] })"_json,
								.expected = {
															R"({ "KV_GET_RSP":{ "st":1, "myinteger":5 } })"_json,  
														 	R"({ "KV_GET_RSP":{ "st":27, "k":"" } })"_json
														}
							}
		}
	};

  setData();
  
	TestClient tc;

	ASSERT_TRUE(tc.open());

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, KeyNotExist)
{
	TestClient tc;

	ASSERT_TRUE(tc.open());
	
	tc.test(TestData { .request = R"({ "KV_GET":["imnothere"] })"_json,	.expected = {R"({ "KV_GET_RSP":{ "st":22, "k":"imnothere" } })"_json} });
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}