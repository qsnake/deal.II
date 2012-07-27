//---------------------------------------------------------------------------
//    $Id: trilinos_sparse_matrix.cc 23986 2011-08-02 08:55:36Z kronbichler $
//    Version: $Name$
//
//    Copyright (C) 2008, 2009, 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <deal.II/lac/trilinos_sparse_matrix.h>

#ifdef DEAL_II_USE_TRILINOS

#  include <deal.II/base/utilities.h>
#  include <deal.II/lac/sparse_matrix.h>
#  include <deal.II/lac/trilinos_sparsity_pattern.h>
#  include <deal.II/lac/sparsity_pattern.h>
#  include <deal.II/lac/compressed_sparsity_pattern.h>
#  include <deal.II/lac/compressed_set_sparsity_pattern.h>
#  include <deal.II/lac/compressed_simple_sparsity_pattern.h>

#  include <ml_epetra_utils.h>
#  include <ml_struct.h>
#  include <Teuchos_RCP.hpp>

DEAL_II_NAMESPACE_OPEN

namespace TrilinosWrappers
{
  namespace MatrixIterators
  {
    void
    SparseMatrix::const_iterator::Accessor::
    visit_present_row ()
    {
				  // if we are asked to visit the
				  // past-the-end line, then simply
				  // release all our caches and go on
				  // with life
      if (this->a_row == matrix->m())
	{
	  colnum_cache.reset ();
	  value_cache.reset ();

	  return;
	}

				  // otherwise first flush Trilinos caches
      matrix->compress ();

				  // get a representation of the present
				  // row
      int ncols;
      int colnums = matrix->n();
      if (value_cache.get() == 0)
	{
	  value_cache.reset (new std::vector<TrilinosScalar> (matrix->n()));
	  colnum_cache.reset (new std::vector<unsigned int> (matrix->n()));
	}
      else
	{
	  value_cache->resize (matrix->n());
	  colnum_cache->resize (matrix->n());
	}

      int ierr = matrix->trilinos_matrix().
	ExtractGlobalRowCopy((int)this->a_row,
			     colnums,
			     ncols, &((*value_cache)[0]),
			     reinterpret_cast<int*>(&((*colnum_cache)[0])));
      value_cache->resize (ncols);
      colnum_cache->resize (ncols);
      AssertThrow (ierr == 0, ExcTrilinosError(ierr));

				  // copy it into our caches if the
				  // line isn't empty. if it is, then
				  // we've done something wrong, since
				  // we shouldn't have initialized an
				  // iterator for an empty line (what
				  // would it point to?)
    }
  }


				  // The constructor is actually the
				  // only point where we have to check
				  // whether we build a serial or a
				  // parallel Trilinos matrix.
				  // Actually, it does not even matter
				  // how many threads there are, but
				  // only if we use an MPI compiler or
				  // a standard compiler. So, even one
				  // thread on a configuration with
				  // MPI will still get a parallel
				  // interface.
  SparseMatrix::SparseMatrix ()
		  :
                  column_space_map (new Epetra_Map (0, 0,
						     Utilities::Trilinos::comm_self())),
		  matrix (new Epetra_FECrsMatrix(View, *column_space_map,
						  *column_space_map, 0)),
		  last_action (Zero),
		  compressed (true)
  {
    matrix->FillComplete();
  }



  SparseMatrix::SparseMatrix (const Epetra_Map  &input_map,
			      const unsigned int n_max_entries_per_row)
		  :
                  column_space_map (new Epetra_Map (input_map)),
		  matrix (new Epetra_FECrsMatrix(Copy, *column_space_map,
						  int(n_max_entries_per_row), false)),
		  last_action (Zero),
		  compressed (false)
  {}



  SparseMatrix::SparseMatrix (const Epetra_Map                &input_map,
			      const std::vector<unsigned int> &n_entries_per_row)
		  :
                  column_space_map (new Epetra_Map (input_map)),
		  matrix (new Epetra_FECrsMatrix
			  (Copy, *column_space_map,
			   (int*)const_cast<unsigned int*>(&(n_entries_per_row[0])),
			   false)),
		  last_action (Zero),
		  compressed (false)
  {}



  SparseMatrix::SparseMatrix (const Epetra_Map  &input_row_map,
			      const Epetra_Map  &input_col_map,
			      const unsigned int n_max_entries_per_row)
		  :
                  column_space_map (new Epetra_Map (input_col_map)),
		  matrix (new Epetra_FECrsMatrix(Copy, input_row_map,
						 int(n_max_entries_per_row), false)),
		  last_action (Zero),
		  compressed (false)
  {}



  SparseMatrix::SparseMatrix (const Epetra_Map                &input_row_map,
			      const Epetra_Map                &input_col_map,
			      const std::vector<unsigned int> &n_entries_per_row)
		  :
                  column_space_map (new Epetra_Map (input_col_map)),
		  matrix (new Epetra_FECrsMatrix(Copy, input_row_map,
		      (int*)const_cast<unsigned int*>(&(n_entries_per_row[0])),
						 false)),
		  last_action (Zero),
		  compressed (false)
  {}



  SparseMatrix::SparseMatrix (const unsigned int m,
			      const unsigned int n,
			      const unsigned int n_max_entries_per_row)
		  :
                  column_space_map (new Epetra_Map (n, 0,
						    Utilities::Trilinos::comm_self())),

				   // on one processor only, we know how the
				   // columns of the matrix will be
				   // distributed (everything on one
				   // processor), so we can hand in this
				   // information to the constructor. we
				   // can't do so in parallel, where the
				   // information from columns is only
				   // available when entries have been added
		  matrix (new Epetra_FECrsMatrix(Copy,
						 Epetra_Map (m, 0,
							     Utilities::Trilinos::comm_self()),
						 *column_space_map,
						 n_max_entries_per_row,
						 false)),
		  last_action (Zero),
		  compressed (false)
  {}



  SparseMatrix::SparseMatrix (const unsigned int               m,
			      const unsigned int               n,
			      const std::vector<unsigned int> &n_entries_per_row)
		  :
                  column_space_map (new Epetra_Map (n, 0,
						    Utilities::Trilinos::comm_self())),
		  matrix (new Epetra_FECrsMatrix(Copy,
						 Epetra_Map (m, 0,
							     Utilities::Trilinos::comm_self()),
						 *column_space_map,
			   (int*)const_cast<unsigned int*>(&(n_entries_per_row[0])),
						 false)),
		  last_action (Zero),
		  compressed (false)
  {}



  SparseMatrix::SparseMatrix (const IndexSet     &parallel_partitioning,
			      const MPI_Comm     &communicator,
			      const unsigned int n_max_entries_per_row)
		  :
                  column_space_map (new Epetra_Map(parallel_partitioning.
						   make_trilinos_map(communicator, false))),
		  matrix (new Epetra_FECrsMatrix(Copy,
						 *column_space_map,
						 n_max_entries_per_row,
						 false)),
		  last_action (Zero),
		  compressed (false)
  {}



  SparseMatrix::SparseMatrix (const IndexSet     &parallel_partitioning,
			      const MPI_Comm     &communicator,
			      const std::vector<unsigned int> &n_entries_per_row)
		  :
                  column_space_map (new Epetra_Map(parallel_partitioning.
						   make_trilinos_map(communicator, false))),
		  matrix (new Epetra_FECrsMatrix(Copy,
						 *column_space_map,
			   (int*)const_cast<unsigned int*>(&(n_entries_per_row[0])),
						 false)),
		  last_action (Zero),
		  compressed (false)
  {}



  SparseMatrix::SparseMatrix (const IndexSet     &row_parallel_partitioning,
			      const IndexSet     &col_parallel_partitioning,
			      const MPI_Comm     &communicator,
			      const unsigned int n_max_entries_per_row)
		  :
                  column_space_map (new Epetra_Map(col_parallel_partitioning.
						   make_trilinos_map(communicator, false))),
		  matrix (new Epetra_FECrsMatrix(Copy,
						 row_parallel_partitioning.
						 make_trilinos_map(communicator, false),
						 n_max_entries_per_row,
						 false)),
		  last_action (Zero),
		  compressed (false)
  {}



  SparseMatrix::SparseMatrix (const IndexSet     &row_parallel_partitioning,
			      const IndexSet     &col_parallel_partitioning,
			      const MPI_Comm     &communicator,
			      const std::vector<unsigned int> &n_entries_per_row)
		  :
                  column_space_map (new Epetra_Map(col_parallel_partitioning.
						   make_trilinos_map(communicator, false))),
		  matrix (new Epetra_FECrsMatrix(Copy,
						 row_parallel_partitioning.
						 make_trilinos_map(communicator, false),
			   (int*)const_cast<unsigned int*>(&(n_entries_per_row[0])),
						 false)),
		  last_action (Zero),
		  compressed (false)
  {}



  SparseMatrix::SparseMatrix (const SparsityPattern &sparsity_pattern)
		  :
		  column_space_map (new Epetra_Map (sparsity_pattern.domain_partitioner())),
		  matrix (new Epetra_FECrsMatrix(Copy,
						 sparsity_pattern.trilinos_sparsity_pattern(),
						 false)),
		  last_action (Zero),
		  compressed (true)
  {
    Assert(sparsity_pattern.trilinos_sparsity_pattern().Filled() == true,
	   ExcMessage("The Trilinos sparsity pattern has not been compressed."));
    compress();
  }



  SparseMatrix::SparseMatrix (const SparseMatrix &input_matrix)
		  :
                  Subscriptor(),
		  column_space_map (new Epetra_Map (input_matrix.domain_partitioner())),
		  matrix (new Epetra_FECrsMatrix(*input_matrix.matrix)),
		  last_action (Zero),
		  compressed (true)
  {}



  SparseMatrix::~SparseMatrix ()
  {}



  void
  SparseMatrix::copy_from (const SparseMatrix &m)
  {

				   // check whether we need to update the
				   // partitioner or can just copy the data:
				   // in case we have the same distribution,
				   // we can just copy the data.
    if (local_range() == m.local_range())
      *matrix = *m.matrix;
    else
      {
	column_space_map.reset (new Epetra_Map (m.domain_partitioner()));

				// release memory before reallocation
	matrix.reset ();
	temp_vector.clear ();
	matrix.reset (new Epetra_FECrsMatrix(*m.matrix));
      }

    compress();
  }



  template <typename SparsityType>
  void
  SparseMatrix::reinit (const SparsityType &sparsity_pattern)
  {
    const Epetra_Map rows (sparsity_pattern.n_rows(),
			   0,
			   Utilities::Trilinos::comm_self());
    const Epetra_Map columns (sparsity_pattern.n_cols(),
			      0,
			      Utilities::Trilinos::comm_self());

    reinit (rows, columns, sparsity_pattern);
  }



  template <typename SparsityType>
  void
  SparseMatrix::reinit (const Epetra_Map    &input_map,
			const SparsityType  &sparsity_pattern,
			const bool           exchange_data)
  {
    reinit (input_map, input_map, sparsity_pattern, exchange_data);
  }



  template <typename SparsityType>
  void
  SparseMatrix::reinit (const Epetra_Map    &input_row_map,
			const Epetra_Map    &input_col_map,
			const SparsityType  &sparsity_pattern,
			const bool           exchange_data)
  {
				// release memory before reallocation
    temp_vector.clear();
    matrix.reset();

				// if we want to exchange data, build
				// a usual Trilinos sparsity pattern
				// and let that handle the
				// exchange. otherwise, manually
				// create a CrsGraph, which consumes
				// considerably less memory because it
				// can set correct number of indices
				// right from the start
    if (exchange_data)
      {
	SparsityPattern trilinos_sparsity;
	trilinos_sparsity.reinit (input_row_map, input_col_map,
				  sparsity_pattern, exchange_data);
	reinit (trilinos_sparsity);

	return;
      }

    Assert (exchange_data == false, ExcNotImplemented());
    if (input_row_map.Comm().MyPID() == 0)
      {
	AssertDimension (sparsity_pattern.n_rows(),
			 static_cast<unsigned int>(input_row_map.NumGlobalElements()));
	AssertDimension (sparsity_pattern.n_cols(),
			 static_cast<unsigned int>(input_col_map.NumGlobalElements()));
      }

    column_space_map.reset (new Epetra_Map (input_col_map));

    const unsigned int first_row = input_row_map.MinMyGID(),
      last_row = input_row_map.MaxMyGID()+1;
    std::vector<int> n_entries_per_row(last_row-first_row);

    for (unsigned int row=first_row; row<last_row; ++row)
      n_entries_per_row[row-first_row] = sparsity_pattern.row_length(row);

				  // The deal.II notation of a Sparsity
				  // pattern corresponds to the Epetra
				  // concept of a Graph. Hence, we generate
				  // a graph by copying the sparsity pattern
				  // into it, and then build up the matrix
				  // from the graph. This is considerable
				  // faster than directly filling elements
				  // into the matrix. Moreover, it consumes
				  // less memory, since the internal
				  // reordering is done on ints only, and we
				  // can leave the doubles aside.

				   // for more than one processor, need to
				   // specify only row map first and let the
				   // matrix entries decide about the column
				   // map (which says which columns are
				   // present in the matrix, not to be
				   // confused with the col_map that tells
				   // how the domain dofs of the matrix will
				   // be distributed). for only one
				   // processor, we can directly assign the
				   // columns as well. Compare this with bug
				   // # 4123 in the Sandia Bugzilla.
    std_cxx1x::shared_ptr<Epetra_CrsGraph> graph;
    if (input_row_map.Comm().NumProc() > 1)
      graph.reset (new Epetra_CrsGraph (Copy, input_row_map,
					&n_entries_per_row[0], true));
    else
      graph.reset (new Epetra_CrsGraph (Copy, input_row_map, input_col_map,
					&n_entries_per_row[0], true));

				  // This functions assumes that the
				  // sparsity pattern sits on all processors
				  // (completely). The parallel version uses
				  // an Epetra graph that is already
				  // distributed.

				  // now insert the indices
    std::vector<int>   row_indices;

    for (unsigned int row=first_row; row<last_row; ++row)
      {
	const int row_length = sparsity_pattern.row_length(row);
	if (row_length == 0)
	  continue;

	row_indices.resize (row_length, -1);

	typename SparsityType::row_iterator col_num = sparsity_pattern.row_begin (row),
	  row_end = sparsity_pattern.row_end(row);
	for (unsigned int col = 0; col_num != row_end; ++col_num, ++col)
	  row_indices[col] = *col_num;

	graph->Epetra_CrsGraph::InsertGlobalIndices (row, row_length,
						     &row_indices[0]);
      }

				  // Eventually, optimize the graph
				  // structure (sort indices, make memory
				  // contiguous, etc).
    graph->FillComplete(input_col_map, input_row_map);
    graph->OptimizeStorage();

				   // check whether we got the number of
				   // columns right.
    AssertDimension (sparsity_pattern.n_cols(),
		     static_cast<unsigned int>(graph->NumGlobalCols()));

				  // And now finally generate the matrix.
    matrix.reset (new Epetra_FECrsMatrix(Copy, *graph, false));
    last_action = Zero;

				  // In the end, the matrix needs to
				  // be compressed in order to be
				  // really ready.
    compress();
  }



  void
  SparseMatrix::reinit (const SparsityPattern &sparsity_pattern)
  {
    temp_vector.clear ();
    matrix.reset ();

				   // reinit with a (parallel) Trilinos
				   // sparsity pattern.
    column_space_map.reset (new Epetra_Map
			    (sparsity_pattern.domain_partitioner()));
    matrix.reset (new Epetra_FECrsMatrix
		  (Copy, sparsity_pattern.trilinos_sparsity_pattern(), false));
    compress();
  }



  void
  SparseMatrix::reinit (const SparseMatrix &sparse_matrix)
  {
    column_space_map.reset (new Epetra_Map (sparse_matrix.domain_partitioner()));
    temp_vector.clear ();
    matrix.reset ();
    matrix.reset (new Epetra_FECrsMatrix
		  (Copy, sparse_matrix.trilinos_sparsity_pattern(), false));

    compress();
  }



  template <typename number>
  void
  SparseMatrix::reinit (const ::dealii::SparseMatrix<number> &dealii_sparse_matrix,
			const double                          drop_tolerance,
			const bool                            copy_values,
			const ::dealii::SparsityPattern      *use_this_sparsity)
  {
    const Epetra_Map rows (dealii_sparse_matrix.m(),
			   0,
			   Utilities::Trilinos::comm_self());
    const Epetra_Map columns (dealii_sparse_matrix.n(),
			      0,
			      Utilities::Trilinos::comm_self());
    reinit (rows, columns, dealii_sparse_matrix, drop_tolerance,
	    copy_values, use_this_sparsity);
  }



  template <typename number>
  void
  SparseMatrix::reinit (const Epetra_Map                     &input_map,
			const ::dealii::SparseMatrix<number> &dealii_sparse_matrix,
			const double                          drop_tolerance,
			const bool                            copy_values,
			const ::dealii::SparsityPattern      *use_this_sparsity)
  {
    reinit (input_map, input_map, dealii_sparse_matrix, drop_tolerance,
	    copy_values, use_this_sparsity);
  }



  template <typename number>
  void
  SparseMatrix::reinit (const Epetra_Map                     &input_row_map,
			const Epetra_Map                     &input_col_map,
			const ::dealii::SparseMatrix<number> &dealii_sparse_matrix,
			const double                          drop_tolerance,
			const bool                            copy_values,
			const ::dealii::SparsityPattern      *use_this_sparsity)
  {
    if (copy_values == false)
      {
				   // in case we do not copy values, just
				   // call the other function.
	if (use_this_sparsity == 0)
	  reinit (input_row_map, input_col_map,
		  dealii_sparse_matrix.get_sparsity_pattern());
	else
	  reinit (input_row_map, input_col_map,
		  *use_this_sparsity);
	return;
      }

    unsigned int n_rows = dealii_sparse_matrix.m();

    Assert (input_row_map.NumGlobalElements() == (int)n_rows,
	    ExcDimensionMismatch (input_row_map.NumGlobalElements(),
				  n_rows));
    Assert (input_col_map.NumGlobalElements() == (int)dealii_sparse_matrix.n(),
	    ExcDimensionMismatch (input_col_map.NumGlobalElements(),
				  dealii_sparse_matrix.n()));

    const ::dealii::SparsityPattern & sparsity_pattern =
      (use_this_sparsity!=0)? *use_this_sparsity :
      dealii_sparse_matrix.get_sparsity_pattern();

    if (matrix.get() != 0 && m() == n_rows &&
	n_nonzero_elements() == sparsity_pattern.n_nonzero_elements())
      goto set_matrix_values;

    {
      SparsityPattern trilinos_sparsity;
      trilinos_sparsity.reinit (input_row_map, input_col_map, sparsity_pattern);
      reinit (trilinos_sparsity);
    }

  set_matrix_values:
				// fill the values. the same as above: go
				// through all rows of the matrix, and then
				// all columns. since the sparsity patterns of
				// the input matrix and the specified sparsity
				// pattern might be different, need to go
				// through the row for both these sparsity
				// structures simultaneously in order to
				// really set the correct values.
    const std::size_t * const in_rowstart_indices
      = dealii_sparse_matrix.get_sparsity_pattern().get_rowstart_indices();
    const unsigned int * const in_cols
      = dealii_sparse_matrix.get_sparsity_pattern().get_column_numbers();
    const unsigned int * cols = sparsity_pattern.get_column_numbers();
    const std::size_t * rowstart_indices =
      sparsity_pattern.get_rowstart_indices();

    unsigned int maximum_row_length = matrix->MaxNumEntries();
    std::vector<unsigned int> row_indices (maximum_row_length);
    std::vector<TrilinosScalar> values (maximum_row_length);
    std::size_t in_index, index;

    for (unsigned int row=0; row<n_rows; ++row)
      if (input_row_map.MyGID(row))
	{
	  index = rowstart_indices[row];
	  in_index = in_rowstart_indices[row];
	  unsigned int col = 0;
	  if (sparsity_pattern.optimize_diagonal())
	    {
	      values[col] = dealii_sparse_matrix.global_entry(in_index);
	      row_indices[col++] = row;
	      ++index;
	      ++in_index;
	    }

	  while (in_index < in_rowstart_indices[row+1] &&
		 index < rowstart_indices[row+1])
	    {
	      while (cols[index] < in_cols[in_index] && index < rowstart_indices[row+1])
		++index;
	      while (in_cols[in_index] < cols[index] && in_index < in_rowstart_indices[row+1])
		++in_index;

	      if (std::fabs(dealii_sparse_matrix.global_entry(in_index)) > drop_tolerance)
		{
		  values[col] = dealii_sparse_matrix.global_entry(in_index);
		  row_indices[col++] = in_cols[in_index];
		}
	      ++index;
	      ++in_index;
	    }
	  set (row, col, reinterpret_cast<unsigned int*>(&row_indices[0]),
	       &values[0], false);
	}

    compress();
  }



  void
  SparseMatrix::reinit (const Epetra_CrsMatrix &input_matrix,
			const bool              copy_values)
  {
    Assert (input_matrix.Filled()==true,
	    ExcMessage("Input CrsMatrix has not called FillComplete()!"));

    column_space_map.reset (new Epetra_Map (input_matrix.DomainMap()));

    const Epetra_CrsGraph *graph = &input_matrix.Graph();

    temp_vector.clear ();
    matrix.reset ();
    matrix.reset (new Epetra_FECrsMatrix(Copy, *graph, false));

    matrix->FillComplete (*column_space_map, input_matrix.RangeMap(), true);

    if (copy_values == true)
      {
				// point to the first data entry in the two
				// matrices and copy the content
	const TrilinosScalar * in_values = input_matrix[0];
	TrilinosScalar * values = (*matrix)[0];
	const unsigned int my_nonzeros = input_matrix.NumMyNonzeros();
	std::memcpy (&values[0], &in_values[0],
		     my_nonzeros*sizeof (TrilinosScalar));
      }

    compress();
  }



  void
  SparseMatrix::clear ()
  {
				  // When we clear the matrix, reset
				  // the pointer and generate an
				  // empty matrix.
    column_space_map.reset (new Epetra_Map (0, 0,
					    Utilities::Trilinos::comm_self()));
    temp_vector.clear();
    matrix.reset (new Epetra_FECrsMatrix(View, *column_space_map, 0));

    matrix->FillComplete();

    compressed = true;
  }



  void
  SparseMatrix::clear_row (const unsigned int   row,
			   const TrilinosScalar new_diag_value)
  {
    Assert (matrix->Filled()==true, ExcMatrixNotCompressed());

				  // Only do this on the rows owned
				  // locally on this processor.
    int local_row = matrix->LRID(row);
    if (local_row >= 0)
      {
	TrilinosScalar *values;
	int *col_indices;
	int num_entries;
	const int ierr = matrix->ExtractMyRowView(local_row, num_entries,
						  values, col_indices);

	Assert (ierr == 0,
		ExcTrilinosError(ierr));

	int* diag_find = std::find(col_indices,col_indices+num_entries,
				   local_row);
	int diag_index = (int)(diag_find - col_indices);

	for (int j=0; j<num_entries; ++j)
	  if (diag_index != j || new_diag_value == 0)
	    values[j] = 0.;

	if (diag_find && std::fabs(values[diag_index]) == 0.0 &&
	    new_diag_value != 0.0)
	  values[diag_index] = new_diag_value;
      }
  }



  void
  SparseMatrix::clear_rows (const std::vector<unsigned int> &rows,
			    const TrilinosScalar             new_diag_value)
  {
    compress();
    for (unsigned int row=0; row<rows.size(); ++row)
      clear_row(rows[row], new_diag_value);

				        // This function needs to be called
				        // on all processors. We change some
				        // data, so we need to flush the
				        // buffers to make sure that the
				        // right data is used.
    compress();
  }



  TrilinosScalar
  SparseMatrix::operator() (const unsigned int i,
			    const unsigned int j) const
  {
				      // Extract local indices in
				      // the matrix.
    int trilinos_i = matrix->LRID(i), trilinos_j = matrix->LCID(j);
    TrilinosScalar value = 0.;

				      // If the data is not on the
				      // present processor, we throw
				      // an exception. This is one of
				      // the two tiny differences to
				      // the el(i,j) call, which does
				      // not throw any assertions.
    if (trilinos_i == -1)
      {
	Assert (false, ExcAccessToNonLocalElement(i, j, local_range().first,
						  local_range().second));
      }
    else
      {
				      // Check whether the matrix has
				      // already been transformed to local
				      // indices.
	Assert (matrix->Filled(), ExcMatrixNotCompressed());

				      // Prepare pointers for extraction
				      // of a view of the row.
	int nnz_present = matrix->NumMyEntries(trilinos_i);
	int nnz_extracted;
	int *col_indices;
	TrilinosScalar *values;

				      // Generate the view and make
				      // sure that we have not generated
				      // an error.
	int ierr = matrix->ExtractMyRowView(trilinos_i, nnz_extracted,
					    values, col_indices);
	Assert (ierr==0, ExcTrilinosError(ierr));

	Assert (nnz_present == nnz_extracted,
		ExcDimensionMismatch(nnz_present, nnz_extracted));

				      // Search the index where we
				      // look for the value, and then
				      // finally get it.

	int* el_find = std::find(col_indices, col_indices + nnz_present,
				 trilinos_j);

	int local_col_index = (int)(el_find - col_indices);

				        // This is actually the only
				        // difference to the el(i,j)
				        // function, which means that
				        // we throw an exception in
				        // this case instead of just
				        // returning zero for an
				        // element that is not present
				        // in the sparsity pattern.
	if (local_col_index == nnz_present)
	  {
	    Assert (false, ExcInvalidIndex (i,j));
	  }
	else
	  value = values[local_col_index];
      }

    return value;
  }



  TrilinosScalar
  SparseMatrix::el (const unsigned int i,
		    const unsigned int j) const
  {
				      // Extract local indices in
				      // the matrix.
    int trilinos_i = matrix->LRID(i), trilinos_j = matrix->LCID(j);
    TrilinosScalar value = 0.;

				      // If the data is not on the
				      // present processor, we can't
				      // continue. Just print out zero
				      // as discussed in the
				      // documentation of this
				      // function. if you want error
				      // checking, use operator().
    if ((trilinos_i == -1 ) || (trilinos_j == -1))
      return 0.;
    else
    {
				      // Check whether the matrix
				      // already is transformed to
				      // local indices.
      Assert (matrix->Filled(), ExcMatrixNotCompressed());

				      // Prepare pointers for extraction
				      // of a view of the row.
      int nnz_present = matrix->NumMyEntries(trilinos_i);
      int nnz_extracted;
      int *col_indices;
      TrilinosScalar *values;

				      // Generate the view and make
				      // sure that we have not generated
				      // an error.
      int ierr = matrix->ExtractMyRowView(trilinos_i, nnz_extracted,
					  values, col_indices);
      Assert (ierr==0, ExcTrilinosError(ierr));

      Assert (nnz_present == nnz_extracted,
	      ExcDimensionMismatch(nnz_present, nnz_extracted));

				      // Search the index where we
				      // look for the value, and then
				      // finally get it.
      int* el_find = std::find(col_indices, col_indices + nnz_present,
			       trilinos_j);

      int local_col_index = (int)(el_find - col_indices);


				        // This is actually the only
				        // difference to the () function
				        // querying (i,j), where we throw an
				        // exception instead of just
				        // returning zero for an element
				        // that is not present in the
				        // sparsity pattern.
      if (local_col_index == nnz_present)
	value = 0;
      else
	value = values[local_col_index];
    }

    return value;
  }



  TrilinosScalar
  SparseMatrix::diag_element (const unsigned int i) const
  {
    Assert (m() == n(), ExcNotQuadratic());

				  // Trilinos doesn't seem to have a
				  // more efficient way to access the
				  // diagonal than by just using the
				  // standard el(i,j) function.
    return el(i,i);
  }



  unsigned int
  SparseMatrix::row_length (const unsigned int row) const
  {
    Assert (row < m(), ExcInternalError());

				  // get a representation of the
				  // present row
    int ncols = -1;
    int local_row = matrix->LRID(row);

				  // on the processor who owns this
				  // row, we'll have a non-negative
				  // value.
    if (local_row >= 0)
      {
	int ierr = matrix->NumMyRowEntries (local_row, ncols);
	AssertThrow (ierr == 0, ExcTrilinosError(ierr));
      }

    return ncols;
  }



  namespace internals
  {
    void perform_mmult (const SparseMatrix &inputleft,
			const SparseMatrix &inputright,
			SparseMatrix       &result,
			const VectorBase   &V,
			const bool          transpose_left)
    {
      const bool use_vector = (V.size() == inputright.m() ? true : false);
      if (transpose_left == false)
	{
	  Assert (inputleft.n() == inputright.m(),
		  ExcDimensionMismatch(inputleft.n(), inputright.m()));
	  Assert (inputleft.domain_partitioner().SameAs(inputright.range_partitioner()),
		  ExcMessage ("Parallel partitioning of A and B does not fit."));
	}
      else
	{
	  Assert (inputleft.m() == inputright.m(),
		  ExcDimensionMismatch(inputleft.m(), inputright.m()));
	  Assert (inputleft.range_partitioner().SameAs(inputright.range_partitioner()),
		  ExcMessage ("Parallel partitioning of A and B does not fit."));
	}

      result.clear();

				   // create a suitable operator B: in case
				   // we do not use a vector, all we need to
				   // do is to set the pointer. Otherwise,
				   // we insert the data from B, but
				   // multiply each row with the respective
				   // vector element.
      Teuchos::RCP<Epetra_CrsMatrix> mod_B;
      if (use_vector == false)
	{
	  mod_B = Teuchos::rcp(const_cast<Epetra_CrsMatrix*>
			       (&inputright.trilinos_matrix()),
			       false);
	}
      else
	{
	  mod_B = Teuchos::rcp(new Epetra_CrsMatrix
			       (Copy, inputright.trilinos_sparsity_pattern()),
			       true);
	  mod_B->FillComplete(inputright.domain_partitioner(),
			      inputright.range_partitioner());
	  Assert (inputright.local_range() == V.local_range(),
		  ExcMessage ("Parallel distribution of matrix B and vector V "
			      "does not match."));

	  const int local_N = inputright.local_size();
	  for (int i=0; i<local_N; ++i)
	    {
	      int N_entries = -1;
	      double *new_data, *B_data;
	      mod_B->ExtractMyRowView (i, N_entries, new_data);
	      inputright.trilinos_matrix().ExtractMyRowView (i, N_entries, B_data);
	      double value = V.trilinos_vector()[0][i];
	      for (int j=0; j<N_entries; ++j)
		new_data[j] = value * B_data[j];
	    }
	}

				   // use ML built-in method for performing
				   // the matrix-matrix product.
				   // create ML operators on top of the
				   // Epetra matrices. if we use a
				   // transposed matrix, let ML know it
      ML_Comm* comm;
      ML_Comm_Create(&comm);
#ifdef ML_MPI
      const Epetra_MpiComm *epcomm = dynamic_cast<const Epetra_MpiComm*>(&(inputleft.trilinos_matrix().Comm()));
      // Get the MPI communicator, as it may not be MPI_COMM_W0RLD, and update the ML comm object
      if (epcomm) ML_Comm_Set_UsrComm(comm,epcomm->Comm());
#endif
      ML_Operator *A_ = ML_Operator_Create(comm);
      ML_Operator *B_ = ML_Operator_Create(comm);
      ML_Operator *C_ = ML_Operator_Create(comm);
      SparseMatrix transposed_mat;

      if (transpose_left == false)
	ML_Operator_WrapEpetraCrsMatrix
	  (const_cast<Epetra_CrsMatrix*>(&inputleft.trilinos_matrix()),A_,
	   false);
      else
	{
				// create transposed matrix
	  SparsityPattern sparsity_transposed (inputleft.domain_partitioner(),
					       inputleft.range_partitioner());
	  Assert (inputleft.domain_partitioner().LinearMap() == true,
		  ExcMessage("Matrix must be partitioned contiguously between procs."));
	  for (unsigned int i=0; i<inputleft.local_size(); ++i)
	    {
	      int num_entries, * indices;
	      inputleft.trilinos_sparsity_pattern().ExtractMyRowView(i, num_entries,
								     indices);
	      Assert (num_entries >= 0, ExcInternalError());
	      const unsigned int GID = inputleft.row_partitioner().GID(i);
	      for (int j=0; j<num_entries; ++j)
		sparsity_transposed.add (inputleft.col_partitioner().GID(indices[j]),
					 GID);
	    }

	  sparsity_transposed.compress();
	  transposed_mat.reinit (sparsity_transposed);
	  for (unsigned int i=0; i<inputleft.local_size(); ++i)
	    {
	      int num_entries, * indices;
	      double * values;
	      inputleft.trilinos_matrix().ExtractMyRowView(i, num_entries,
							   values, indices);
	      Assert (num_entries >= 0, ExcInternalError());
	      const unsigned int GID = inputleft.row_partitioner().GID(i);
	      for (int j=0; j<num_entries; ++j)
		transposed_mat.set (inputleft.col_partitioner().GID(indices[j]),
				    GID, values[j]);
	    }
	  transposed_mat.compress();
	  ML_Operator_WrapEpetraCrsMatrix
	    (const_cast<Epetra_CrsMatrix*>(&transposed_mat.trilinos_matrix()),
	     A_,false);
	}
      ML_Operator_WrapEpetraCrsMatrix(mod_B.get(),B_,false);

				   // We implement the multiplication by
				   // hand in a similar way as is done in
				   // ml/src/Operator/ml_rap.c for a triple
				   // matrix product. This means that the
				   // code is very similar to the one found
				   // in ml/src/Operator/ml_rap.c

				   // import data if necessary
      ML_Operator *Btmp, *Ctmp, *Ctmp2, *tptr;
      ML_CommInfoOP *getrow_comm;
      int max_per_proc;
      int N_input_vector = B_->invec_leng;
      getrow_comm = B_->getrow->pre_comm;
      if ( getrow_comm != NULL)
	for (int i = 0; i < getrow_comm->N_neighbors; i++)
	  for (int j = 0; j < getrow_comm->neighbors[i].N_send; j++)
	    AssertThrow (getrow_comm->neighbors[i].send_list[j] < N_input_vector,
			 ExcInternalError());

      ML_create_unique_col_id(N_input_vector, &(B_->getrow->loc_glob_map),
			      getrow_comm, &max_per_proc, B_->comm);
      B_->getrow->use_loc_glob_map = ML_YES;
      if (A_->getrow->pre_comm != NULL)
	ML_exchange_rows( B_, &Btmp, A_->getrow->pre_comm);
      else Btmp = B_;

				   // perform matrix-matrix product
      ML_matmat_mult(A_, Btmp , &Ctmp);

				   // release temporary structures we needed
				   // for multiplication
      ML_free(B_->getrow->loc_glob_map);
      B_->getrow->loc_glob_map = NULL;
      B_->getrow->use_loc_glob_map = ML_NO;
      if (A_->getrow->pre_comm != NULL)
	{
	  tptr = Btmp;
	  while ( (tptr!= NULL) && (tptr->sub_matrix != B_))
	    tptr = tptr->sub_matrix;
	  if (tptr != NULL) tptr->sub_matrix = NULL;
	  ML_RECUR_CSR_MSRdata_Destroy(Btmp);
	  ML_Operator_Destroy(&Btmp);
	}

				   // make correct data structures
      if (A_->getrow->post_comm != NULL)
	ML_exchange_rows(Ctmp, &Ctmp2, A_->getrow->post_comm);
      else
	Ctmp2 = Ctmp;

      ML_back_to_csrlocal(Ctmp2, C_, max_per_proc);

      ML_RECUR_CSR_MSRdata_Destroy (Ctmp);
      ML_Operator_Destroy (&Ctmp);

      if (A_->getrow->post_comm != NULL)
	{
	  ML_RECUR_CSR_MSRdata_Destroy(Ctmp2);
	  ML_Operator_Destroy (&Ctmp2);
	}

				   // create an Epetra matrix from the ML
				   // matrix that we got as a result.
      Epetra_CrsMatrix * C_mat;
      ML_Operator2EpetraCrsMatrix(C_, C_mat);
      C_mat->FillComplete();
      C_mat->OptimizeStorage();
      result.reinit (*C_mat);

				   // destroy allocated memory
      delete C_mat;
      ML_Operator_Destroy (&A_);
      ML_Operator_Destroy (&B_);
      ML_Operator_Destroy (&C_);
      ML_Comm_Destroy (&comm);
    }
  }


  void
  SparseMatrix::mmult (SparseMatrix       &C,
		       const SparseMatrix &B,
		       const VectorBase   &V) const
  {
    internals::perform_mmult (*this, B, C, V, false);
  }



  void
  SparseMatrix::Tmmult (SparseMatrix       &C,
			const SparseMatrix &B,
			const VectorBase   &V) const
  {
    internals::perform_mmult (*this, B, C, V, true);
  }



  void
  SparseMatrix::add (const TrilinosScalar  factor,
		     const SparseMatrix   &rhs)
  {
    Assert (rhs.m() == m(), ExcDimensionMismatch (rhs.m(), m()));
    Assert (rhs.n() == n(), ExcDimensionMismatch (rhs.n(), n()));

    const std::pair<unsigned int, unsigned int>
      local_range = rhs.local_range();

    int ierr;

				   // If both matrices have been transformed
				   // to local index space (in Trilinos
				   // speak: they are filled), we're having
				   // matrices based on the same indices
				   // with the same number of nonzeros
				   // (actually, we'd need sparsity pattern,
				   // but that is too expensive to check),
				   // we can extract views of the column
				   // data on both matrices and simply
				   // manipulate the values that are
				   // addressed by the pointers.
    if (matrix->Filled() == true &&
	rhs.matrix->Filled() == true &&
	this->local_range() == local_range &&
	matrix->NumMyNonzeros() == rhs.matrix->NumMyNonzeros())
      for (unsigned int row=local_range.first;
	   row < local_range.second; ++row)
	{
	  Assert (matrix->NumGlobalEntries(row) ==
		  rhs.matrix->NumGlobalEntries(row),
		  ExcDimensionMismatch(matrix->NumGlobalEntries(row),
				       rhs.matrix->NumGlobalEntries(row)));

	  const int row_local = matrix->RowMap().LID(row);
	  int n_entries, rhs_n_entries;
	  TrilinosScalar *value_ptr, *rhs_value_ptr;

				   // In debug mode, we want to check
				   // whether the indices really are the
				   // same in the calling matrix and the
				   // input matrix. The reason for doing
				   // this only in debug mode is that both
				   // extracting indices and comparing
				   // indices is relatively slow compared to
				   // just working with the values.
#ifdef DEBUG
	  int *index_ptr, *rhs_index_ptr;
	  ierr = rhs.matrix->ExtractMyRowView (row_local, rhs_n_entries,
					       rhs_value_ptr, rhs_index_ptr);
	  Assert (ierr == 0, ExcTrilinosError(ierr));

	  ierr = matrix->ExtractMyRowView (row_local, n_entries, value_ptr,
					   index_ptr);
	  Assert (ierr == 0, ExcTrilinosError(ierr));
#else
	  rhs.matrix->ExtractMyRowView (row_local, rhs_n_entries,rhs_value_ptr);
	  matrix->ExtractMyRowView (row_local, n_entries, value_ptr);
#endif

	  AssertThrow (n_entries == rhs_n_entries,
		       ExcDimensionMismatch (n_entries, rhs_n_entries));

	  for (int i=0; i<n_entries; ++i)
	    {
	      *value_ptr++ += *rhs_value_ptr++ * factor;
#ifdef DEBUG
	      Assert (*index_ptr++ == *rhs_index_ptr++,
		      ExcInternalError());
#endif
	    }
	}
				   // If we have different sparsity patterns
				   // (expressed by a different number of
				   // nonzero elements), we have to be more
				   // careful and extract a copy of the row
				   // data, multiply it by the factor and
				   // then add it to the matrix using the
				   // respective add() function.
    else
      {
	unsigned int max_row_length = 0;
	for (unsigned int row=local_range.first;
	   row < local_range.second; ++row)
	    max_row_length
	      = std::max (max_row_length,
			  static_cast<unsigned int>(rhs.matrix->NumGlobalEntries(row)));

	std::vector<int>            column_indices (max_row_length);
	std::vector<TrilinosScalar> values (max_row_length);

	if (matrix->Filled() == true && rhs.matrix->Filled() == true &&
	    this->local_range() == local_range)
	  for (unsigned int row=local_range.first;
	       row < local_range.second; ++row)
	    {
	      const int row_local = matrix->RowMap().LID(row);
	      int n_entries;

	      ierr = rhs.matrix->ExtractMyRowCopy (row_local, max_row_length,
						   n_entries,
						   &values[0],
						   &column_indices[0]);
	      Assert (ierr == 0, ExcTrilinosError(ierr));

	      for (int i=0; i<n_entries; ++i)
		values[i] *= factor;

	      TrilinosScalar *value_ptr = &values[0];

	      ierr = matrix->SumIntoMyValues (row_local, n_entries, value_ptr,
					      &column_indices[0]);
	      Assert (ierr == 0, ExcTrilinosError(ierr));
	    }
	else
	  {
	    for (unsigned int row=local_range.first;
		 row < local_range.second; ++row)
	      {
		int n_entries;
		ierr = rhs.matrix->Epetra_CrsMatrix::ExtractGlobalRowCopy
		    ((int)row, max_row_length, n_entries, &values[0], &column_indices[0]);
		Assert (ierr == 0, ExcTrilinosError(ierr));

		for (int i=0; i<n_entries; ++i)
		  values[i] *= factor;

		ierr = matrix->Epetra_CrsMatrix::SumIntoGlobalValues
		    ((int)row, n_entries, &values[0], &column_indices[0]);
		Assert (ierr == 0, ExcTrilinosError(ierr));
	      }
	    compress ();

	  }
      }
  }



  void
  SparseMatrix::transpose ()
  {
				  // This only flips a flag that tells
				  // Trilinos that any vmult operation
				  // should be done with the
				  // transpose. However, the matrix
				  // structure is not reset.
    int ierr;

    if (!matrix->UseTranspose())
      {
	ierr = matrix->SetUseTranspose (true);
	AssertThrow (ierr == 0, ExcTrilinosError(ierr));
      }
    else
      {
	ierr = matrix->SetUseTranspose (false);
	AssertThrow (ierr == 0, ExcTrilinosError(ierr));
      }
  }



  void
  SparseMatrix::write_ascii ()
  {
    Assert (false, ExcNotImplemented());
  }



				  // As of now, no particularly neat
				  // ouput is generated in case of
				  // multiple processors.
  void
  SparseMatrix::print (std::ostream &out,
		       const bool    print_detailed_trilinos_information) const
  {
    if (print_detailed_trilinos_information == true)
      out << *matrix;
    else
      {
	double * values;
	int * indices;
	int num_entries;

	for (int i=0; i<matrix->NumMyRows(); ++i)
	  {
	    matrix->ExtractMyRowView (i, num_entries, values, indices);
	    for (int j=0; j<num_entries; ++j)
	      out << "(" << matrix->GRID(i) << "," << matrix->GCID(indices[j]) << ") "
		  << values[j] << std::endl;
	  }
      }

    AssertThrow (out, ExcIO());
  }



  std::size_t
  SparseMatrix::memory_consumption () const
  {
    unsigned int static_memory = sizeof(this) + sizeof (*matrix)
      + sizeof(*matrix->Graph().DataPtr());
    return ((sizeof(TrilinosScalar)+sizeof(int))*matrix->NumMyNonzeros() +
	    sizeof(int)*local_size() +
	    static_memory);
  }




  // explicit instantiations
  //
  template void
  SparseMatrix::reinit (const dealii::SparsityPattern &);
  template void
  SparseMatrix::reinit (const CompressedSparsityPattern &);
  template void
  SparseMatrix::reinit (const CompressedSetSparsityPattern &);
  template void
  SparseMatrix::reinit (const CompressedSimpleSparsityPattern &);

  template void
  SparseMatrix::reinit (const Epetra_Map &,
			const dealii::SparsityPattern &,
			const bool);
  template void
  SparseMatrix::reinit (const Epetra_Map &,
			const CompressedSparsityPattern &,
			const bool);
  template void
  SparseMatrix::reinit (const Epetra_Map &,
			const CompressedSetSparsityPattern &,
			const bool);
  template void
  SparseMatrix::reinit (const Epetra_Map &,
			const CompressedSimpleSparsityPattern &,
			const bool);


  template void
  SparseMatrix::reinit (const Epetra_Map &,
			const Epetra_Map &,
			const dealii::SparsityPattern &,
			const bool);
  template void
  SparseMatrix::reinit (const Epetra_Map &,
			const Epetra_Map &,
			const CompressedSparsityPattern &,
			const bool);
  template void
  SparseMatrix::reinit (const Epetra_Map &,
			const Epetra_Map &,
			const CompressedSimpleSparsityPattern &,
			const bool);
  template void
  SparseMatrix::reinit (const Epetra_Map &,
			const Epetra_Map &,
			const CompressedSetSparsityPattern &,
			const bool);

  template void
  SparseMatrix::reinit (const dealii::SparseMatrix<float> &,
			const double,
			const bool,
			const dealii::SparsityPattern *);
  template void
  SparseMatrix::reinit (const dealii::SparseMatrix<double> &,
			const double,
			const bool,
			const dealii::SparsityPattern *);
  template void
  SparseMatrix::reinit (const dealii::SparseMatrix<long double> &,
			const double,
			const bool,
			const dealii::SparsityPattern *);

  template void
  SparseMatrix::reinit (const Epetra_Map &,
			const dealii::SparseMatrix<float> &,
			const double,
			const bool,
			const dealii::SparsityPattern *);
  template void
  SparseMatrix::reinit (const Epetra_Map &,
			const dealii::SparseMatrix<double> &,
			const double,
			const bool,
			const dealii::SparsityPattern *);
  template void
  SparseMatrix::reinit (const Epetra_Map &,
			const dealii::SparseMatrix<long double> &,
			const double,
			const bool,
			const dealii::SparsityPattern *);

  template void
  SparseMatrix::reinit (const Epetra_Map &,
			const Epetra_Map &,
			const dealii::SparseMatrix<float> &,
			const double,
			const bool,
			const dealii::SparsityPattern *);
  template void
  SparseMatrix::reinit (const Epetra_Map &,
			const Epetra_Map &,
			const dealii::SparseMatrix<double> &,
			const double,
			const bool,
			const dealii::SparsityPattern *);
  template void
  SparseMatrix::reinit (const Epetra_Map &,
			const Epetra_Map &,
			const dealii::SparseMatrix<long double> &,
			const double,
			const bool,
			const dealii::SparsityPattern *);


}

DEAL_II_NAMESPACE_CLOSE

#endif // DEAL_II_USE_TRILINOS
