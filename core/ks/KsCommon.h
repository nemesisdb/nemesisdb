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
  List,
  AddKey,
  Get,
  RemoveKey,
  ClearSet,
  DeleteSet,
  DeleteAllSets,
  SetExists,
  KeyExists,
  MoveKey,
  Max,
  Unknown
};


const std::map<const std::string_view, std::tuple<const KsQueryType, const fcjson::value_t>> QueryNameToType = 
{
  {"KS_CREATE",       {KsQueryType::Create,         fcjson::value_t::object}},
  {"KS_LIST",         {KsQueryType::List,           fcjson::value_t::object}},
  {"KS_DELETE_SET",   {KsQueryType::DeleteSet,      fcjson::value_t::object}},
  {"KS_DELETE_ALL",   {KsQueryType::DeleteAllSets,  fcjson::value_t::object}},
  {"KS_SET_EXISTS",   {KsQueryType::SetExists,      fcjson::value_t::object}},
  {"KS_KEY_EXISTS",   {KsQueryType::KeyExists,      fcjson::value_t::object}},
  {"KS_MOVE_KEY",     {KsQueryType::MoveKey,        fcjson::value_t::object}},
  {"KS_GET",          {KsQueryType::Get,            fcjson::value_t::array}},
  {"KS_ADD_KEY",      {KsQueryType::AddKey,         fcjson::value_t::object}},
  {"KS_RMV_KEY",      {KsQueryType::RemoveKey,      fcjson::value_t::object}},
  {"KS_CLEAR_SET",    {KsQueryType::ClearSet,       fcjson::value_t::object}}
};


}
}
}


#endif
