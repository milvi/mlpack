// Généré par tools/generate-native.py ; conserver ce fichier dans Git.
#define BINDING_TYPE BINDING_TYPE_GO
#include <mlpack/methods/hmm/hmm_generate_main.cpp>
#include <mlpack/bindings/go/capi/io_util.hpp>
#include <mlpack/bindings/go/capi/ownership.hpp>
#include <mlpack/bindings/go/capi/errors.hpp>

extern "C" void mlpackHmmGenerate(void* params, void* timers)
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

extern "C" void mlpackSetHMMModelPtr(void* params, const char* id, void* value)
{
  try
  {
    mlpack::bindings::go::BorrowModel(params, value);
    static_cast<mlpack::util::Params*>(params)->Get<HMMModel*>(id) = static_cast<HMMModel*>(value);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

extern "C" void* mlpackGetHMMModelPtr(void* params, const char* id)
{
  try
  {
    auto* value = static_cast<mlpack::util::Params*>(params)->Get<HMMModel*>(id);
    mlpack::bindings::go::ExportModel(params, value);
    return value;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return nullptr;
}

extern "C" void mlpackDeleteHMMModelPtr(void* value)
{
  try
  {
    delete static_cast<HMMModel*>(value);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}
