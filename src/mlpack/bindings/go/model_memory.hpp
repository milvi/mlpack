#ifndef MLPACK_BINDINGS_GO_MODEL_MEMORY_HPP
#define MLPACK_BINDINGS_GO_MODEL_MEMORY_HPP

#include <mlpack/core/util/param_data.hpp>

namespace mlpack { namespace bindings { namespace go {

// Les matrices restent dans std::any ; seuls les modèles pointés sont concernés.
template<typename T>
void ModelMemory(util::ParamData& data, const void* action, void* output)
{
  if constexpr (std::is_pointer_v<T>)
  {
    T& value = *std::any_cast<T>(&data.value);
    if (output != nullptr)
      *static_cast<void**>(output) = value;
    else if (action != nullptr)
      value = nullptr;
    else
    {
      delete value;
      value = nullptr;
    }
  }
  else if (output != nullptr)
    *static_cast<void**>(output) = nullptr;
}

}}}
#endif
