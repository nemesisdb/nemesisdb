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



class TsSeriesTest  : public ::testing::Test
{
public:
  TsSeriesTest()
  {

  }

	virtual ~TsSeriesTest() = default;


  virtual void SetUp() override
	{
    initLogger (consoleAppender);
	}


	void TearDown() override
	{
		
	}


protected:

  void addSimpleData(const SeriesName& name, Series& s)
  {
    std::stringstream ss;
    ss << R"(
              {
                "TS_ADD":
                {
                  "ts":")" << name << R"(",
                  "t":[1,2,3,5],
                  "v":["1","2","3","5"]
                }
              }
            )";


    njson query = njson::parse(ss.str());

    std::cout << query << '\n';

    ASSERT_EQ(s.add(query.at("TS_ADD")).status, TsStatus::Ok);
  }
};


TEST_F(TsClassTest, GetSingle)
{
	OrderedSeries os = OrderedSeries::create("os1");

  addSimpleData(os);

  auto r1 = os.get(GetParams{.start = 3});
  std::cout << r1 << '\n';

  auto r2 = os.get(GetParams{.start = 1});
  std::cout << r2 << '\n';

  auto r3 = os.get(GetParams{.start = 3, .end = 5});
  std::cout << r3 << '\n';

  auto r4 = os.get(GetParams{.end = 4});
  std::cout << r4 << '\n';

  auto r5 = os.get(GetParams{.start = 10});
  std::cout << r5 << '\n';

  auto r6 = os.get(GetParams{.start = 20});
  std::cout << r6 << '\n';

  auto r7 = os.get(GetParams{.start = 20, .end = 25});
  std::cout << r7 << '\n';
}


/* 
TEST_F(TsSeriesTest, GetMultiple)
{
  Series s;

  ASSERT_TRUE(s.create("os1").status);
  ASSERT_TRUE(s.containsSeries("os1"));

  addSimpleData("os1", s);

  
  {
    const auto query1 = njson::parse(R"( { "TS_GET":{"ts":["os1"], "rng":[1,5]} } )");
    const auto res1 = s.get(query1.at("TS_GET"));
    
    ASSERT_EQ(res1.status, TsStatus::Ok);
    ASSERT_TRUE(res1.rsp.contains("t"));
    ASSERT_TRUE(res1.rsp.contains("v"));

    ASSERT_EQ(res1.rsp.at("t").size(), 4);
    ASSERT_EQ(res1.rsp.at("t").size(), res1.rsp.at("v").size());
    
    std::cout << res1.rsp << '\n';
  }

  // same as before but no end set
  {
    const auto query1 = njson::parse(R"( { "TS_GET":{"ts":["os1"], "rng":[1]} } )");
    const auto res1 = s.get(query1.at("TS_GET"));
    
    ASSERT_EQ(res1.status, TsStatus::Ok);
    ASSERT_TRUE(res1.rsp.contains("t"));
    ASSERT_TRUE(res1.rsp.contains("v"));

    ASSERT_EQ(res1.rsp.at("t").size(), 4);
    ASSERT_EQ(res1.rsp.at("t").size(), res1.rsp.at("v").size());
    
    std::cout << res1.rsp << '\n';
  }
}
 */


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
