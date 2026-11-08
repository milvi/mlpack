# Propriété mémoire du binding Go du fork milvi

Le module `github.com/milvi/mlpack-go` dépend du module natif
`github.com/milvi/mlpack`. Tous deux demandent Go ≥ 1.27.3. Le [guide natif](../../../README-go-milvi.md)
et le [rapport de validation](https://github.com/milvi/mlpack-go/blob/master/docs/corrections-et-validation.md)
décrivent la version publiée et ses preuves.

## Entrées et sorties

Les matrices Gonum sont stockées par lignes, Armadillo par colonnes : les
conversions inversent leurs dimensions. Une vue Gonum non contiguë est d’abord
compactée. Les données réelles sont **copiées dans une mémoire C++ possédée** ;
les conversions vers les entiers possèdent également leur résultat. Un modèle
qui déplace une matrice d’entraînement ne conserve donc aucun pointeur vers Go.
Le pinning automatique pendant l’appel cgo suffit ; aucun `runtime.Pinner`
persistant n’est conservé dans les paramètres.

Les sorties sont copiées vers les tampons Go. Le stockage natif reste possédé
par ses objets C++ et disparaît avec eux. Les anciens transferts `GetMemory`,
les getters de tampons non possédés et le setter générique de pointeur brut
inutilisé sont supprimés. Les dimensions sont échangées en `size_t`, avec
vérification des dépassements avant les allocations Go.

`mlpackCopyParamMat` rend `false` pour une longueur incorrecte ou un type invalide.
L’identifiant de conversion est :

| kind | Type natif |
|---:|---|
| 0 | `arma::mat` |
| 1 | `arma::Mat<size_t>` |
| 2 | `arma::rowvec` |
| 3 | `arma::Row<size_t>` |
| 4 | `arma::colvec` |
| 5 | `arma::Col<size_t>` |
| 6 | Matrice du tuple `DatasetInfo, arma::mat` |

Les valeurs entières restent des `float64` dans l’interface Gonum historique.
La nouvelle interface `Cluster` rend directement des étiquettes `[]int` et des
centres `[]float64`, sans objet natif persistant et sans `Close`.

## Modèles et erreurs

Les 26 bindings créateurs de modèles utilisent un propriétaire local jusqu’au
transfert dans les paramètres de sortie. Les entrées fournies comme modèles
restent empruntées. Les paramètres détruisent les modèles non exportés ; le
frontend partage ensuite un propriétaire entre les alias d’un modèle exporté.
`Close` est idempotent et ferme tous les alias. Un registre faible et un nettoyage
automatique servent de dernier recours lorsque l’utilisateur omet `Close`.

Les fonctions C interceptent les exceptions dans un tampon borné. Les fonctions
Go historiques gardent leurs signatures et produisent `panic(error)` ; `Cluster`
rend `error`. Un verrou commun couvre les appels et leur nettoyage. Chaque chaîne
C temporaire est libérée dans sa portée.

## Compilation et maintenance

`tools/generate-native.py` génère les wrappers C++ conservés dans Git et déduplique
les types de modèles. Les aides Go et les générateurs CMake utilisent le chemin
`github.com/milvi/mlpack-go`. La procédure utilisateur est celle du
[quickstart Go](../../quickstart/go.md) : APT puis `go get` et `go build`.

Les en-têtes imbriqués sont suivis par le cache de Go avec `go:embed`, sans
conservation de son système de fichiers inutilisé dans l’exécutable. Les en-têtes
et bibliothèques système APT restent extérieurs à cette clé de cache.
Les tests vérifient les restitutions, les exceptions, la vie des modèles après
ramasse-miettes et modification des entrées, et la parité k-means naïf/dual-tree.
Le pilote `tests/native_memory.cpp` ajoute l’injection de pannes, ASan, UBSan et
LSan. Ces contrôles ne constituent pas une preuve exhaustive de tous les chemins
internes de tous les algorithmes.
