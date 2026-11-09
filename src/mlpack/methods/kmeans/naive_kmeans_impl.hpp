/**
 * @file methods/kmeans/naive_kmeans_impl.hpp
 * @author Ryan Curtin
 * @author Shikhar Bhardwaj
 *
 * An implementation of a naively-implemented step of the Lloyd algorithm for
 * k-means clustering, using OpenMP for parallelization over multiple threads.
 * This may still be the best choice for small datasets or datasets with very
 * high dimensionality.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_METHODS_KMEANS_NAIVE_KMEANS_IMPL_HPP
#define MLPACK_METHODS_KMEANS_NAIVE_KMEANS_IMPL_HPP

// In case it hasn't been included yet.
#include "naive_kmeans.hpp"

#include <mlpack/core/util/log.hpp>
#include <atomic>

namespace mlpack {

template<typename DistanceType, typename MatType>
NaiveKMeans<DistanceType, MatType>::NaiveKMeans(const MatType& dataset,
                                                DistanceType& distance) :
    dataset(dataset),
    distance(distance),
    distanceCalculations(0)
{ /* Nothing to do. */ }

// Run a single iteration.
template<typename DistanceType, typename MatType>
double NaiveKMeans<DistanceType, MatType>::Iterate(const arma::mat& centroids,
                                                   arma::mat& newCentroids,
                                                   arma::Col<size_t>& counts)
{
  newCentroids.zeros(centroids.n_rows, centroids.n_cols);
  counts.zeros(centroids.n_cols);

  // Allouer avant la région parallèle : une allocation en échec peut ainsi
  // remonter normalement à l’appelant, au lieu de traverser une frontière OpenMP.
  size_t threads = 1;
  #if defined(MLPACK_USE_OPENMP)
    threads = omp_get_max_threads();
  #endif
  std::vector<arma::mat> sums;
  std::vector<arma::Col<size_t>> sizes;
  sums.reserve(threads);
  sizes.reserve(threads);
  for (size_t thread = 0; thread < threads; ++thread)
  {
    sums.emplace_back(centroids.n_rows, centroids.n_cols, arma::fill::zeros);
    sizes.emplace_back(centroids.n_cols, arma::fill::zeros);
  }
  std::exception_ptr failure;
  const auto* deadline = detail::currentDeadline;
  std::atomic<bool> failed{false};
  #pragma omp parallel
  {
  detail::DeadlineScope scope(deadline);
  #pragma omp for schedule(static)
  for (size_t i = 0; i < dataset.n_cols; ++i)
  {
    try
    {
      if (failed.load(std::memory_order_relaxed)) continue;
      detail::PollDeadline();
      size_t thread = 0;
      #if defined(MLPACK_USE_OPENMP)
        thread = omp_get_thread_num();
      #endif
      double best = std::numeric_limits<double>::infinity();
      size_t closest = centroids.n_cols;
      for (size_t j = 0; j < centroids.n_cols; ++j)
      {
        const double candidate = distance.Evaluate(dataset.col(i), centroids.col(j));
        if (candidate < best) { best = candidate; closest = j; }
      }
      if (closest == centroids.n_cols)
        throw std::overflow_error("aucune distance finie vers les centres");
      sums[thread].col(closest) += dataset.col(i);
      ++sizes[thread][closest];
    }
    catch (...)
    {
      failed.store(true, std::memory_order_relaxed);
      #pragma omp critical
      { if (!failure) failure = std::current_exception(); }
    }
  }
  } // Les workers ont terminé avant toute destruction des temporaires.
  if (failure) std::rethrow_exception(failure);
  // Un ordre de réduction fixe évite une dépendance à l’ordonnancement des threads.
  for (size_t thread = 0; thread < threads; ++thread)
  {
    newCentroids += sums[thread];
    counts += sizes[thread];
  }
  for (size_t i = 0; i < centroids.n_cols; ++i)
    if (counts[i] != 0) newCentroids.col(i) /= counts[i];

  distanceCalculations += centroids.n_cols * dataset.n_cols;

  // Calculate cluster distortion for this iteration.
  double cNorm = 0.0;
  for (size_t i = 0; i < centroids.n_cols; ++i)
  {
    cNorm += std::pow(distance.Evaluate(centroids.col(i), newCentroids.col(i)),
        2.0);
  }
  distanceCalculations += centroids.n_cols;

  return std::sqrt(cNorm);
}

} // namespace mlpack

#endif
