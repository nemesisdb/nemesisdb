#include "../useful/TestCommon.h"
#include <core/ts/OrderedSeries.h>
#include <core/LogFormatter.h>
#include <string_view>

using namespace nemesis::test;
using namespace nemesis::core::ts;

static plog::ColorConsoleAppender<NdbFormatter> consoleAppender;



TEST(OrderedSeries, Add)
{
  using namespace std::chrono_literals;

  initLogger (consoleAppender);

	OrderedSeries os = OrderedSeries::create("o1");

  SeriesDuration d1(1us);
  SeriesValue v1 = "v1";

  SeriesDuration d2(2us);
  SeriesValue v2 = "v2";

  SeriesDuration d3(3us);
  SeriesValue v3 = "v3";

  SeriesDuration d4(5us);
  SeriesValue v4 = "v4";

  os.add(d1, v1);
  os.add(d2, v2);
  os.add(d3, v3);
  os.add(d4, v4);

  auto r1 = os.get(GetParams{.start = 3us});
  std::cout << r1 << '\n';

  auto r2 = os.get(GetParams{.start = 1us});
  std::cout << r2 << '\n';

  auto r3 = os.get(GetParams{.start = 3us, .end = 5us});
  std::cout << r3 << '\n';

  auto r4 = os.get(GetParams{.end = 4us});
  std::cout << r4 << '\n';

  auto r5 = os.get(GetParams{.start = 10us});
  std::cout << r5 << '\n';
}



int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
