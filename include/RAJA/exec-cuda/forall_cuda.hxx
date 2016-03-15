/*
 * Copyright (c) 2016, Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory.
 *
 * All rights reserved.
 *
 * This source code cannot be distributed without permission and
 * further review from Lawrence Livermore National Laboratory.
 */

/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Header file containing RAJA segment template methods for 
 *          execution via CUDA kernel launch.
 *
 *          These methods should work on any platform that supports 
 *          CUDA devices.
 *
 ******************************************************************************
 */

#ifndef RAJA_forall_cuda_HXX
#define RAJA_forall_cuda_HXX

#include "RAJA/config.hxx"

#include "RAJA/int_datatypes.hxx"

#include "RAJA/fault_tolerance.hxx"

#include <iostream>
#include <cstdlib>

#include <cfloat>


namespace RAJA {


//
//////////////////////////////////////////////////////////////////////
//
// CUDA kernel templates.
//
//////////////////////////////////////////////////////////////////////
//

/*!
 ******************************************************************************
 *
 * \brief CUDA kernal forall template for index range.
 *
 ******************************************************************************
 */
template <typename LOOP_BODY>
__global__ void forall_cuda_kernel(LOOP_BODY loop_body,
                                   Index_type begin, Index_type len)
{
   Index_type ii = blockDim.x * blockIdx.x + threadIdx.x;
   if (ii < len) {
      loop_body(begin+ii);
   }
}

/*!
 ******************************************************************************
 *
 * \brief CUDA kernal forall_Icount template for index range.
 *
 *         NOTE: lambda loop body requires two args (icount, index).
 *
 ******************************************************************************
 */
template <typename LOOP_BODY>
__global__ void forall_Icount_cuda_kernel(LOOP_BODY loop_body,
                                          Index_type begin, Index_type len,
                                          Index_type icount)
{
   Index_type ii = blockDim.x * blockIdx.x + threadIdx.x;
   if (ii < len) {
      loop_body(ii+icount, ii+begin);
   }
}

/*!
 ******************************************************************************
 *
 * \brief  CUDA kernal forall template for indirection array.
 *
 ******************************************************************************
 */
template <typename LOOP_BODY>
__global__ void forall_cuda_kernel(LOOP_BODY loop_body, 
                                   const Index_type* idx, 
                                   Index_type length)
{
   Index_type ii = blockDim.x * blockIdx.x + threadIdx.x;
   if (ii < length) {
      loop_body(idx[ii]);
   }
}

/*!
 ******************************************************************************
 * 
 * \brief  CUDA kernal forall_Icount template for indiraction array.
 *
 *         NOTE: lambda loop body requires two args (icount, index).
 *
 ******************************************************************************
 */
template <typename LOOP_BODY>
__global__ void forall_Icount_cuda_kernel(LOOP_BODY loop_body,
                                          const Index_type* idx,
                                          Index_type length,
                                          Index_type icount)
{
   Index_type ii = blockDim.x * blockIdx.x + threadIdx.x;
   if (ii < length) {
      loop_body(ii+icount, idx[ii]);
   }
}


//
////////////////////////////////////////////////////////////////////////
//
// Function templates for CUDA execution over index ranges.
//
////////////////////////////////////////////////////////////////////////
//

/*!
 ******************************************************************************
 *
 * \brief  Forall execution over index range via CUDA kernal launch.
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall(cuda_exec<BLOCK_SIZE>,
            Index_type begin, Index_type end, 
            LOOP_BODY loop_body)
{
   Index_type len = end - begin;
   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body, 
                                                begin, len);
   gpuErrchk(cudaPeekAtLastError());
   gpuErrchk(cudaDeviceSynchronize());

   RAJA_FT_END ;
}

/*!
 ******************************************************************************
 *
 * \brief  Forall execution over index range via CUDA kernal launch 
 *         without call to cudaDeviceSynchronize() after kernel completes.
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall(cuda_exec_async<BLOCK_SIZE>,
            Index_type begin, Index_type end,
            LOOP_BODY loop_body)
{
   Index_type len = end - begin;
   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body,
                                                begin, len);
   gpuErrchk(cudaPeekAtLastError());

   RAJA_FT_END ;
}

/*!
 ******************************************************************************
 *
 * \brief  Forall execution over index range with index count,
 *         via CUDA kernal launch.
 *
 *         NOTE: lambda loop body requires two args (icount, index).
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall_Icount(cuda_exec<BLOCK_SIZE>,
                   Index_type begin, Index_type end,
                   Index_type icount,
                   LOOP_BODY loop_body)
{
   Index_type len = end - begin;

   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_Icount_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body, 
                                                       begin, len,
                                                       icount);

   gpuErrchk(cudaPeekAtLastError());
   gpuErrchk(cudaDeviceSynchronize());

   RAJA_FT_END ;
}

/*!
 ******************************************************************************
 *
 * \brief  Forall execution over index range with index count,
 *         via CUDA kernal launch without call to cudaDeviceSynchronize() 
 *         after kernel completes.
 *
 *         NOTE: lambda loop body requires two args (icount, index).
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall_Icount(cuda_exec_async<BLOCK_SIZE>,
                   Index_type begin, Index_type end,
                   Index_type icount,
                   LOOP_BODY loop_body)
{
   Index_type len = end - begin;

   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_Icount_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body,
                                                       begin, len,
                                                       icount);
   gpuErrchk(cudaPeekAtLastError());

   RAJA_FT_END ;
}


//
////////////////////////////////////////////////////////////////////////
//
// Function templates for CUDA execution over range segments.
//
////////////////////////////////////////////////////////////////////////
//

/*!
 ******************************************************************************
 *
 * \brief  Forall execution over range segment object via CUDA kernal launch.
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall(cuda_exec<BLOCK_SIZE>,
            const RangeSegment& iseg,
            LOOP_BODY loop_body)
{
   Index_type begin = iseg.getBegin();
   Index_type end   = iseg.getEnd();
   Index_type len = end - begin;

   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body, 
                                                begin, len);

   gpuErrchk(cudaPeekAtLastError());
   gpuErrchk(cudaDeviceSynchronize());

   RAJA_FT_END ;
}

/*!
 ******************************************************************************
 *
 * \brief  Forall execution over range segment object via CUDA kernal launch
 *         without call to cudaDeviceSynchronize() after kernel completes.
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall(cuda_exec_async<BLOCK_SIZE>,
            const RangeSegment& iseg,
            LOOP_BODY loop_body)
{
   Index_type begin = iseg.getBegin();
   Index_type end   = iseg.getEnd();
   Index_type len = end - begin;

   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body,
                                                begin, len);
   gpuErrchk(cudaPeekAtLastError());
   
   RAJA_FT_END ;
}

/*!
 ******************************************************************************
 *
 * \brief  Forall execution over range segment object with index count
 *         via CUDA kernal launch.
 *
 *         NOTE: lambda loop body requires two args (icount, index).
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall_Icount(cuda_exec<BLOCK_SIZE>,
                   const RangeSegment& iseg,
                   Index_type icount,
                   LOOP_BODY loop_body)
{
   Index_type begin = iseg.getBegin();
   Index_type len = iseg.getEnd() - begin;

   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_Icount_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body, 
                                                       begin, len,
                                                       icount);

   gpuErrchk(cudaPeekAtLastError());
   gpuErrchk(cudaDeviceSynchronize());

   RAJA_FT_END ;
}

/*!
 ******************************************************************************
 *
 * \brief  Forall execution over range segment object with index count
 *         via CUDA kernal launch without call to cudaDeviceSynchronize() 
 *         after kernel completes.
 *
 *         NOTE: lambda loop body requires two args (icount, index).
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall_Icount(cuda_exec_async<BLOCK_SIZE>,
                   const RangeSegment& iseg,
                   Index_type icount,
                   LOOP_BODY loop_body)
{
   Index_type begin = iseg.getBegin();
   Index_type len = iseg.getEnd() - begin;

   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_Icount_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body,
                                                       begin, len,
                                                       icount);
   gpuErrchk(cudaPeekAtLastError());
   
   RAJA_FT_END ;
}


//
////////////////////////////////////////////////////////////////////////
//
// Function templates that iterate over indirection arrays. 
//
////////////////////////////////////////////////////////////////////////
//

/*!
 ******************************************************************************
 *
 * \brief  Forall execution for indirection array via CUDA kernal launch.
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall(cuda_exec<BLOCK_SIZE>,
            const Index_type* idx, Index_type len,
            LOOP_BODY loop_body)
{
   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body, 
                                                idx, len);

   gpuErrchk(cudaPeekAtLastError());
   gpuErrchk(cudaDeviceSynchronize());

   RAJA_FT_END ;
}

/*!
 ******************************************************************************
 *
 * \brief  Forall execution for indirection array via CUDA kernal launch
 *         without call to cudaDeviceSynchronize() after kernel completes.
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall(cuda_exec_async<BLOCK_SIZE>,
            const Index_type* idx, Index_type len,
            LOOP_BODY loop_body)
{
   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body,
                                                idx, len);
   gpuErrchk(cudaPeekAtLastError());

   RAJA_FT_END ;
}


/*!
 ******************************************************************************
 *
 * \brief  Forall execution over indirection array with index count
 *         via CUDA kernal launch.
 *
 *         NOTE: lambda loop body requires two args (icount, index).
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall_Icount(cuda_exec<BLOCK_SIZE>,
                   const Index_type* idx, Index_type len,
                   Index_type icount,
                   LOOP_BODY loop_body)
{
   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_Icount_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body,
                                                       idx, len,
                                                       icount);

   gpuErrchk(cudaPeekAtLastError());
   gpuErrchk(cudaDeviceSynchronize());

   RAJA_FT_END ;
}

/*!
 ******************************************************************************
 *
 * \brief  Forall execution over indirection array with index count
 *         via CUDA kernal launch without call to cudaDeviceSynchronize() 
 *         after kernel completes.
 *
 *         NOTE: lambda loop body requires two args (icount, index).
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall_Icount(cuda_exec_async<BLOCK_SIZE>,
                   const Index_type* idx, Index_type len,
                   Index_type icount,
                   LOOP_BODY loop_body)
{
   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_Icount_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body,
                                                       idx, len,
                                                       icount);
   gpuErrchk(cudaPeekAtLastError());

   RAJA_FT_END ;
}


//
////////////////////////////////////////////////////////////////////////
//
// Function templates that iterate over list segments.
//
////////////////////////////////////////////////////////////////////////
//

/*!
 ******************************************************************************
 *
 * \brief  Forall execution for list segment object via CUDA kernal launch.
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall(cuda_exec<BLOCK_SIZE>,
            const ListSegment& iseg,
            LOOP_BODY loop_body)
{
   const Index_type* idx = iseg.getIndex();
   Index_type len = iseg.getLength();

   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body, 
                                                idx, len);

   gpuErrchk(cudaPeekAtLastError());
   gpuErrchk(cudaDeviceSynchronize());

   RAJA_FT_END ;
}

/*!
 ******************************************************************************
 *
 * \brief  Forall execution for list segment object via CUDA kernal launch
 *         without call to cudaDeviceSynchronize() after kernel completes.
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall(cuda_exec_async<BLOCK_SIZE>,
            const ListSegment& iseg,
            LOOP_BODY loop_body)
{
   const Index_type* idx = iseg.getIndex();
   Index_type len = iseg.getLength();

   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body,
                                                idx, len);
   gpuErrchk(cudaPeekAtLastError());
   
   RAJA_FT_END ;
}

/*!
 ******************************************************************************
 *
 * \brief  Forall execution over list segment object with index count
 *         via CUDA kernal launch.
 *
 *         NOTE: lambda loop body requires two args (icount, index).
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall_Icount(cuda_exec<BLOCK_SIZE>,
                   const ListSegment& iseg,
                   Index_type icount,
                   LOOP_BODY loop_body)
{
   const Index_type* idx = iseg.getIndex();
   Index_type len = iseg.getLength();

   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_Icount_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body,
                                                       idx, len,
                                                       icount);

   gpuErrchk(cudaPeekAtLastError());
   gpuErrchk(cudaDeviceSynchronize());

   RAJA_FT_END ;
}

/*!
 ******************************************************************************
 *
 * \brief  Forall execution over list segment object with index count
 *         via CUDA kernal launch without call to cudaDeviceSynchronize() 
 *         after kernel completes.
 *
 *         NOTE: lambda loop body requires two args (icount, index).
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall_Icount(cuda_exec_async<BLOCK_SIZE>,
                   const ListSegment& iseg,
                   Index_type icount,
                   LOOP_BODY loop_body)
{
   const Index_type* idx = iseg.getIndex();
   Index_type len = iseg.getLength();

   size_t gridSize = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

   RAJA_FT_BEGIN ;

   forall_Icount_cuda_kernel<<<gridSize, BLOCK_SIZE>>>(loop_body,
                                                       idx, len,
                                                       icount);
   gpuErrchk(cudaPeekAtLastError());
   
   RAJA_FT_END ;
}


//
//////////////////////////////////////////////////////////////////////
//
// The following function templates iterate over index set segments
// using the explicitly named segment iteration policy and execute
// segments as CUDA kernels.
//
//////////////////////////////////////////////////////////////////////
//

/*!
 ******************************************************************************
 *
 * \brief  Sequential iteration over segments of index set and
 *         CUDA execution for segments.
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall( IndexSet::ExecPolicy< seq_segit, cuda_exec<BLOCK_SIZE> >,
             const IndexSet& iset,
             LOOP_BODY loop_body )
{
   int num_seg = iset.getNumSegments();
   for ( int isi = 0; isi < num_seg; ++isi ) {

      const IndexSetSegInfo* seg_info = iset.getSegmentInfo(isi);
      executeRangeList_forall< cuda_exec_async<BLOCK_SIZE> >(
                               seg_info, loop_body );

   } // iterate over segments of index set

   gpuErrchk(cudaPeekAtLastError());
   gpuErrchk(cudaDeviceSynchronize());
}

/*!
 ******************************************************************************
 *
 * \brief  Sequential iteration over segments of index set and
 *         CUDA execution for segments.
 *
 *         This method passes index count to segment iteration.
 *
 *         NOTE: lambda loop body requires two args (icount, index).
 *
 ******************************************************************************
 */
template <size_t BLOCK_SIZE, typename LOOP_BODY>
RAJA_INLINE
void forall_Icount( IndexSet::ExecPolicy< seq_segit, cuda_exec<BLOCK_SIZE> >,
                    const IndexSet& iset,
                    LOOP_BODY loop_body )
{
   int num_seg = iset.getNumSegments();
   for ( int isi = 0; isi < num_seg; ++isi ) {

      const IndexSetSegInfo* seg_info = iset.getSegmentInfo(isi);
      executeRangeList_forall_Icount< cuda_exec_async<BLOCK_SIZE> >(
                                      seg_info, loop_body );

   } // iterate over segments of index set

   gpuErrchk(cudaPeekAtLastError());
   gpuErrchk(cudaDeviceSynchronize());
}


}  // closing brace for RAJA namespace


#endif  // closing endif for header file include guard