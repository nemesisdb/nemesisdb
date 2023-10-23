#ifndef NDB_CORE_KSCOMMON_H
#define NDB_CORE_KSCOMMON_H


#include <vector>
#include <ankerl/unordered_dense.h>


namespace nemesis { namespace core { namespace ks {

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


const std::map<const std::string_view, std::tuple<const KsQueryType, const njson::value_t>> QueryNameToType = 
{
  {"KS_CREATE",       {KsQueryType::Create,         njson::value_t::object}},
  {"KS_LIST",         {KsQueryType::List,           njson::value_t::object}},
  {"KS_DELETE_SET",   {KsQueryType::DeleteSet,      njson::value_t::object}},
  {"KS_DELETE_ALL",   {KsQueryType::DeleteAllSets,  njson::value_t::object}},
  {"KS_SET_EXISTS",   {KsQueryType::SetExists,      njson::value_t::object}},
  {"KS_KEY_EXISTS",   {KsQueryType::KeyExists,      njson::value_t::object}},
  {"KS_MOVE_KEY",     {KsQueryType::MoveKey,        njson::value_t::object}},
  {"KS_GET",          {KsQueryType::Get,            njson::value_t::array}},
  {"KS_ADD_KEY",      {KsQueryType::AddKey,         njson::value_t::object}},
  {"KS_RMV_KEY",      {KsQueryType::RemoveKey,      njson::value_t::object}},
  {"KS_CLEAR_SET",    {KsQueryType::ClearSet,       njson::value_t::object}}
};


}
}
}


#endif
