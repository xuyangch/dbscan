/**
 *  HMLP (High-Performance Machine Learning Primitives)
 *
 *  Copyright (C) 2014-2018, The University of Texas at Austin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see the LICENSE file.
 *
 **/

/** Use MPI-GOFMM templates. */
#include <gofmm_mpi.hpp>
/** Use dense SPD matrices. */
#include <containers/SPDMatrix.hpp>
/** Use implicit kernel matrices (only coordinates are stored). */
#include <containers/KernelMatrix.hpp>
/** Use STL and HMLP namespaces. */
#include "Distributed_boruvka.hpp"
#include "SingleLinkageTree.hpp"
#include "CondensedTree.hpp"


using namespace std;
using namespace hmlp;

/**
 *  @brief In this example, we explain how you can compute
 *         approximate all-nearest neighbors (ANN) using MPIGOFMM.
 */
int main( int argc, char *argv[] )
{
  try
  {
    /** Use float as data type. */
    using T = double;
    /** [Required] Problem size. */
    size_t n = 5000;
    /** Maximum leaf node size (not used in neighbor search). */
    size_t m = 128;
    /** [Required] Number of nearest neighbors. */
    size_t k = 5;
    /** Maximum off-diagonal rank (not used in neighbor search). */
    size_t s = 128;
    /** Approximation tolerance (not used in neighbor search). */
    T stol = 1E-5;
    /** The amount of direct evaluation (not used in neighbor search). */
    T budget = 0.01;

    /** MPI (Message Passing Interface): check for THREAD_MULTIPLE support. */
    int  provided = 0;
    mpi::Init_thread( &argc, &argv, MPI_THREAD_MULTIPLE, &provided );
    if ( provided != MPI_THREAD_MULTIPLE ) exit( 1 );
    /** MPI (Message Passing Interface): create a specific comm for GOFMM. */
    mpi::Comm CommGOFMM;
    mpi::Comm_dup( MPI_COMM_WORLD, &CommGOFMM );
    /** [Step#0] HMLP API call to initialize the runtime. */
    HANDLE_ERROR( hmlp_init( &argc, &argv, CommGOFMM ) );

    /** Here neighbors1 is distributed in DistData<STAR, CBLK, T> over CommGOFMM. */
    int rank; mpi::Comm_rank( CommGOFMM, &rank );
    int size; mpi::Comm_size( CommGOFMM, &size );

    /** [Step#1] Create a configuration for kernel matrices. */
    gofmm::Configuration<T> config2( GEOMETRY_DISTANCE, n, m, k, s, stol, budget );
    /** [Step#2] Create a distributed Gaussian kernel matrix with random 6D data. */
    size_t d = 6;
    DistData<STAR, CBLK, T> X( d, n, CommGOFMM ); X.randn();
    DistKernelMatrix<T, T> K2( X, CommGOFMM );
    /** [Step#3] Create a distributed randomized splitter. */
    mpigofmm::randomsplit<DistKernelMatrix<T, T>, 2, T> rkdtsplitter2( K2 );
    /** [Step#4] Perform the iterative neighbor search. */
    auto neighbors2 = mpigofmm::FindNeighbors( K2, rkdtsplitter2, config2, CommGOFMM );

    hdbscan::Distributed_Boruvka<double, uint32_t> boruvka(neighbors2, CommGOFMM);
    boruvka.run();

    if (rank == 0) {
      hdbscan::SingleLinkageTree<double, uint32_t> slt{boruvka.edge_set, 50};
      hdbscan::CondensedTree<double, uint32_t> ct{slt};
      ct.print();
    }

    /** [Step#5] HMLP API call to terminate the runtime. */
    HANDLE_ERROR( hmlp_finalize() );
    /** Finalize Message Passing Interface. */
    mpi::Finalize();
  }
  catch ( const exception & e )
  {
    cout << e.what() << endl;
    return -1;
  }
  return 0;
}; /** end main() */
