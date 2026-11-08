# Module C++ compilé par Go

Ce fork conserve mlpack amont 4.8.0 et ajoute le module
`github.com/milvi/mlpack` (`package native`). Le consommateur utilise normalement
[`github.com/milvi/mlpack-go`](https://github.com/milvi/mlpack-go).

Les fichiers `native_*.cpp` à la racine sont compilés automatiquement par cgo.
Ils réunissent les 46 bindings historiques, deux utilitaires communs et une
interface k-means autonome. `tools/generate-native.py` régénère les wrappers
historiques et déduplique les 26 types de modèles. Aucun générateur, CMake,
script d’installation ou patch n’est exécuté chez le consommateur.

Le module Go requiert Go ≥ 1.27.3, C++17, OpenMP et les paquets Ubuntu
`build-essential libarmadillo-dev libensmallen-dev libcereal-dev`.
`go test .` vérifie l’interface native. Le frontend teste aussi les conversions,
les erreurs, les modèles, la concurrence et les restitutions de mémoire.

## Propriété et frontière de langage

`KMeans` reçoit des slices Go de scalaires. Pendant un unique appel C, le C++
copie les entrées, utilise des objets dont les destructeurs libèrent les
ressources, puis écrit dans les slices de sortie Go. Les pointeurs ne sont pas
conservés après le retour. L’affectation finale utilise une recherche exacte de
voisins avec KD-tree, en conservant les permutations des références et requêtes.
Les centres vides sont conservés pendant les itérations et retirés uniquement à
la fin, sauf option contraire. Le seuil de convergence est la norme de Frobenius
du déplacement des centres effectivement conservés.

Les arbres de données et de centres de `DualTreeKMeans` utilisent un propriétaire
unique. Les constructeurs interrompus de `BinarySpaceTree` libèrent leurs enfants
et leur matrice possédée. Les affectations ne détruisent plus les matrices
empruntées et les déplacements rétablissent le parent des enfants. La variante
naïve alloue ses accumulateurs avant la région OpenMP et intercepte les exceptions
dans celle-ci avant de les relancer sur le thread appelant.

Toutes les fonctions exportées en C interceptent les exceptions C++. Les anciens
bindings les transmettent par un tampon de taille fixe, sous `native.Acquire`.
Le frontend préserve leurs signatures avec `panic(error)` ; l’API moderne rend
`error`. Les matrices sont copiées et libérées avec leurs paramètres natifs ;
les anciens transferts de pointeurs Armadillo non possédés sont supprimés.

Les modèles opaques portent une propriété explicite : les paramètres distinguent
modèles empruntés, exportés et non exportés. Le frontend partage un propriétaire
entre alias et propose `Close`, avec un nettoyage automatique de secours. La
libération des paramètres détruit les modèles non exportés, sans double destruction.
Les 26 bindings qui construisent des modèles utilisent aussi un propriétaire local
avant le transfert vers la sortie : une exception pendant l’entraînement ou la
prédiction ne perd plus l’objet, et les anciennes destructions manuelles ne
libèrent plus les modèles d’entrée empruntés.

La compilation des méthodes dans un exécutable commun impose aussi une définition
partagée des journaux (`MLPACK_GO_MONOLITHIC`) et des vérifications qui utilisent
les paramètres de l’appel. Un nom d’algorithme dans une macro utilisée par une
fonction inline commune violerait la règle de définition unique et pourrait
sélectionner les contraintes d’une autre méthode.

## Cache de compilation

`native.go` référence `src/mlpack.hpp` et `src/mlpack` avec `go:embed` : les
fichiers natifs imbriqués participent ainsi à la clé de cache de Go. La variable
inutilisée est éliminée au lien. Le frontend vérifie l’invalidation après un
changement d’en-tête et l’absence d’un témoin de source dans l’exécutable.
Le cache ne suit pas les en-têtes et bibliothèques d’APT : utiliser
`go build -a ./...` après une modification de ces dépendances.

## Contrôle des allocations

`tests/native_memory.cpp` est un pilote indépendant de Go, compilé avec
AddressSanitizer (ASan) et UndefinedBehaviorSanitizer (UBSan). Il interrompt une
allocation à chaque position successivement, vérifie que l’erreur traverse la
frontière C et compte les `new`/`delete` encore actifs après chaque appel.
LeakSanitizer (LSan) complète le contrôle quand l’environnement permet son
inspection des processus. Ces contrôles couvrent le chemin k-means, les arbres
employés et un modèle de classification créé via l’interface C, y compris sur exception ; ils ne constituent pas une certification de tous les algorithmes mlpack.

L’historique est linéaire : nouveaux commits et nouveaux tags de module immuables.
La version de module `v0.1.0` n’est pas la version C++ amont `4.8.0`.

Les conversions réelles copient maintenant dans des matrices Armadillo possédées.
Un modèle qui déplace la matrice d’entraînement ne peut donc pas conserver un
pointeur vers une slice Go. Les conversions entières copient lors du changement
de type. Les `runtime.Pinner` persistants et le setter générique de pointeur brut
inutilisé sont retirés : seul l’épinglage automatique pendant chaque appel cgo
est nécessaire. Les modèles peuvent survivre au ramasse-miettes Go et à une
modification ultérieure des matrices d’entrée.
