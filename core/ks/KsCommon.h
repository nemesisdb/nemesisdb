#ifndef FC_CORE_KSCOMMON_H
#define FC_CORE_KSCOMMON_H


#include <vector>
#include <ankerl/unordered_dense.h>


namespace fusion { namespace core { namespace ks {

using ksname = std::string;
using ksset = ankerl::unordered_dense::set<cachedkey>;


enum class KsQueryType : std::uint8_t
{  
  Create,
  Add,
  Get,
  Max,
  Unknown
};


const std::map<const std::string_view, std::tuple<const KsQueryType, const fcjson::value_t>> QueryNameToType = 
{
  {"KS_CREATE",   {KsQueryType::Create, fcjson::value_t::object}},
  {"KS_ADD",      {KsQueryType::Add, fcjson::value_t::object}},
  {"KS_GET",      {KsQueryType::Get, fcjson::value_t::array}}
};




}
}
}


#endif
