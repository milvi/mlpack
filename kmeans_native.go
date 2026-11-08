package native

/*
#include "native_kmeans.h"
*/
import "C"

import (
	"fmt"
	"math"
	"unsafe"
)

// Result contient seulement de la mémoire Go, valide après la fin de l’appel C++.
type Result struct {
	Centers                          []float64
	Labels                           []int
	Clusters, Dimensions, Iterations int
	DistanceCalculations             uint64
	Inertia                          float64
	Converged                        bool
}

// KMeans est la frontière native sans état persistant. L’initialisation est fournie par Go.
func KMeans(data []float64, dimensions, clusters int, initial []float64,
	maxIterations int, tolerance float64, algorithm, threads int, keepEmpty bool) (Result, error) {
	if dimensions <= 0 || len(data) == 0 || len(data)%dimensions != 0 ||
		clusters <= 0 || clusters > len(data)/dimensions || clusters > int(^uint(0)>>1)/dimensions ||
		len(initial) != clusters*dimensions || maxIterations < 1 || threads < 1 || threads > math.MaxInt32 ||
		!(tolerance > 0) || math.IsInf(tolerance, 0) || (algorithm != 0 && algorithm != 1) {
		return Result{}, fmt.Errorf("mlpack : paramètres k-means invalides")
	}
	for _, values := range [][]float64{data, initial} {
		for _, v := range values {
			if math.IsNaN(v) || math.IsInf(v, 0) {
				return Result{}, fmt.Errorf("mlpack : coordonnées non finies")
			}
		}
	}
	result := Result{Centers: make([]float64, clusters*dimensions), Labels: make([]int, len(data)/dimensions), Dimensions: dimensions}
	var actual, iterations, calculations C.size_t
	var inertia C.double
	var converged C.int
	var message [4096]C.char
	keep := C.int(0)
	if keepEmpty {
		keep = 1
	}
	calls.Lock()
	defer calls.Unlock()
	status := C.milviKmeans((*C.double)(unsafe.Pointer(unsafe.SliceData(data))), C.size_t(len(result.Labels)),
		C.size_t(dimensions), C.size_t(clusters), (*C.double)(unsafe.Pointer(unsafe.SliceData(initial))),
		C.size_t(maxIterations), C.double(tolerance), C.int(algorithm), C.int(threads), keep,
		(*C.double)(unsafe.Pointer(unsafe.SliceData(result.Centers))), (*C.intptr_t)(unsafe.Pointer(unsafe.SliceData(result.Labels))),
		&actual, &iterations, &calculations, &inertia, &converged, &message[0], C.size_t(len(message)))
	if status != 0 {
		return Result{}, fmt.Errorf("mlpack : %s", C.GoString(&message[0]))
	}
	result.Clusters, result.Iterations = int(actual), int(iterations)
	result.Centers = result.Centers[:result.Clusters*dimensions]
	result.DistanceCalculations, result.Inertia, result.Converged = uint64(calculations), float64(inertia), converged != 0
	return result, nil
}
