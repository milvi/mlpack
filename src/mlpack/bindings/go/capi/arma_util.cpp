/**
 * @file bindings/go/capi/arma_util.cpp
 * @author Yasmine Dumouchel
 * @author Yashwant Singh
 *
 * Utility function for Go to pass gonum object to an Armadillo Object and
 * vice versa.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#include <mlpack/bindings/go/capi/arma_util.h>
#include "errors.hpp"
#include <algorithm>
#include "io_util.hpp"
#include <mlpack/core/util/io.hpp>

namespace mlpack {
namespace util {

namespace {

// Copie vers un tampon appartenant à Go, sans transférer la propriété native.
template<typename MatType>
bool CopyMatToGo(const MatType& source, double* destination, const size_t length)
{
  try
  {
    if (source.n_elem != length || (length != 0 && destination == nullptr))
      return false;
    if (length != 0)
      std::copy(source.begin(), source.end(), destination);
    return true;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

} // namespace

extern "C" {

bool mlpackCopyParamMat(void* params, const char* identifier,
                       double* destination, const size_t length, const int kind)
{
  try
  {
    if (params == nullptr || identifier == nullptr)
      return false;
    try
    {
      util::Params& p = *static_cast<util::Params*>(params);
      switch (kind)
      {
        case 0: return CopyMatToGo(p.Get<arma::mat>(identifier), destination, length);
        case 1: return CopyMatToGo(p.Get<arma::Mat<size_t>>(identifier), destination, length);
        case 2: return CopyMatToGo(p.Get<arma::rowvec>(identifier), destination, length);
        case 3: return CopyMatToGo(p.Get<arma::Row<size_t>>(identifier), destination, length);
        case 4: return CopyMatToGo(p.Get<arma::colvec>(identifier), destination, length);
        case 5: return CopyMatToGo(p.Get<arma::Col<size_t>>(identifier), destination, length);
        case 6:
          return CopyMatToGo(std::get<1>(p.Get<std::tuple<DatasetInfo, arma::mat>>(identifier)),
                             destination, length);
        default: return false;
      }
    }
    catch (...) { throw; }
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Pass Gonum Dense pointer and wrap an Armadillo mat around it.
 */
void mlpackToArmaMat(void* params,
                     const char* identifier,
                     double* mat,
                     const size_t row,
                     const size_t col,
                     bool transpose)
{
  try
  {
    util::Params& p = *((util::Params*) params);
  
    // Copie propriétaire : aucun emprunt Go ne survit à cet appel.
    arma::mat m(mat, row, col, true);
  
    // Transpose if necessary.
    if (transpose)
      arma::inplace_trans(m);
  
    // Set input parameter with corresponding matrix in IO.
    SetParam(p, identifier, m);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Pass Gonum Dense pointer and wrap an Armadillo mat around it.
 */
void mlpackToArmaUmat(void* params,
                      const char* identifier,
                      double* mat,
                      const size_t row,
                      const size_t col)
{
  try
  {
    util::Params& p = *((util::Params*) params);
  
    // Advanced constructor.
    arma::mat m(mat, row, col, false, false);
  
    arma::Mat<size_t> matr = arma::conv_to<arma::Mat<size_t>>::from(m);
  
    // Set input parameter with corresponding matrix in IO.
    SetParam(p, identifier, matr);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Pass Gonum VecDense pointer and wrap an Armadillo rowvec around it.
 */
void mlpackToArmaRow(void* params,
                     const char* identifier,
                     double* rowvec,
                     const size_t elem)
{
  try
  {
    util::Params& p = *((util::Params*) params);
  
    // Copie propriétaire : aucun emprunt Go ne survit à cet appel.
    arma::rowvec m(rowvec, elem, true);
  
    // Set input parameter with corresponding row in IO.
    SetParam(p, identifier, m);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Pass Gonum VecDense pointer and wrap an Armadillo rowvec around it.
 */
void mlpackToArmaUrow(void* params,
                      const char* identifier,
                      double* rowvec,
                      const size_t elem)
{
  try
  {
    util::Params& p = *((util::Params*) params);
  
    // Advanced constructor.
    arma::rowvec m(rowvec, elem, false, false);
  
    arma::Row<size_t> matr = arma::conv_to<arma::Row<size_t>>::from(m);
  
    // Set input parameter with corresponding row in IO.
    SetParam(p, identifier, matr);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Pass Gonum VecDense pointer and wrap an Armadillo colvec around it.
 */
void mlpackToArmaCol(void* params,
                     const char* identifier,
                     double* colvec,
                     const size_t elem)
{
  try
  {
    util::Params& p = *((util::Params*) params);
  
    // Copie propriétaire : aucun emprunt Go ne survit à cet appel.
    arma::colvec m(colvec, elem, true);
  
    // Set input parameter with corresponding column in IO.
    SetParam(p, identifier, m);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Pass Gonum VecDense pointer and wrap an Armadillo colvec around it.
 */
void mlpackToArmaUcol(void* params,
                      const char* identifier,
                      double* colvec,
                      const size_t elem)
{
  try
  {
    util::Params& p = *((util::Params*) params);
  
    // Advanced constructor.
    arma::colvec m(colvec, elem, false, false);
  
    arma::Col<size_t> matr = arma::conv_to<arma::Col<size_t>>::from(m);
  
    // Set input parameter with corresponding column in IO.
    SetParam(p, identifier, matr);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}
/**
 * Return the memory pointer of an Armadillo mat object.
 */


/**
 * Return the memory pointer of an Armadillo umat object.
 */


/**
 * Return the memory pointer of an Armadillo row object.
 */


/**
 * Return the memory pointer of an Armadillo urow object.
 */


/**
 * Return the memory pointer of an Armadillo col object.
 */


/**
 * Return the memory pointer of an Armadillo ucol object.
 */


/**
 * Return the number of rows in a Armadillo mat.
 */
size_t mlpackNumRowMat(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<arma::mat>(identifier).n_rows;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Return the number of columns in an Armadillo mat.
 */
size_t mlpackNumColMat(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<arma::mat>(identifier).n_cols;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Return the number of elements in an Armadillo mat.
 */
size_t mlpackNumElemMat(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<arma::mat>(identifier).n_elem;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Return the number of rows in an Armadillo umat.
 */
size_t mlpackNumRowUmat(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<arma::Mat<size_t>>(identifier).n_rows;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Return the number of columns in an Armadillo umat.
 */
size_t mlpackNumColUmat(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<arma::Mat<size_t>>(identifier).n_cols;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Return the number of elements in an Armadillo umat.
 */
size_t mlpackNumElemUmat(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<arma::Mat<size_t>>(identifier).n_elem;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Return the number of elements in an Armadillo row.
 */
size_t mlpackNumElemRow(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<arma::Row<double>>(identifier).n_elem;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Return the number of elements in an Armadillo urow.
 */
size_t mlpackNumElemUrow(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<arma::Row<size_t>>(identifier).n_elem;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Return the number of elements in an Armadillo col.
 */
size_t mlpackNumElemCol(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<arma::Col<double>>(identifier).n_elem;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Return the number of elements in an Armadillo ucol.
 */
size_t mlpackNumElemUcol(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    return p.Get<arma::Col<size_t>>(identifier).n_elem;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Call IO::SetParam<std::tuple<DatasetInfo, arma::mat>>().
 */
void mlpackToArmaMatWithInfo(void* params,
                             const char* identifier,
                             const bool* dimensions,
                             double* memptr,
                             const size_t rows,
                             const size_t cols)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    DatasetInfo d(rows);
    bool hasCategoricals = false;
    for (size_t i = 0; i < d.Dimensionality(); ++i)
    {
      d.Type(i) = (dimensions[i]) ? Datatype::categorical : Datatype::numeric;
      if (dimensions[i])
        hasCategoricals = true;
    }
  
    arma::mat m(memptr, rows, cols, true);
  
    // Do we need to find how many categories we have?
    if (hasCategoricals)
    {
      arma::vec maxs = arma::max(m, 1) + 1;
  
      for (size_t i = 0; i < d.Dimensionality(); ++i)
      {
        if (dimensions[i])
        {
          // Map the right number of objects.
          for (size_t j = 0; j < (size_t) maxs[i]; ++j)
          {
            std::ostringstream oss;
            oss << j;
            d.MapString<double>(oss.str(), i);
          }
        }
      }
    }
  
    std::get<0>(p.Get<std::tuple<DatasetInfo, arma::mat>>(identifier)) =
        std::move(d);
    std::get<1>(p.Get<std::tuple<DatasetInfo, arma::mat>>(identifier)) =
        std::move(m);
    p.SetPassed(identifier);
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
}

/**
 * Get the number of elements in a matrix with DatasetInfo parameter.
 */
size_t mlpackArmaMatWithInfoElements(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    using TupleType = std::tuple<DatasetInfo, arma::mat>;
    return std::get<1>(p.Get<TupleType>(identifier)).n_elem;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Get the number of rows in a matrix with DatasetInfo parameter.
 */
size_t mlpackArmaMatWithInfoRows(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    using TupleType = std::tuple<DatasetInfo, arma::mat>;
    return std::get<1>(p.Get<TupleType>(identifier)).n_rows;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Get the number of columns in a matrix with DatasetInfo parameter.
 */
size_t mlpackArmaMatWithInfoCols(void* params, const char* identifier)
{
  try
  {
    util::Params& p = *((util::Params*) params);
    using TupleType = std::tuple<DatasetInfo, arma::mat>;
    return std::get<1>(p.Get<TupleType>(identifier)).n_cols;
  }
  catch (const std::exception& error) { MilviSetError(error.what()); }
  catch (...) { MilviSetError("exception C++ inconnue"); }
  return 0;
}

/**
 * Get a pointer to the memory of the matrix.  The calling function is expected
 * to own the memory.
 */


} // extern "C"

} // namespace util
} // namespace mlpack

