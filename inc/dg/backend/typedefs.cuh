#ifndef _DG_TYPEDEFS_CUH_
#define _DG_TYPEDEFS_CUH_

/*! @file

  This file contains useful typedefs of commonly used types.
  */
namespace dg{
typedef thrust::device_vector<double> DVec; //!< Device Vector
typedef thrust::host_vector<double> HVec; //!< Host Vector

typedef cusp::coo_matrix<int, double, cusp::host_memory> Matrix; //!< default matrix
typedef cusp::csr_matrix<int, double, cusp::host_memory> HMatrix; //!< CSR host Matrix
#if THRUST_DEVICE_SYSTEM!=THRUST_DEVICE_SYSTEM_CUDA
typedef cusp::csr_matrix<int, double, cusp::device_memory> DMatrix; //!< most efficient matrix format for omp
#else
typedef cusp::csr_matrix<int, double, cusp::device_memory> DMatrix;
//!< sec efficient matrix format for gpu and only efficient format which can handly plusT, minusT matrices!
// typedef cusp::ell_matrix<int, double, cusp::device_memory> DMatrix; //!< most efficient matrix format for gpu
#endif

}//namespace dg

#endif//_DG_TYPEDEFS_CUH_
