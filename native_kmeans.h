#ifndef MILVI_MLPACK_KMEANS_H
#define MILVI_MLPACK_KMEANS_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

// Retour : 0 succès, 1 erreur, 2 budget expiré. iterations reste diagnostique.
// Tous les tampons appartiennent à l’appelant ; aucun pointeur n’est conservé.
int milviKmeans(const double* data, size_t n, size_t dimensions, size_t clusters,
    const double* initial, uint64_t max_nanoseconds, double tolerance, int algorithm,
    int threads, int keep_empty, double* centers, intptr_t* labels,
    size_t* actual_clusters, size_t* iterations, size_t* calculations,
    double* inertia, int* converged, char* error, size_t error_capacity);
#ifdef __cplusplus
}
#endif
#endif
