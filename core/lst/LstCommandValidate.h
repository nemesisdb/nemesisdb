#ifndef NDB_CORE_LSTCMDVALIDATE_H
#define NDB_CORE_LSTCMDVALIDATE_H

#include <tuple>
#include <string_view>
#include <core/NemesisCommon.h>
#include <core/lst/LstCommands.h>
#include <core/lst/LstCommon.h>


namespace nemesis { namespace lst {

  using namespace nemesis::lst::cmds;
  //using Validity = std::tuple<bool, njson>;


  template<typename Cmds>
  static RequestStatus validateCreate (const njson& request)
  {
    const auto status = isValid(Cmds::create.rsp, request.at(Cmds::create.req), { {Param::required("name", JsonString)} });
    return status;
  }


  template<typename Cmds>
  static RequestStatus validateDelete (const njson& request)
  {
    const auto status = isValid(Cmds::del.rsp, request.at(Cmds::del.req), { {Param::required("name", JsonString)} });
    return status;
  }


  template<typename Cmds>
  static RequestStatus validateExist (const njson& req)
  {
    const auto status = isValid(Cmds::exist.rsp, req.at(Cmds::exist.req), { {Param::required("name", JsonString)}});
    return status;
  }


  template<typename Cmds>
  static RequestStatus validateRemove (const njson& req)
  {
    auto checkRng = [](const njson& body) -> RequestStatus
    {
      if (!body.contains("rng"))
        return RequestStatus::Ok;

      if (const auto nDims = body.at("rng").size(); !(nDims == 1 || nDims == 2)) [[unlikely]]
        return RequestStatus::ValueSize;
      else
      {
        const auto rngArray = body.at("rng").array_range();
        const auto first = rngArray.begin();
        
        if (!first->is_uint64())
          return RequestStatus::ValueTypeInvalid;
        else if (nDims == 2)
        {
          if (const auto second = std::next(first); !second->is_uint64())
            return RequestStatus::ValueTypeInvalid;
          else
          {
            const auto start = first->as<std::size_t>();
            const auto stop = second->as<std::size_t>();

            if (start > stop)
              return RequestStatus::CommandSyntax;
          }
        }
      
        return RequestStatus::Ok;
      }
    };

    const auto status = isValid(Cmds::remove.rsp, req.at(Cmds::remove.req), { {Param::required("name", JsonString)},
                                                                              {Param::optional("head", JsonBool)},
                                                                              {Param::optional("tail", JsonBool)},
                                                                              {Param::optional("rng", JsonArray)}}, checkRng);
    return status;
  }


  template<typename Cmds>
  static RequestStatus validateAdd (const njson& req)
  {
    auto itemsValid = [](const njson& body) -> RequestStatus
    {
      for(const auto& item : body.at("items").array_range())
        if (!Cmds::isTypeValid(item.type()))
          return RequestStatus::ValueTypeInvalid;
      
      return RequestStatus::Ok;
    };

    const auto status = isValid(Cmds::add.rsp, req.at(Cmds::add.req), { {Param::required("name", JsonString)},
                                                                        {Param::optional("pos",  JsonUInt)},
                                                                        {Param::required("items", JsonArray)} }, itemsValid);
    return status;
  }


  template<typename Cmds>
  static RequestStatus validateSetRange (const njson& req)
  {
    auto itemsValid = [](const njson& body) -> RequestStatus
    {
      for(const auto& item : body.at("items").array_range())
        if (!Cmds::isTypeValid(item.type()))
          return RequestStatus::ValueTypeInvalid;
      
      return RequestStatus::Ok;
    };


    ValidateParams params = {{Param::required("name", JsonString)},
                             {Param::required("items", JsonArray)},
                             {Param::required("pos", JsonUInt)}};

    const auto status = isValid(Cmds::setRng.rsp, req.at(Cmds::setRng.req), params, itemsValid);
    return status;
  }


  template<typename Cmds>
  static RequestStatus validateGet (const njson& req)
  {
    const auto status = isValid(Cmds::get.rsp, req.at(Cmds::get.req), { {Param::required("name", JsonString)},
                                                                        {Param::optional("pos",  JsonUInt)}});
    return status;
  }


  template<typename Cmds>
  static RequestStatus validateGetRange (const njson& req)
  {
    auto checkRng = [](const njson& body) -> RequestStatus
    {
      if (const auto nDims = body.at("rng").size(); !(nDims == 1 || nDims == 2)) [[unlikely]]
        return RequestStatus::ValueSize;
      else
      {
        const auto rngArray = body.at("rng").array_range();
        const auto first = rngArray.begin();
        
        if (!first->is_uint64())
          return RequestStatus::ValueTypeInvalid;
        else if (nDims == 2)
        {
          if (const auto second = std::next(first); !second->is_uint64())
            return RequestStatus::ValueTypeInvalid;
          else
          {
            const auto start = first->as<std::size_t>();
            const auto stop = second->as<std::size_t>();

            if (start > stop)
              return RequestStatus::CommandSyntax;
          }
        }
      }
      
      return RequestStatus::Ok;
    };


    const auto status = isValid(Cmds::getRng.rsp, req.at(Cmds::getRng.req), { {Param::required("name", JsonString)},
                                                                              {Param::required("rng",  JsonArray)}}, checkRng);
    return status;
  }


  template<typename Cmds>
  static RequestStatus validateLength (const njson& req)
  {
    const auto status = isValid(Cmds::len.rsp, req.at(Cmds::len.req), { {Param::required("name", JsonString)}});
    return status;
  }


  template<typename Cmds>
  static RequestStatus validateClear (const njson& req)
  {
    auto checkRange = [](const njson& body) -> RequestStatus
    {
      if (const auto nRng = body.at("rng").size(); !(nRng == 2 || nRng == 1))
        return RequestStatus::CommandSyntax;
      else
      {
        const auto rngArray = body.at("rng").array_range();
        const auto first = rngArray.begin();

        if (!first->is_uint64())
            return RequestStatus::ValueTypeInvalid;

        if (nRng == 2)
        {
          const auto second = std::next(first);
          if (!(first->is_uint64() && second->is_uint64()))
            return RequestStatus::ValueTypeInvalid;
        }
      }

      return RequestStatus::Ok;
    };

    const auto status = isValid(Cmds::clear.rsp, req.at(Cmds::clear.req), { {Param::required("name", JsonString)},
                                                                            {Param::required("rng", JsonArray)}}, checkRange);
    return status;
  }


  template<typename Cmds>
  static RequestStatus validateSplice (const njson& req)
  {
    const auto status = isValid(Cmds::splice.rsp, req.at(Cmds::splice.req), { {Param::required("srcName", JsonString)},
                                                                              {Param::required("srcRng", JsonArray)},
                                                                              {Param::required("destName", JsonString)},
                                                                              {Param::optional("destPos", JsonUInt)}});
    return status;
  }
}
}

#endif

