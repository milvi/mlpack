// Budget monotone par appel ; aucune exception pendant le nettoyage.
#ifndef MLPACK_CORE_UTIL_DEADLINE_HPP
#define MLPACK_CORE_UTIL_DEADLINE_HPP
#include <chrono>
#include <cstdint>
#include <exception>

namespace mlpack { namespace detail {
class TimeLimit final : public std::exception
{
 public:
  const char* what() const noexcept override { return "durée maximale dépassée"; }
};
#ifdef MLPACK_DEADLINE_TESTING
// Injection déterministe réservée au pilote de test natif.
inline thread_local long checksBeforeTimeout = -1;
#endif
class Deadline
{
  const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  const std::chrono::nanoseconds budget;
 public:
  explicit Deadline(uint64_t nanoseconds) : budget(nanoseconds) { }
  void Check() const
  {
#ifdef MLPACK_DEADLINE_TESTING
    if (checksBeforeTimeout == 0) throw TimeLimit{};
    if (checksBeforeTimeout > 0) --checksBeforeTimeout;
#endif
    if (std::chrono::steady_clock::now() - start >= budget) throw TimeLimit{};
  }
};
inline thread_local const Deadline* currentDeadline = nullptr;
inline thread_local unsigned deadlinePolls = 0;
class DeadlineScope
{
  const Deadline* previous;
  unsigned previousPolls;
 public:
  explicit DeadlineScope(const Deadline* deadline) noexcept :
      previous(currentDeadline), previousPolls(deadlinePolls)
  { currentDeadline = deadline; deadlinePolls = 0; }
  ~DeadlineScope() noexcept
  { currentDeadline = previous; deadlinePolls = previousPolls; }
  DeadlineScope(const DeadlineScope&) = delete;
  DeadlineScope& operator=(const DeadlineScope&) = delete;
};
inline void CheckDeadline()
{ if (currentDeadline) currentDeadline->Check(); }
inline void PollDeadline()
{
  // Une lecture d’horloge pour 256 points de contrôle, sans allocation.
  if (currentDeadline && (++deadlinePolls & 255u) == 0) currentDeadline->Check();
}
}} // namespace mlpack::detail
#endif
