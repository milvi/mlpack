// Vérification native indépendante de Go : exceptions, allocations interrompues et restitutions.
#include "native_kmeans.h"
#include <mlpack/core/tree/binary_space_tree.hpp>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <new>
#include <vector>

static thread_local long failAfter = -1;
static std::atomic<long> liveAllocations{0};
static void* Allocate(size_t size)
{
  if (failAfter == 0) { failAfter = -1; throw std::bad_alloc(); }
  if (failAfter > 0) --failAfter;
  void* result = std::malloc(size == 0 ? 1 : size);
  if (!result) throw std::bad_alloc();
  ++liveAllocations;
  return result;
}
void* operator new(size_t size) { return Allocate(size); }
void* operator new[](size_t size) { return Allocate(size); }
void operator delete(void* value) noexcept { if (value) --liveAllocations; std::free(value); }
void operator delete[](void* value) noexcept { if (value) --liveAllocations; std::free(value); }
void operator delete(void* value, size_t) noexcept { if (value) --liveAllocations; std::free(value); }
void operator delete[](void* value, size_t) noexcept { if (value) --liveAllocations; std::free(value); }

int main()
{
  const size_t n = 40, d = 2, k = 3;
  std::vector<double> data(n*d), centers(k*d), initial{0,0,0.5,0.5,1,1};
  std::vector<intptr_t> labels(n);
  for (size_t i = 0; i < n; ++i) { data[2*i] = double(i)/n; data[2*i+1] = double(i%7)/7; }
  size_t actual, iterations, calculations;
  double inertia;
  int converged;
  char message[4096];
  auto call = [&](int algorithm) {
    return milviKmeans(data.data(),n,d,k,initial.data(),20,1e-5,algorithm,1,0,
        centers.data(),labels.data(),&actual,&iterations,&calculations,&inertia,
        &converged,message,sizeof(message));
  };
  if (call(0) != 0 || call(1) != 0) return 1;
  const long baseline = liveAllocations;
  size_t rejected = 0;
  // Chaque point d’injection n’échoue qu’une fois ; le déroulement des destructeurs reste possible.
  for (int algorithm : {0,1})
    for (long position = 0; position < 1600; ++position)
    {
      failAfter = position;
      const int status = call(algorithm);
      failAfter = -1;
      if (status != 0) ++rejected;
      if (liveAllocations != baseline)
      {
        std::cerr << "Allocation non restituée : algorithme " << algorithm << ", position " << position << ", différence " << liveAllocations-baseline << std::endl;
        return 3;
      }
    }
  // Les affectations d’arbres doivent également rester destructibles après échec.
  size_t treeFailures = 0, treeSuccesses = 0;
  for (long position = 0; position < 2000; ++position)
  {
    failAfter = position;
    try
    {
      arma::mat points(data.data(), d, n);
      mlpack::KDTree<> source(points, 1), destination(points, 1);
      destination = source;
      mlpack::KDTree<> moved(points, 1);
      moved = std::move(destination);
      mlpack::HRectBound<> firstBound(2), secondBound(3), emptyBound;
      firstBound = secondBound;
      emptyBound |= points;
      secondBound = std::move(firstBound);
      ++treeSuccesses;
      if ((moved.Left() && moved.Left()->Parent() != &moved) ||
          (moved.Right() && moved.Right()->Parent() != &moved)) return 5;
    }
    catch (const std::bad_alloc&) { ++treeFailures; }
    failAfter = -1;
    if (liveAllocations != baseline)
    {
      std::cerr << "Arbre non restitué à la position " << position << ", différence " << liveAllocations-baseline << std::endl;
      return 4;
    }
  }
  if (treeSuccesses == 0) return 6;
  std::cout << "Pannes d’affectation/construction d’arbre interceptées : " << treeFailures << std::endl;
  std::cout << "Allocations interrompues interceptées : " << rejected << std::endl;
  if (rejected < 20 || call(0) != 0) return 2;
  return 0;
}
