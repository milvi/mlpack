#ifndef MLPACK_CORE_UTIL_OWNED_MODEL_HPP
#define MLPACK_CORE_UTIL_OWNED_MODEL_HPP

#include <memory>
#include <type_traits>
#include <utility>

namespace mlpack { namespace util {

// La construction reste possédée jusqu’au transfert explicite au résultat.
// Un modèle d’entrée emprunté n’est jamais attaché à ce propriétaire local.
template<typename Model, typename... Args>
Model* MakeOwnedModel(std::unique_ptr<Model>& owner, Args&&... args)
{
  auto model = std::make_unique<Model>(std::forward<Args>(args)...);
  owner = std::move(model);
  return owner.get();
}

// Variante pour une fabrique existante qui transfère un pointeur brut.
template<typename Model>
Model* AdoptOwnedModel(std::unique_ptr<Model>& owner, Model* model)
{
  owner.reset(model);
  return owner.get();
}

}} // namespace mlpack::util
#endif
