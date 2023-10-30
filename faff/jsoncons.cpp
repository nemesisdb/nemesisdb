#include <string>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>
#include <ankerl/unordered_dense.h>

using json = jsoncons::ojson;

using cachedkey = std::string;
using cachedvalue = json;

using Map = ankerl::unordered_dense::segmented_map<cachedkey, cachedvalue>;



void set (Map& m, json&& j)
{
  // type and command name
  if (j.is_object())
  {
    auto& cmdName = j.object_range().cbegin()->key();
    auto& cmd = j.at(cmdName);

    std::cout << cmdName << '\n';

    // contains
    std::cout << j.contains("tkn") << '\n';
    
    // erase tkn
    cmd.erase("tkn");

    // store in map and rsp
    
    json rsp;
    rsp["KV_SET_RSP"]["tkn"] = "1234";

    for (const auto& i : cmd.at("keys").object_range())
    {
      auto [ignore, inserted] = m.insert_or_assign(i.key(), std::move(i.value()));
      
      rsp["KV_SET_RSP"]["keys"][i.key()] = inserted ? 20 : 21;
    }
      
    std::cout << rsp.to_string() << '\n';
  }

}


void get(Map& m, json&& j)
{
  if (j.is_object())
  {
    json rsp;
    rsp["KV_GET_RSP"]["tkn"] = "1234";

    auto& cmdName = j.object_range().cbegin()->key();
    auto& cmd = j.at(cmdName);

    cmd.erase("tkn");

    for (const auto& i : cmd.at("keys").array_range())
    {
      if (i.is_string())
      {
        const auto& k = i.as_string();
        if (auto it = m.find(k) ; it != m.cend())
          rsp["KV_GET_RSP"]["keys"][k] = it->second;
        else
          rsp["KV_GET_RSP"]["keys"][k] = json::null();
      }
    }

    std::cout << rsp.to_string() << '\n';
  }
}


int main (int argc, char ** argv)
{
  std::string qSet = R"({
                      "KV_SET":
                      {
                        "tkn":"1234",
                        "keys":
                        {
                          "int":1,
                          "string":"str",
                          "bool":true,
                          "array":[1,2,3],
                          "object":
                          {
                            "username":"a"
                          }
                        }
                      }
                    })";

  std::string qGet = R"({
                      "KV_GET":
                      {
                        "tkn":"1234",
                        "keys":["int","string","bool","array","object"]
                      }
                    })";  

  Map m;

  auto jSet = json::parse(qSet);
  set(m, std::move(jSet));

  auto jGet = json::parse(qGet);
  get(m, std::move(jGet));
  
  return 0;
}