package native

import (
	"errors"
	"testing"
	"time"
)

func TestDurationIncludesWaitingForNativeLock(t *testing.T) {
	calls.Lock()
	defer calls.Unlock()
	started := time.Now()
	_, err := KMeans([]float64{0, 0}, 2, 1, []float64{0, 0}, 10*time.Millisecond, 1e-5, 0, 1, false)
	if !errors.Is(err, ErrTimeLimit) {
		t.Fatal(err)
	}
	if time.Since(started) > time.Second {
		t.Fatal("attente du verrou non bornée")
	}
}
