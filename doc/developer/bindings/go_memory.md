# Propriété mémoire du binding Go corrigé

Le binding Go doit conserver un propriétaire explicite de chaque allocation.
Les utilitaires corrigés utilisent Go ≥ 1.21 pour `runtime.Pinner`. La distribution
[milvi/mlpack-go](https://github.com/milvi/mlpack-go) utilise Go 1.27.3 et conserve
ses tests de régression et sa recette Ubuntu 26.04.

## Entrées

Les matrices Go contiguës sont présentées à Armadillo en inversant les dimensions
pour passer du stockage par lignes à celui par colonnes. Lorsqu'une vue Gonum a
un pas supérieur à sa largeur, `compactData` la copie dans un stockage contigu.
Les données que C++ peut conserver entre appels cgo sont épinglées par le
`runtime.Pinner` de l'objet `params`. Elles sont désépinglées seulement après
`mlpackCleanParams`. Les matrices vides ou absentes sont refusées explicitement.

## Sorties

L'ancien mécanisme `GetMemory` détachait un tampon natif sans propriétaire capable
de le libérer du côté Go. Il reste disponible pour les anciens clients de l'ABI,
mais le nouveau wrapper Go ne l'utilise plus.

`mlpackCopyParamMat` copie vers un tampon `double` alloué par Go pendant un seul
appel cgo. Le stockage natif ne change pas de propriétaire et disparaît avec
`util::Params`. L'identifiant de conversion est :

| `kind` | Type natif |
|---:|---|
| 0 | `arma::mat` |
| 1 | `arma::Mat<size_t>` |
| 2 | `arma::rowvec` |
| 3 | `arma::Row<size_t>` |
| 4 | `arma::colvec` |
| 5 | `arma::Col<size_t>` |
| 6 | Matrice du tuple `DatasetInfo, arma::mat` |

La longueur doit correspondre exactement au nombre d'éléments. Le tampon n'est
jamais conservé après le retour de la fonction. Un pointeur nul avec une longueur
non nulle, un type invalide ou une exception donne `false`. Une copie vide peut
recevoir un pointeur nul. Les valeurs de `size_t` sont converties en `double`,
comme dans l'interface Go existante.

`mlpackCopyParamVectorInt` applique le même contrat au vecteur d'entiers, avec un
tampon `long long`. Ces nouvelles fonctions ne retirent aucun symbole historique.
Leur présence est requise par les nouveaux wrappers Go : il faut distribuer les
deux parties ensemble.

## Génération et nettoyage

`print_go.cpp` émet les `defer` de nettoyage des paramètres et des chronomètres
immédiatement après leur création. `AppendModel.cmake` émet une libération pour
chaque chaîne C temporaire de ses accesseurs. Les utilitaires Go communs libèrent
aussi toutes leurs chaînes temporaires. Le chemin du module reste
`mlpack.org/v1/mlpack` dans la source du générateur ; la distribution du fork
utilise son propre chemin `github.com/milvi/mlpack-go`.

Pour générer les wrappers, configurer CMake avec `BUILD_GO_BINDINGS=ON`,
`FORCE_BUILD_GO_BINDINGS=ON`, `BUILD_GO_SHLIB=ON`, puis construire la cible `go`.
`generate_go_kmeans` permet de vérifier seulement le générateur de k-means.
Pour utiliser des wrappers déjà générés, `BUILD_GO_BINDINGS=OFF` et
`BUILD_GO_SHLIB=ON` suffisent à construire les bibliothèques natives.

## Validation et portée

Le patch a été appliqué à l'archive officielle mlpack 4.8.0 et les utilitaires ont
été recompilés avec GCC 15.2.0 et les dépendances Ubuntu 26.04. Le générateur
`generate_go_kmeans` a produit exactement le wrapper corrigé distribué ; les
52 chaînes C des accesseurs de modèles générés ont leur libération différée.

Les tests du dépôt de distribution vérifient l'indépendance des sorties, les vues
Gonum non contiguës, les conversions usuelles, les vecteurs, k-means et les règles
de pointeurs avec `GOEXPERIMENT=cgocheck2`. Un diagnostic glibc sur 50 appels de
100 000 points a donné +2 464 octets natifs après correction, contre +40 081 888
avant correction. Les détails et les limites du laboratoire sont dans le rapport
de validation du dépôt de distribution.

La construction intégrale neuve des 47 bibliothèques, sous utilisateur ordinaire
dans un conteneur Ubuntu 26.04, a aussi
[réussi dans GitHub Actions](https://github.com/milvi/mlpack-go/actions/runs/41368925523)
sur le commit de distribution `a4cedd823d9c44d970a1330fe6ef8a2d4e27149c`.
Les tests ordinaires, les contrôles `cgocheck2`, les tests de mémoire native et
l'exemple k-means y passent. La variation native du même diagnostic y est de
+3 920 octets. Ce résultat est distinct de la mesure du laboratoire ci-dessus.

Ce changement ne définit pas encore de contrat général de destruction des modèles
opaques, de conversion des exceptions d'algorithmes en erreurs Go ou de sûreté
des appels concurrents. Les anciens accesseurs natifs à transfert de pointeurs
restent historiques ; les anciens clients qui les appellent ne sont pas corrigés
par une simple mise à jour de la bibliothèque partagée.
