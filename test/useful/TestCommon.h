#ifndef FC_TEST_COMMON_H
#define FC_TEST_COMMON_H

#include <thread>
#include <gtest/gtest.h>
#include <core/FusionConfig.h>
#include <core/FusionCommon.h>
#include <core/kv/KvServer.h>


namespace fusion { namespace test {

using namespace fusion::core;
using namespace fusion::core::kv;


class KvTestServer  : public ::testing::Test
{
public:
  KvTestServer(FusionConfig config = FusionConfig { R"({ "version":1, "kv":{ "ip":"127.0.0.1", "port":1987, "maxPayload":1024 } })"_json}) : m_config(config)
  {

  }


  void SetUp() override
	{
    m_server.run(m_config);
	}


	void TearDown() override
	{
		m_server.stop();
	}

private:
  FusionConfig m_config;
  fusion::core::kv::KvServer m_server;
};

class FusionTest : public fusion::test::KvTestServer
{
public:
  FusionTest()
  {

  }
};


}
}

#endif
