#ifndef MLPACK_BINDINGS_GO_OWNERSHIP_HPP
#define MLPACK_BINDINGS_GO_OWNERSHIP_HPP

#include <mlpack/core/util/params.hpp>
#include <unordered_map>
#include <vector>

namespace mlpack { namespace bindings { namespace go {

struct Ownership
{
  std::vector<void*> borrowed;
  std::vector<void*> exported;
};

inline std::unordered_map<void*, Ownership>& Owners()
{
  static std::unordered_map<void*, Ownership> owners;
  return owners;
}

inline void BorrowModel(void* params, void* value)
{
  if (value != nullptr) Owners().at(params).borrowed.push_back(value);
}

inline void ExportModel(void* params, void* value)
{
  if (value != nullptr) Owners().at(params).exported.push_back(value);
}

inline void ReleaseUnexportedModels(util::Params& params)
{
  const auto found = Owners().find(&params);
  if (found == Owners().end()) return;
  const auto& ownership = found->second;
  for (auto& entry : params.Parameters())
  {
    auto& data = entry.second;
    auto types = params.functionMap.find(data.tname);
    if (types == params.functionMap.end()) continue;
    auto operation = types->second.find("ModelMemory");
    if (operation == types->second.end()) continue;
    void* pointer = nullptr;
    operation->second(data, nullptr, &pointer);
    if (pointer == nullptr ||
        std::find(ownership.borrowed.begin(), ownership.borrowed.end(), pointer) != ownership.borrowed.end() ||
        std::find(ownership.exported.begin(), ownership.exported.end(), pointer) != ownership.exported.end())
      continue;
    operation->second(data, nullptr, nullptr);
    // Un même modèle peut figurer dans plusieurs paramètres : ne le détruire qu’une fois.
    for (auto& alias : params.Parameters())
    {
      auto& other = alias.second;
      auto function = params.functionMap.at(other.tname).find("ModelMemory");
      if (function == params.functionMap.at(other.tname).end()) continue;
      void* address = nullptr;
      function->second(other, nullptr, &address);
      if (address == pointer) function->second(other, &params, nullptr);
    }
  }
  Owners().erase(found);
}

}}}
#endif
