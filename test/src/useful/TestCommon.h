#ifndef NDB_TEST_COMMON_H
#define NDB_TEST_COMMON_H


#define NDB_UNIT_TEST_NOMONITOR			// disable session monitoring
#define NDB_UNIT_TEST_NOPORTCHECK		// rapid server shutdown/startup seems the socket isn't closed immediately (check this)


#include <thread>
#include <gtest/gtest.h>
#include <core/NemesisConfig.h>
#include <core/NemesisCommon.h>
#include <core/kv/KvServer.h>
#include <clients/utils/Client.hpp>
#include <nlohmann/json.hpp>


namespace nemesis { namespace test {

using namespace nemesis::core;
using namespace nemesis::core::kv;
using namespace fusion::client;
using namespace std::string_literals;

namespace asio = boost::asio;


using testjson = nlohmann::json;


class KvTestServer  : public ::testing::Test
{
public:
  KvTestServer(NemesisConfig config = NemesisConfig { R"({ "version":1, "kv":{ "ip":"127.0.0.1", "port":1987, "maxPayload":1024 } })"sv}) : m_config(config)
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
  NemesisConfig m_config;
  nemesis::core::kv::KvServer m_server;
};


class NemesisTest : public nemesis::test::KvTestServer
{
public:
  NemesisTest()
  {

  }
};



using Server = nemesis::test::KvTestServer;


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
	testjson request;
	std::vector<testjson> expected;
	std::vector<testjson> actual;
	std::size_t nResponses{0};
	bool checkToken{false};
	bool checkResponses{true};
	testjson token;	
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
				responses.emplace_back (testjson::parse(response.msg));
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
		ASSERT_FALSE(td.expected.size() && td.nResponses) << "Set either expected or nResponses";

		responses.clear();
		td.actual.clear();

		if (!token.is_null() && td.request.begin().value().is_object())
			td.request.begin().value().insert(token.cbegin(), token.cend());

		// expected responses are the JSON (TestData::expected) or a count (TestData::nResponses)
		// this can be 0 because SETQ/ADDQ only response on error
		const auto expected = td.expected.empty() ? td.nResponses : td.expected.size();

		if (expected)
			latch = std::make_unique<std::latch>(expected);

		ws->send(td.request.dump());
		
		if (expected)
		{
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

					// only remove tkn if responses are checked
					if (!td.checkToken && td.checkResponses)
						rsp.begin().value().erase("tkn");
				}

				if (td.checkResponses)
					EXPECT_TRUE(std::any_of(td.expected.cbegin(), td.expected.cend(), [&rsp](auto& expected){ return expected == rsp; })) << rsp;
			}

			td.actual = std::move(responses);
		}
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

			testjson sesh{{"SH_NEW", {{"name","test"}}}};
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
		std::vector<testjson> responses;
		std::mutex responsesMux;
		testjson token;
		bool removeTokenInRsp{false};
};

}
}

#endif
