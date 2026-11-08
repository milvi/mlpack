// Package native compile le C++ mlpack et son interface C avec cgo.
// Les consommateurs utilisent normalement github.com/milvi/mlpack-go.
package native

/*
#cgo CXXFLAGS: -std=c++17 -O3 -DNDEBUG -fopenmp -I${SRCDIR}/src -DMLPACK_GO_MONOLITHIC
#cgo !mlpack_debug CXXFLAGS: -g0
#cgo LDFLAGS: -larmadillo -lstdc++ -fopenmp
#include <stddef.h>
const char* milviLastError(void);
void milviClearError(void);
*/
import "C"

import (
	"embed"
	"errors"
	"sync"
)

var calls sync.Mutex

// Acquire sérialise les appels qui utilisent les réglages globaux de mlpack.
// La fonction rendue doit être différée jusqu’après le nettoyage natif.
func Acquire() func() {
	calls.Lock()
	C.milviClearError()
	return calls.Unlock
}

// Error récupère une exception interceptée à la frontière C, puis l’efface.
func Error() error {
	message := C.GoString(C.milviLastError())
	C.milviClearError()
	if message == "" {
		return nil
	}
	return errors.New(message)
}

// Le cache de go build suit aussi les en-têtes C++ imbriqués.
// La variable inutilisée est éliminée de l’exécutable par l’éditeur de liens.
//
//go:embed src/mlpack.hpp src/mlpack
var _ embed.FS
