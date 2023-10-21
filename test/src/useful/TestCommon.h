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
	std::vector<json> actual;
	std::size_t nResponses{1};
	bool checkToken{false};
	bool checkResponses{true};
	json token;	
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

	bool openNoSession (const std::string host = "127.0.0.1", const int port = 1987)
	{
		removeTokenInRsp = false;
		ws = client.openQueryWebSocket(host, port, "/", std::bind(&TestClient::onMessage, std::ref(*this), std::placeholders::_1));
		return ws != nullptr;
	}

	bool open (const std::string host = "127.0.0.1", const int port = 1987)
	{
		removeTokenInRsp = true;
		return openNoSession(host, port) && createSession();
	}


	void test (TestData& td)
	{		
		responses.clear();

		if (!token.is_null() && td.request.begin().value().is_object())
			td.request.begin().value().insert(token.cbegin(), token.cend());

		// expected responses are the JSON (TestData::expected) or a count (TestData::nResponses)
		// this can be 0 because SETQ/ADDQ only response on error
		const auto expected = td.expected.empty() ? td.nResponses : td.expected.size();

		if (expected)
			latch = std::make_unique<std::latch>(expected);

		ws->send(td.request.dump());
		
		if (expected)
			latch->wait();
		
		latch.reset();


		// even if checkResponses is false, we still do this to grab the token
		for (auto& rsp : responses)
		{
			// we always erase the token because setting that in TestData each time is tedious
			// so instead we grab it out of the response
			if (rsp.begin().value().contains("tkn"))
			{
				if (!rsp.begin().value().at("tkn").is_null())
					td.token["tkn"] = rsp.begin().value().at("tkn");

				if (!td.checkToken)
					rsp.begin().value().erase("tkn");
			}

			if (td.checkResponses)
				EXPECT_TRUE(std::any_of(td.expected.cbegin(), td.expected.cend(), [&rsp](auto& expected){ return expected == rsp; })) << rsp;
		}

		td.actual = std::move(responses);
	}


	void test (TestData&& data)
	{		
		TestData td = std::move(data);
		test(td);
	}


	private:
		bool createSession ()
		{
			latch = std::make_unique<std::latch>(1);

			json sesh{{"SH_NEW", {{"name","test"}}}};
			ws->send(sesh.dump());

			latch->wait();
			latch.reset();

			auto& rsp = responses[0];

			if (rsp.contains("SH_NEW_RSP") && rsp["SH_NEW_RSP"]["st"] == 1)
			{
				token["tkn"] = rsp["SH_NEW_RSP"]["tkn"];
				return true;
			}
			return false;
		}


	public:
		Ioc ioc;
		Client client;
		WebSocketSession ws;
		std::unique_ptr<std::latch> latch;
		std::vector<json> responses;
		std::mutex responsesMux;
		json token;
		bool removeTokenInRsp{false};
};

}
}

#endif
