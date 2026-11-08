# K-means C++ compilé automatiquement par Go

Branche `codex/kmeans-only`, préversion `v0.2.0-kmeans.1` du module
`github.com/milvi/mlpack`. Le code est extrait de mlpack C++ 4.8.0.
Le module utilisateur est [`github.com/milvi/mlpack-go`](https://github.com/milvi/mlpack-go/tree/codex/kmeans-only).

## Contenu conservé

- Une seule unité C++ : `native_kmeans_api.cpp`, une seule fonction C : `milviKmeans`.
- K-means dual-tree avec KD-tree, bornes rectangulaires et recherche exacte de
  voisins nécessaire aux itérations et à l’affectation finale.
- K-means naïf comme référence numérique, accessible par le même appel.
- 55 en-têtes : dépendances exactes du calcul et du pilote mémoire, vérifiées
  par le préprocesseur en modes normal et debug.

Les autres méthodes, bindings, générateurs, modèles persistants, chargeurs de
fichiers, autres arbres et infrastructures CMake sont retirés. La sérialisation
est retirée des classes conservées : cereal et ensmallen ne sont plus requis.
Les copyrights et la licence des sources conservées restent présents.

## Construction sur Ubuntu 26.04

Go 1.27.3 ou ultérieur et cgo activé ; seuls ces paquets de développement sont nécessaires :

```sh
sudo apt-get update
sudo apt-get install --no-install-recommends build-essential libarmadillo-dev ca-certificates
```

Le consommateur importe le module utilisateur ; `go build` compile ce module
natif et réutilise ensuite le cache Go standard. Les en-têtes sous `src/mlpack`
participent au cache grâce à `go:embed`. Le système de fichiers inutilisé est
éliminé du binaire. Après modification des bibliothèques ou en-têtes APT,
utiliser `go build -a ./...` pour forcer leur prise en compte.

## Propriété mémoire et erreurs

`KMeans` reçoit des slices Go et fait un seul appel C. Les données d’entrée sont
copiées côté C++ avant tout réordonnancement. Les centres et étiquettes sont
écrits dans des slices appartenant à Go ; aucun pointeur Go ni objet C++ ne survit
au calcul. Aucun `Close` n’est nécessaire.

Les propriétaires uniques des arbres, le nettoyage des constructeurs interrompus,
les affectations sûres des arbres et des bornes, ainsi que la restauration des
enfants masqués sur exception sont conservés. Le k-means naïf intercepte ses
exceptions dans la région OpenMP et les relance sur le thread appelant. La
frontière C intercepte les exceptions et remplit un message borné ; Go rend
`error`. Un verrou protège les réglages partagés et ceux-ci sont restaurés au retour.

Les centres vides restent fixes pendant les itérations, puis sont retirés à la
fin sauf option contraire. Les étiquettes sont renumérotées et correspondent à
l’ordre initial des points. `Converged == false` indique que le budget d’itérations
est atteint ; il faut examiner ce champ même si `error` est nul.

## Vérifications

```sh
python3 tools/check-source.py
go test -timeout 90s ./...
```

`tools/check-source.py` contrôle les inclusions, l’absence de dépendance retirée,
une seule unité de production et l’absence d’en-tête superflu. Le pilote
`tests/native_memory.cpp` interrompt chaque allocation à tour de rôle et vérifie
le retour exact au nombre initial de `new` actifs après chaque scénario k-means
ou arbre. Il couvre aussi les copies et déplacements d’arbres et de bornes.

Le [Dockerfile](Dockerfile.golang) et le [workflow](.github/workflows/go-ubuntu2604.yml)
exécutent ces contrôles sur Ubuntu 26.04, avec détection des accès mémoire
invalides, comportements indéfinis et fuites. Chaque compilation et exécution de
test est limitée à deux minutes. Les tests Go et les mesures de restitution du
frontend complètent ce contrôle ; ils ne constituent pas une preuve pour toute
entrée imaginable.

## Maintenance

Modifier directement les sources conservées ; aucun patch ni génération chez le
consommateur. Après validation, publier un nouveau tag sémantique immuable du
module natif, puis mettre sa version à jour dans le module utilisateur. Les
versions stables existantes et `master` restent indépendants de cette branche.
