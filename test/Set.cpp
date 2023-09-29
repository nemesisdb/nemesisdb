#include "useful/TestCommon.h"
#include <fusion/client/client.hpp>
#include <barrier>


using namespace fusion::test;
using namespace fusion::client;


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

const std::string host = "127.0.0.1";
const int port = 1987;

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


	bool open (const std::string& host, const int port)
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
		{
			EXPECT_TRUE(td.expected[0] == responses[0]) << responses[0];			
		}			
		else
		{
			for (auto& rsp : responses)
				//std::any_of(td.expected.cbegin(), td.expected.cend(), [&rsp](auto& expected){ std::cout << expected << "==" << rsp << '\n'; return false; });
				EXPECT_TRUE(std::any_of(td.expected.cbegin(), td.expected.cend(), [&rsp](auto& expected){ return expected == rsp; }));
		}
	}

	Ioc ioc;
	Client client;
	WebSocketSession ws;
	std::unique_ptr<std::latch> latch;
	std::vector<json> responses;
	std::mutex responsesMux;
};


TEST_F(FusionTest, Scalar)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_SET":{"string1":"asda"}})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"string1" } })"_json} }},
		{TestData { .request = R"({ "KV_SET":{"integer1":1}})"_json,			.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"integer1" } })"_json} }},
		{TestData { .request = R"({ "KV_SET":{"decimal1":1.5}})"_json,		.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"decimal1" } })"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open(host, port));

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, Object)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_SET":{"object1": { "a":"a", "b":2 } }})"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"object1" } })"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open(host, port));

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, Array)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_SET":{"array1":["a", "b"] } })"_json,	.expected = {R"({ "KV_SET_RSP":{ "st":20, "k":"array1" } })"_json} }}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open(host, port));

	for(auto& d : data)
		tc.test(d);
}


TEST_F(FusionTest, MultipleVarious)
{
	const std::vector<TestData> data = 
	{
		{TestData { .request = R"({ "KV_SET":{"array1":["a", "b"], "array2":[1.5], "myinta":5, "someobject":{"user":"toad"} } })"_json,
								.expected = {
															R"({ "KV_SET_RSP":{ "st":20, "k":"array1" } })"_json,
														 	R"({ "KV_SET_RSP":{ "st":20, "k":"array2" } })"_json,
															R"({ "KV_SET_RSP":{ "st":20, "k":"myinta" } })"_json,
															R"({ "KV_SET_RSP":{ "st":20, "k":"someobject"} })"_json
														}
							}
		}
	};


	TestClient tc;

	ASSERT_TRUE(tc.open(host, port));

	for(auto& d : data)
		tc.test(d);
}


int main (int argc, char ** argv)
{
	testing::InitGoogleTest(&argc, argv);	

	return RUN_ALL_TESTS();	
}
