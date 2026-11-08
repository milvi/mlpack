package native_test

import (
	native "github.com/milvi/mlpack"
	"testing"
)

func TestNativeKMeans(t *testing.T) {
	points := []float64{0, 0, 0, 1, 10, 10, 10, 11}
	result, err := native.KMeans(points, 2, 2, []float64{0, 0, 10, 10}, 100, 1e-5, 0, 1, false)
	if err != nil {
		t.Fatal(err)
	}
	if result.Clusters != 2 || result.Labels[0] != result.Labels[1] || result.Labels[2] != result.Labels[3] || result.Labels[0] == result.Labels[2] {
		t.Fatal("partition incorrecte")
	}
}
