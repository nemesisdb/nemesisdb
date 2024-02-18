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
    SeriesTime d1(1);
    SeriesValue v1 = "1";

    SeriesTime d2(2);
    SeriesValue v2 = "2";

    SeriesTime d3(3);
    SeriesValue v3 = "3";

    SeriesTime d4(5);
    SeriesValue v4 = "5";

    os.add(d1, v1);
    os.add(d2, v2);
    os.add(d3, v3);
    os.add(d4, v4);

    njson times1 {json_array_arg, {20,21,22,23,24,25,30}};
    njson vals1 {json_array_arg, {"20","21","22","23","24","25","30"}};

    os.add(times1, std::move(vals1));
  }
};



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
