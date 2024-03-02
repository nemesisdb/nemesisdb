#define NDB_NOLOG

#include "../useful/TestCommon.h"
#include <core/ts/OrderedSeries.h>
#include <core/ts/Series.h>
#include <core/LogFormatter.h>
#include <string_view>
#include <sstream>


using namespace nemesis::test;
using namespace nemesis::core::ts;

static plog::ColorConsoleAppender<NdbFormatter> consoleAppender;


class TsClassTest  : public ::testing::Test
{
public:
  TsClassTest()
  {

  }

	virtual ~TsClassTest() = default;


  virtual void SetUp() override
	{
    initLogger (consoleAppender);
	}


	void TearDown() override
	{
		
	}


protected:

  void addSimpleData(OrderedSeries& os)
  {
    njson times1 {json_array_arg, {1,2,3,4}};
    njson vals1 = njson::parse(R"([{"a":1},{"a":2},{"a":3},{"a":4}])");

    njson times2 {json_array_arg, {20,21,22,23,24,25,30}};
    njson vals2 = njson::parse(R"([{"a":20},{"a":21},{"a":22},{"a":23},{"a":24},{"a":25},{"a":30}])");

    os.add(times1, std::move(vals1));
    os.add(times2, std::move(vals2));
  }
};


/* Can only be tested using TsHandler because OrderedSeries and Series expect JSON to be validated
TEST_F(TsClassTest, InvalidValues)
{
  {
    njson times {json_array_arg, {1,2,3,4}};
    njson vals {json_array_arg, {"string", 3, 3.5, true}};
  }
}
 */

TEST_F(TsClassTest, GetSingle)
{
	OrderedSeries os = OrderedSeries::create("os1");

  addSimpleData(os);


  {
    GetParams params;
    params.setStart(3);

    auto r = os.get(params);

    ASSERT_EQ(r["t"].size(), 9);
    ASSERT_EQ(r["t"].size(), r["v"].size());
  }

  
  {
    GetParams params;
    params.setStart(1);

    auto r = os.get(params);

    ASSERT_EQ(r["t"].size(), 11);
    ASSERT_EQ(r["t"].size(), r["v"].size());
  }


  {
    GetParams params;
    params.setStart(3);
    params.setEnd(5);

    auto r = os.get(params);

    ASSERT_EQ(r["t"].size(), 2);
    ASSERT_EQ(r["t"].size(), r["v"].size());
  }


  {
    GetParams params;
    params.setStart(10);

    auto r = os.get(params);

    ASSERT_EQ(r["t"].size(), 7);
    ASSERT_EQ(r["t"].size(), r["v"].size());
  }


  {
    GetParams params;
    params.setStart(20);

    auto r = os.get(params);

    ASSERT_EQ(r["t"].size(), 7);
    ASSERT_EQ(r["t"].size(), r["v"].size());
  }
  

  {
    GetParams params;
    params.setStart(20);
    params.setEnd(25);

    auto r = os.get(params);

    ASSERT_EQ(r["t"].size(), 6);
    ASSERT_EQ(r["t"].size(), r["v"].size());
  }
}



int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
