/**
 * @file bindings/go/capi/io_util.cpp
 * @author Yasmine Dumouchel
 * @author Yashwant Singh
 *
 * Utility function for Go to set and get parameters to and from the IO.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#include <mlpack/bindings/go/capi/io_util.h>
#include "io_util.hpp"
#include "ownership.hpp"
#include "errors.hpp"
#include <memory>
#include <mlpack/core/util/io.hpp>

namespace mlpack {

extern "C" {

bool mlpackCopyParamVectorInt(void* params, const char* identifier,
                             long long* destination, const size_t length)
{
  try
  {
    if (params == nullptr || identifier == nullptr ||
        (length != 0 && destination == nullptr))
      return false;
    try
    {
      const auto& source = static_cast<util::Params*>(params)->Get<std::vector<int>>(identifier);
      if (source.size() != length)
        return false;
      for (size_t i = 0; i < length; ++i)
        destination[i] = source[i];
      return true;
    }
    catch (...) { throw; }
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Get a new Params object for the given binding name.
 */
void* mlpackGetParams(const char* bindingName)
{
  try
  {
    auto p = std::make_unique<util::Params>(IO::Parameters(bindingName));
    bindings::go::Owners().emplace(p.get(), bindings::go::Ownership{});
    return p.release();
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return nullptr;
}

/**
 * Get a new Timers object.
 */
void* mlpackGetTimers()
{
  try
  {
    util::Timers* t = new util::Timers();
    return (void*) t;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return nullptr;
}

/**
 * Delete the given Params object.
 */
void mlpackCleanParams(void* params)
{
  try
  {
    util::Params* p = (util::Params*) params;
    if (p != nullptr) bindings::go::ReleaseUnexportedModels(*p);
    delete p;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Delete the given Timers object.
 */
void mlpackCleanTimers(void* timers)
{
  try
  {
    util::Timers* t = (util::Timers*) timers;
    delete t;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Set the double parameter to the given value.
 */
void mlpackSetParamDouble(void* params, const char* identifier, double value)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    util::SetParam(p, identifier, value);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Set the int parameter to the given value.
 */
void mlpackSetParamInt(void* params, const char* identifier, int value)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    util::SetParam(p, identifier, value);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Set the float parameter to the given value.
 */
void mlpackSetParamFloat(void* params, const char* identifier, float value)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    util::SetParam(p, identifier, value);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Set the bool parameter to the given value.
 */
void mlpackSetParamBool(void* params, const char* identifier, bool value)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    util::SetParam(p, identifier, value);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Set the string parameter to the given value.
 */
void mlpackSetParamString(void* params,
                          const char* identifier,
                          const char* value)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    p.Get<std::string>(identifier) = value;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Set the int vector parameter to the given value.
 */
void mlpackSetParamVectorInt(void* params,
                             const char* identifier,
                             const long long* ints,
                             const size_t length)
{
  try
  {
    util::Params& p = *((util::Params*) params);
  
    // Create a std::vector<int> object; unfortunately this requires copying the
    // vector elements.
    std::vector<int> vec(length);
    for (size_t i = 0; i < length; ++i)
      vec[i] = ints[i];
  
    p.Get<std::vector<int>>(identifier) = std::move(vec);
    p.SetPassed(identifier);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Call IO::SetParam<std::vector<std::string>>() to set the length.
 */
void mlpackSetParamVectorStrLen(void* params,
                                const char* identifier,
                                const size_t length)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    p.Get<std::vector<std::string>>(identifier).clear();
    p.Get<std::vector<std::string>>(identifier).resize(length);
    p.SetPassed(identifier);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Set the string vector parameter to the given value.
 */
void mlpackSetParamVectorStr(void* params,
                             const char* identifier,
                             const char* str,
                             const size_t element)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    p.Get<std::vector<std::string>>(identifier)[element] = std::string(str);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Set the parameter to the given value, given that the type is a pointer.
 */


/**
 * Check if IO has a specified parameter.
 */
bool mlpackHasParam(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Has(identifier);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Get the string parameter associated with specified identifier.
 */
const char* mlpackGetParamString(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<std::string>(identifier).c_str();
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return nullptr;
}

/**
 * Get the double parameter associated with specified identifier.
 */
double mlpackGetParamDouble(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<double>(identifier);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Get the int parameter associated with specified identifier.
 */
int mlpackGetParamInt(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<int>(identifier);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Get the bool parameter associated with specified identifier.
 */
bool mlpackGetParamBool(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<bool>(identifier);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Get the vector<int> parameter associated with specified identifier.
 */


/**
 * Get the vector<string> parameter associated with specified identifier.
 */
const char* mlpackGetVecStringPtr(void* params,
                                  const char* identifier,
                                  const size_t i)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<std::vector<std::string>>(identifier)[i].c_str();
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return nullptr;
}

/**
 * Get the vector<int> parameter's size.
 */
size_t mlpackVecIntSize(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<std::vector<int>>(identifier).size();
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Get the vector<string> parameter's size.
 */
size_t mlpackVecStringSize(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<std::vector<std::string>>(identifier).size();
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Set parameter as passed.
 */
void mlpackSetPassed(void* params, const char* name)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    p.SetPassed(name);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Enable timing.
 */
void mlpackEnableTimers()
{
  try
  {
    Timer::EnableTiming();
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Disable backtraces.
 */
void mlpackDisableBacktrace()
{
  try
  {
    Log::Fatal.backtrace = false;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Turn verbose output on.
 */
void mlpackEnableVerbose()
{
  try
  {
    Log::Info.ignoreInput = false;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Turn verbose output off.
 */
void mlpackDisableVerbose()
{
  try
  {
    Log::Info.ignoreInput = true;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

} // extern C

} // namespace mlpack

extern "C" const char* milviLastError() { return MilviErrorBuffer(); }
extern "C" void milviClearError() { MilviErrorBuffer()[0] = '\0'; }
