// Généré par tools/generate-native.py ; conserver ce fichier dans Git.
#define BINDING_TYPE BINDING_TYPE_GO
#include <mlpack/methods/neighbor_search/kfn_main.cpp>
#include <mlpack/bindings/go/capi/io_util.hpp>
#include <mlpack/bindings/go/capi/ownership.hpp>
#include <mlpack/bindings/go/capi/errors.hpp>

extern "C" void mlpackKfn(void* params, void* timers)
{
  try
  {
    mlpack::util::Params& p = *static_cast<mlpack::util::Params*>(params);
    mlpack::util::Timers& t = *static_cast<mlpack::util::Timers*>(timers);
    BINDING_FUNCTION(p, t);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

extern "C" void mlpackSetKFNModelPtr(void* params, const char* id, void* value)
{
  try
  {
    mlpack::bindings::go::BorrowModel(params, value);
    static_cast<mlpack::util::Params*>(params)->Get<KFNModel*>(id) = static_cast<KFNModel*>(value);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

extern "C" void* mlpackGetKFNModelPtr(void* params, const char* id)
{
  try
  {
    auto* value = static_cast<mlpack::util::Params*>(params)->Get<KFNModel*>(id);
    mlpack::bindings::go::ExportModel(params, value);
    return value;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return nullptr;
}

extern "C" void mlpackDeleteKFNModelPtr(void* value)
{
  try
  {
    delete static_cast<KFNModel*>(value);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}
