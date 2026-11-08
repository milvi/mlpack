// Package native compile uniquement k-means et ses dépendances C++ avec cgo.
// Les consommateurs utilisent github.com/milvi/mlpack-go.
package native

/*
#cgo CXXFLAGS: -std=c++17 -O3 -DNDEBUG -fopenmp -I${SRCDIR}/src -DMLPACK_GO_MONOLITHIC
#cgo !mlpack_debug CXXFLAGS: -g0
#cgo LDFLAGS: -larmadillo -lstdc++ -fopenmp
*/
import "C"

import (
	"embed"
	"sync"
)

// Le verrou protège les réglages OpenMP et les journaux pendant un appel.
var calls sync.Mutex

// Les sources imbriquées font partie de la clé du cache de Go.
// Le système de fichiers inutilisé est éliminé au lien.
//
//go:embed src/mlpack
var _ embed.FS
