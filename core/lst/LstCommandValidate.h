#ifndef NDB_CORE_LSTCMDVALIDATE_H
#define NDB_CORE_LSTCMDVALIDATE_H

#include <tuple>
#include <string_view>
#include <core/NemesisCommon.h>
#include <core/lst/LstCommands.h>
#include <core/lst/LstCommon.h>


namespace nemesis { namespace lst {

  using namespace std::literals;

  using namespace nemesis::lst::cmds;
  using Validity = std::tuple<bool, njson>;


  static Validity makeValid ()
  {
    const static Validity result {true, njson{}};
    return result;
  }
  

  static Validity makeInvalid (njson err)
  {
    return Validity{false, std::move(err)};
  }


  template<typename Cmds>
  static Validity validateCreate (const njson& request)
  {
    auto [valid, err] = isValid(Cmds::create.rsp, request.at(Cmds::create.req), { {Param::required("name", JsonString)} });
    return valid ? makeValid() : makeInvalid(std::move(err));
  }


  template<typename Cmds>
  static Validity validateDelete (const njson& request)
  {
    auto [valid, err] = isValid(Cmds::del.rsp, request.at(Cmds::del.req), { {Param::required("name", JsonString)} });
    return valid ? makeValid() : makeInvalid(std::move(err));
  }


  template<typename Cmds>
  static Validity validateExist (const njson& req)
  {
    auto [valid, err] = isValid(Cmds::exist.rsp, req.at(Cmds::exist.req), { {Param::required("name", JsonString)}});
    return valid ? makeValid() : makeInvalid(std::move(err));
  }


  template<typename Cmds>
  static Validity validateRemove (const njson& req)
  {
    auto checkRng = [](const njson& body) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (!body.contains("rng"))
        return {RequestStatus::Ok, ""};

      if (const auto nDims = body.at("rng").size(); !(nDims == 1 || nDims == 2)) [[unlikely]]
        return {RequestStatus::ValueSize, "rng"};
      else
      {
        const auto rngArray = body.at("rng").array_range();
        const auto first = rngArray.begin();
        
        if (!first->is_uint64())
          return {RequestStatus::ValueTypeInvalid, "rng"};
        else if (nDims == 2)
        {
          if (const auto second = std::next(first); !second->is_uint64())
            return {RequestStatus::ValueTypeInvalid, "rng"};
          else
          {
            const auto start = first->as<std::size_t>();
            const auto stop = second->as<std::size_t>();

            if (start > stop)
              return {RequestStatus::CommandSyntax, "start > stop"};
          }
        }
      
        return {RequestStatus::Ok, ""};
      }
    };

    auto [valid, err] = isValid(Cmds::remove.rsp, req.at(Cmds::remove.req), { {Param::required("name", JsonString)},
                                                                              {Param::optional("head", JsonBool)},
                                                                              {Param::optional("tail", JsonBool)},
                                                                              {Param::optional("rng", JsonArray)}}, checkRng);
    return valid ? makeValid() : makeInvalid(std::move(err));
  }


  template<typename Cmds>
  static Validity validateAdd (const njson& req)
  {
    auto itemsValid = [](const njson& body) -> std::tuple<RequestStatus, const std::string_view>
    {
      for(const auto& item : body.at("items").array_range())
        if (!Cmds::isTypeValid(item.type()))
          return {RequestStatus::ValueTypeInvalid, "items"};
      
      return {RequestStatus::Ok, ""};
    };

    auto[valid, err] = isValid(Cmds::add.rsp, req.at(Cmds::add.req), {  {Param::required("name", JsonString)},
                                                                          {Param::optional("pos",  JsonUInt)},
                                                                          {Param::required("items", JsonArray)} }, itemsValid);
    return valid ? makeValid() : makeInvalid(std::move(err));
  }


  template<typename Cmds>
  static Validity validateSetRange (const njson& req)
  {
    auto itemsValid = [](const njson& body) -> std::tuple<RequestStatus, const std::string_view>
    {
      for(const auto& item : body.at("items").array_range())
        if (!Cmds::isTypeValid(item.type()))
          return {RequestStatus::ValueTypeInvalid, "items"};
      
      return {RequestStatus::Ok, ""};
    };


    ValidateParams params = {{Param::required("name", JsonString)},
                             {Param::required("items", JsonArray)},
                             {Param::required("pos", JsonUInt)}};

    auto [valid, err] = isValid(Cmds::setRng.rsp, req.at(Cmds::setRng.req), params, itemsValid);
    return valid ? makeValid() : makeInvalid(std::move(err));
  }


  template<typename Cmds>
  static Validity validateGet (const njson& req)
  {
    auto [valid, err] = isValid(Cmds::get.rsp, req.at(Cmds::get.req), { {Param::required("name", JsonString)},
                                                                        {Param::optional("pos",  JsonUInt)}});
    return valid ? makeValid() : makeInvalid(std::move(err));
  }


  template<typename Cmds>
  static Validity validateGetRange (const njson& req)
  {
    auto checkRng = [](const njson& body) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (const auto nDims = body.at("rng").size(); !(nDims == 1 || nDims == 2)) [[unlikely]]
        return {RequestStatus::ValueSize, "rng"};
      else
      {
        const auto rngArray = body.at("rng").array_range();
        const auto first = rngArray.begin();
        
        if (!first->is_uint64())
          return {RequestStatus::ValueTypeInvalid, "rng"};
        else if (nDims == 2)
        {
          if (const auto second = std::next(first); !second->is_uint64())
            return {RequestStatus::ValueTypeInvalid, "rng"};
          else
          {
            const auto start = first->as<std::size_t>();
            const auto stop = second->as<std::size_t>();

            if (start > stop)
              return {RequestStatus::CommandSyntax, "start > stop"};
          }
        }
      }
      
      return {RequestStatus::Ok, ""};
    };


    auto [valid, err] = isValid(Cmds::getRng.rsp, req.at(Cmds::getRng.req), { {Param::required("name", JsonString)},
                                                                              {Param::required("rng",  JsonArray)}}, checkRng);
    return valid ? makeValid() : makeInvalid(std::move(err));
  }


  template<typename Cmds>
  static Validity validateLength (const njson& req)
  {
    auto [valid, err] = isValid(Cmds::len.rsp, req.at(Cmds::len.req), { {Param::required("name", JsonString)}});
    return valid ? makeValid() : makeInvalid(std::move(err));
  }


  template<typename Cmds>
  static Validity validateClear (const njson& req)
  {
    auto checkRange = [](const njson& body) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (const auto nRng = body.at("rng").size(); !(nRng == 2 || nRng == 1))
        return {RequestStatus::CommandSyntax, "rng"};
      else
      {
        const auto rngArray = body.at("rng").array_range();
        const auto first = rngArray.begin();

        if (!first->is_uint64())
            return {RequestStatus::ValueTypeInvalid, "rng"};

        if (nRng == 2)
        {
          const auto second = std::next(first);
          if (!(first->is_uint64() && second->is_uint64()))
            return {RequestStatus::ValueTypeInvalid, "rng"};
        }
      }

      return {RequestStatus::Ok, ""};
    };

    auto [valid, err] = isValid(Cmds::clear.rsp, req.at(Cmds::clear.req), { {Param::required("name", JsonString)},
                                                                          {Param::required("rng", JsonArray)}}, checkRange);
    return valid ? makeValid() : makeInvalid(std::move(err));
  }


  // template<typename Cmds>
  // static Validity validateSwap (const njson& req)
  // {
  //   auto [valid, err] = isValid(Cmds::swap.rsp, req.at(Cmds::SwapReq), { {Param::required("name", JsonString)},
  //                                                                       {Param::required("posA", JsonUInt)},
  //                                                                       {Param::required("posB", JsonUInt)}});
  //   return valid ? makeValid() : makeInvalid(std::move(err));
  // }
}
}

#endif

