#ifndef FC_TEST_COMMON_H
#define FC_TEST_COMMON_H

#include <thread>
#include <gtest/gtest.h>
#include <core/FusionConfig.h>
#include <core/FusionCommon.h>
#include <core/kv/KvServer.h>
#include <fusion/client/client.hpp>


namespace fusion { namespace test {

using namespace fusion::core;
using namespace fusion::core::kv;
using namespace fusion::client;

namespace asio = boost::asio;


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



using Server = fusion::test::KvTestServer;


struct Ioc
{
  Ioc() : running(false), ioc(std::make_shared<asio::io_context>(1))
  {
    thread = std::move(std::jthread{[this]()
    {
      auto workGuard = asio::make_work_guard(*ioc);
      
			running = true;
			runningCv.notify_one();

			ioc->run();
      stoppedCv.notify_one();
    }});

		std::unique_lock lck {runningMux};
		runningCv.wait(lck, [this](){ return running == true; });
  }

  ~Ioc()
  {
    if (ioc)
    {
      ioc->stop();

      std::unique_lock lck {stoppedMux};
      stoppedCv.wait(lck);
    }      
  }

	std::atomic_bool running;
  std::jthread thread;
  std::shared_ptr<asio::io_context> ioc;
  std::condition_variable stoppedCv, runningCv;
  std::mutex stoppedMux, runningMux;
	
} ioc;


struct TestData
{
	json request;
	std::vector<json> expected;
};


struct TestClient
{
	TestClient () : ioc(), client(*ioc.ioc)
	{

	}
	
	~TestClient()
	{
		if (ws)
		{
			ws->close();
		}
	}

	void onMessage (Response response)
	{
		if (response.connected)
		{
			{
				std::scoped_lock lck{responsesMux};
				responses.emplace_back (json::parse(response.msg));
			}
			
			latch->count_down();
		}
	};


	bool open (const std::string host = "127.0.0.1", const int port = 1987)
	{
		ws = client.openQueryWebSocket(host, port, "/", std::bind(&TestClient::onMessage, std::ref(*this), std::placeholders::_1));
		return ws != nullptr;
	}


	void test(const TestData& td)
	{
		responses.clear();
		latch = std::make_unique<std::latch>(td.expected.size());

		ws->send(td.request.dump());
		
		latch->wait();
		latch.reset();

		// short path
		if (td.expected.size() == 1)
			EXPECT_TRUE(td.expected[0] == responses[0]) << responses[0];			
		else
		{
			for (auto& rsp : responses)
				EXPECT_TRUE(std::any_of(td.expected.cbegin(), td.expected.cend(), [&rsp](auto& expected){ return expected == rsp; })) << rsp;
		}
	}

	Ioc ioc;
	Client client;
	WebSocketSession ws;
	std::unique_ptr<std::latch> latch;
	std::vector<json> responses;
	std::mutex responsesMux;
};

}
}

#endif
