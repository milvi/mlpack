# Démarrage Go du fork milvi

Le module utilisateur est `github.com/milvi/mlpack-go`. Son code C++ est téléchargé
comme une dépendance Go, puis compilé automatiquement par cgo. Les bibliothèques
mathématiques tierces restent installées par APT.

## Installer sur Ubuntu 26.04 amd64

Go 1.27.3 ou une version plus récente doit être installé. Depuis un projet qui
possède déjà son `go.mod` :

```sh
sudo apt-get update
sudo apt-get install --no-install-recommends build-essential libarmadillo-dev libensmallen-dev libcereal-dev ca-certificates
go get github.com/milvi/mlpack-go@v0.1.0
go build -p 2 ./...
```

La première compilation des méthodes C++ prend plusieurs minutes. `-p 2` borne
le nombre de compilations simultanées. Les compilations suivantes réutilisent le
cache Go standard. Aucun `make install`, patch ou `env.sh` n’est nécessaire.

## Un appel k-means

```go
package main

import (
    "fmt"
    mlpack "github.com/milvi/mlpack-go"
)

func main() {
    points := []float64{0, 0, 0, 1, 10, 10, 10, 11}
    resultat, err := mlpack.Cluster(points, 2, 2, nil)
    if err != nil {
        panic(err)
    }
    fmt.Println(resultat.Centers, resultat.Labels, resultat.Converged)
}
```

Une ligne représente un point ; les coordonnées sont contiguës. Par défaut,
`Cluster` utilise dual-tree avec KD-tree, une graine reproductible, un thread et
au plus 1 000 itérations. Les entrées restent inchangées et les résultats sont
des slices Go indépendantes. Aucun objet C++ ne doit être fermé après cet appel.
Lire `Converged` pour distinguer convergence et arrêt à la limite d’itérations.

Gonum v0.17.0 est la version minimale demandée. Une version plus récente requise
par le projet est utilisée par tous les imports qui partagent ce chemin de
module. `go get gonum.org/v1/gonum@latest` met explicitement à jour ce choix.

Le [guide complet](https://github.com/milvi/mlpack-go) présente les emplacements
écrits, le cache, les versions, le schéma de compilation et la maintenance.
Le [rapport](https://github.com/milvi/mlpack-go/blob/master/docs/corrections-et-validation.md)
contient les mesures sur 100 000 et 1 000 000 points et les contrôles de mémoire.
Les [fonctions historiques](../user/bindings/go.md) restent disponibles avec leurs
signatures d’origine ; leurs modèles opaques possèdent `Close() error`.
