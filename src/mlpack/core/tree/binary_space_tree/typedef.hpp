/**
 * @file core/tree/binary_space_tree/typedef.hpp
 * @author Ryan Curtin
 *
 * Template typedefs for the BinarySpaceTree class that satisfy the requirements
 * of the TreeType policy class.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_CORE_TREE_BINARY_SPACE_TREE_TYPEDEF_HPP
#define MLPACK_CORE_TREE_BINARY_SPACE_TREE_TYPEDEF_HPP

// In case it hasn't been included yet.
#include "../binary_space_tree.hpp"

namespace mlpack {

/**
 * The standard midpoint-split kd-tree.  This is not the original formulation by
 * Bentley but instead the later formulation by Deng and Moore, which only holds
 * points in the leaves of the tree.  When recursively splitting nodes, the
 * KDTree class select the dimension with maximum variance to split on, and
 * picks the midpoint of the range in that dimension as the value on which to
 * split nodes.
 *
 * For more information, see the following papers.
 *
 * @code
 * @article{bentley1975multidimensional,
 *   title={Multidimensional binary search trees used for associative searching},
 *   author={Bentley, J.L.},
 *   journal={Communications of the ACM},
 *   volume={18},
 *   number={9},
 *   pages={509--517},
 *   year={1975},
 *   publisher={ACM}
 * }
 *
 * @inproceedings{deng1995multiresolution,
 *   title={Multiresolution instance-based learning},
 *   author={Deng, K. and Moore, A.W.},
 *   booktitle={Proceedings of the 1995 International Joint Conference on AI
 *       (IJCAI-95)},
 *   pages={1233--1239},
 *   year={1995}
 * }
 * @endcode
 *
 * This template typedef satisfies the TreeType policy API.
 *
 * @see @ref trees, BinarySpaceTree, MeanSplitKDTree
 */
template<typename DistanceType = EuclideanDistance,
         typename StatisticType = EmptyStatistic,
         typename MatType = arma::mat>
using KDTree = BinarySpaceTree<DistanceType,
                               StatisticType,
                               MatType,
                               HRectBound,
                               MidpointSplit>;

} // namespace mlpack

#endif
