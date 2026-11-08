#include "native_kmeans.h"
#include <mlpack/methods/kmeans/dual_tree_kmeans.hpp>
#include <mlpack/methods/kmeans/naive_kmeans.hpp>
#include <mlpack/methods/neighbor_search/neighbor_search.hpp>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <omp.h>

namespace {

// Les changements OpenMP et de journalisation sont restaurés dans tous les cas.
struct CallSettings
{
  int previousThreads = omp_get_max_threads();
  bool previousInfo = mlpack::Log::Info.ignoreInput;
  explicit CallSettings(int threads)
  {
    omp_set_num_threads(threads);
    mlpack::Log::Info.ignoreInput = true;
  }
  ~CallSettings()
  {
    mlpack::Log::Info.ignoreInput = previousInfo;
    omp_set_num_threads(previousThreads);
  }
};

template<template<class, class> class LloydStep>
void RunIterations(const arma::mat& data, arma::mat& centers,
    size_t limit, double tolerance, size_t& iterations, size_t& calculations,
    bool& converged)
{
  mlpack::EuclideanDistance distance;
  LloydStep<mlpack::EuclideanDistance, arma::mat> step(data, distance);
  arma::mat next;
  arma::Col<size_t> counts;
  for (iterations = 0; iterations < limit;)
  {
    step.Iterate(centers, next, counts);
    for (size_t c = 0; c < counts.n_elem; ++c)
      if (counts[c] == 0) next.col(c) = centers.col(c);
    const double residual = arma::norm(next - centers, "fro");
    if (!next.is_finite() || !std::isfinite(residual))
      throw std::overflow_error("débordement numérique : réduire l’échelle des coordonnées");
    centers.swap(next);
    ++iterations;
    if (residual <= tolerance) { converged = true; break; }
  }
  calculations += step.DistanceCalculations();
}

void AssignWithKDTree(const arma::mat& data, const arma::mat& centers,
    arma::Row<size_t>& labels, double& inertia, size_t& calculations)
{
  using Search = mlpack::NeighborSearch<>;
  std::vector<size_t> references, queries;
  Search::Tree referenceTree(centers, references);
  Search::Tree queryTree(data, queries);
  Search search(std::move(referenceTree));
  arma::Mat<size_t> neighbors;
  arma::mat distances;
  search.Search(queryTree, 1, neighbors, distances);
  labels.set_size(data.n_cols);
  inertia = 0;
  for (size_t i = 0; i < data.n_cols; ++i)
  {
    labels[queries[i]] = references[neighbors(0, i)];
    inertia += distances(0, i) * distances(0, i);
  }
  calculations += search.BaseCases() + search.Scores();
}

} // namespace

extern "C" int milviKmeans(const double* input, size_t n, size_t dimensions,
    size_t clusters, const double* initial, size_t maxIterations, double tolerance,
    int algorithm, int threads, int keepEmpty, double* outputCenters,
    intptr_t* outputLabels, size_t* actualClusters, size_t* iterations,
    size_t* calculations, double* inertia, int* converged,
    char* error, size_t errorCapacity)
{
  try
  {
    if (!input || !initial || !outputCenters || !outputLabels || !actualClusters ||
        !iterations || !calculations || !inertia || !converged ||
        n == 0 || dimensions == 0 || clusters == 0 || clusters > n ||
        n > std::numeric_limits<size_t>::max() / dimensions ||
        maxIterations == 0 || threads < 1 || !(tolerance > 0) ||
        !std::isfinite(tolerance) || (algorithm != 0 && algorithm != 1))
      throw std::invalid_argument("paramètres k-means invalides");
    CallSettings settings(threads);
    // Copie privée : les réordonnancements des arbres ne touchent jamais Go.
    arma::mat data(input, dimensions, n);
    arma::mat centers(initial, dimensions, clusters);
    if (!data.is_finite() || !centers.is_finite())
      throw std::invalid_argument("coordonnées non finies");
    size_t count = 0, distances = 0;
    bool didConverge = false;
    if (clusters == 1)
    {
      centers = arma::mean(data, 1);
      count = 1;
      didConverge = true;
    }
    else if (algorithm == 0)
      RunIterations<mlpack::DualTreeKMeans>(data, centers, maxIterations,
          tolerance, count, distances, didConverge);
    else
      RunIterations<mlpack::NaiveKMeans>(data, centers, maxIterations,
          tolerance, count, distances, didConverge);

    if (!centers.is_finite())
      throw std::overflow_error("moyenne non finie : réduire l’échelle des coordonnées");
    arma::Row<size_t> assignments;
    double objective = 0;
    // L’API historique refait ici un balayage Θ(nk), même après dual-tree.
    // La recherche exacte par KD-tree évite de perdre l’accélération à cette étape.
    AssignWithKDTree(data, centers, assignments, objective, distances);
    if (!std::isfinite(objective))
      throw std::overflow_error("inertie non finie : réduire l’échelle des coordonnées");
    arma::Col<size_t> counts(clusters, arma::fill::zeros);
    for (size_t label : assignments) ++counts[label];
    std::vector<size_t> mapping(clusters);
    size_t retained = 0;
    for (size_t c = 0; c < clusters; ++c)
    {
      if (keepEmpty || counts[c] != 0)
      {
        mapping[c] = retained;
        std::copy(centers.colptr(c), centers.colptr(c) + dimensions,
            outputCenters + retained * dimensions);
        ++retained;
      }
    }
    for (size_t i = 0; i < n; ++i)
      outputLabels[i] = static_cast<intptr_t>(mapping[assignments[i]]);
    *actualClusters = retained;
    *iterations = count;
    *calculations = distances;
    *inertia = objective;
    *converged = didConverge;
    return 0;
  }
  catch (const std::exception& failure)
  {
    if (error && errorCapacity) std::snprintf(error, errorCapacity, "%s", failure.what());
  }
  catch (...)
  {
    if (error && errorCapacity) std::snprintf(error, errorCapacity, "exception C++ inconnue");
  }
  return 1;
}
