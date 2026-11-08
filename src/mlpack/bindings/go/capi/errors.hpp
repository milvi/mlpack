#ifndef MILVI_MLPACK_NATIVE_ERRORS_HPP
#define MILVI_MLPACK_NATIVE_ERRORS_HPP

#include <cstdio>
#include <exception>

// Accès sérialisés par le verrou commun du paquet Go native.
inline char* MilviErrorBuffer()
{
  static char message[4096] = {};
  return message;
}
inline void MilviSetError(const char* message) noexcept
{
  std::snprintf(MilviErrorBuffer(), 4096, "%s", message);
}
#endif
