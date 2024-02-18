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


TEST_F(TsSeriesTest, Single)
{

}



int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
