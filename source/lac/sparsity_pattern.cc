//---------------------------------------------------------------------------
//    $Id: sparsity_pattern.cc 24428 2011-09-26 15:06:23Z kanschat $
//    Version: $Name$
//
//    Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------

#include <deal.II/base/vector_slice.h>
#include <deal.II/base/utilities.h>
#include <deal.II/lac/sparsity_pattern.h>
#include <deal.II/lac/sparsity_tools.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/compressed_sparsity_pattern.h>
#include <deal.II/lac/compressed_set_sparsity_pattern.h>
#include <deal.II/lac/compressed_simple_sparsity_pattern.h>

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <functional>

DEAL_II_NAMESPACE_OPEN


const unsigned int SparsityPattern::invalid_entry;



SparsityPattern::SparsityPattern ()
                :
		max_dim(0),
		max_vec_len(0),
		rowstart(0),
		colnums(0),
		compressed(false),
		diagonal_optimized(false)
{
  reinit (0,0,0);
}



SparsityPattern::SparsityPattern (const SparsityPattern &s)
                :
		Subscriptor(),
		max_dim(0),
		max_vec_len(0),
		rowstart(0),
		colnums(0),
		compressed(false),
		diagonal_optimized(false)
{
  Assert (s.rowstart == 0, ExcInvalidConstructorCall());
  Assert (s.colnums == 0, ExcInvalidConstructorCall());
  Assert (s.rows == 0, ExcInvalidConstructorCall());
  Assert (s.cols == 0, ExcInvalidConstructorCall());

  reinit (0,0,0, false);
}



SparsityPattern::SparsityPattern (const unsigned int m,
				  const unsigned int n,
				  const unsigned int max_per_row,
				  const bool optimize_diag)
		:
                max_dim(0),
                max_vec_len(0),
                rowstart(0),
                colnums(0),
                compressed(false),
                diagonal_optimized(false)
{
  reinit (m,n,max_per_row, optimize_diag);
}



SparsityPattern::SparsityPattern (
  const unsigned int m,
  const unsigned int n,
  const std::vector<unsigned int>& row_lengths,
  const bool optimize_diag)
		:
                max_dim(0),
                max_vec_len(0),
                rowstart(0),
                colnums(0)
{
  reinit (m, n, row_lengths, optimize_diag);
}



SparsityPattern::SparsityPattern (const unsigned int n,
				  const unsigned int max_per_row)
		:
                max_dim(0),
                max_vec_len(0),
                rowstart(0),
                colnums(0)
{
  reinit (n, n, max_per_row, true);
}



SparsityPattern::SparsityPattern (
  const unsigned int               m,
  const std::vector<unsigned int>& row_lengths,
  const bool optimize_diag)
		:
                max_dim(0),
                max_vec_len(0),
                rowstart(0),
                colnums(0)
{
  reinit (m, m, row_lengths, optimize_diag);
}



SparsityPattern::SparsityPattern (const SparsityPattern &original,
				  const unsigned int     max_per_row,
				  const unsigned int     extra_off_diagonals)
		:
                max_dim(0),
                max_vec_len(0),
                rowstart(0),
                colnums(0)
{
  Assert (original.rows==original.cols, ExcNotQuadratic());
  Assert (original.optimize_diagonal(), ExcNotQuadratic());
  Assert (original.is_compressed(), ExcNotCompressed());

  reinit (original.rows, original.cols, max_per_row,
	  original.optimize_diagonal());

				   // now copy the entries from
				   // the other object
  for (unsigned int row=0; row<original.rows; ++row)
    {
				       // copy the elements of this row
				       // of the other object
				       //
				       // note that the first object actually
				       // is the main-diagonal element,
				       // which we need not copy
				       //
				       // we do the copying in two steps:
				       // first we note that the elements in
				       // @p{original} are sorted, so we may
				       // first copy all the elements up to
				       // the first side-diagonal one which
				       // is to be filled in. then we insert
				       // the side-diagonals, finally copy
				       // the rest from that element onwards
				       // which is not a side-diagonal any
				       // more.
      const unsigned int * const
	original_row_start = &original.colnums[original.rowstart[row]] + 1;
				       // the following requires that
				       // @p{original} be compressed since
				       // otherwise there might be invalid_entry's
      const unsigned int * const
	original_row_end   = &original.colnums[original.rowstart[row+1]];

				       // find pointers before and
				       // after extra
				       // off-diagonals. if at top or
				       // bottom of matrix, then set
				       // these pointers such that no
				       // copying is necessary (see
				       // the @p{copy} commands)
      const unsigned int * const
	original_last_before_side_diagonals
	= (row > extra_off_diagonals ?
	   Utilities::lower_bound (original_row_start,
			     original_row_end,
			     row-extra_off_diagonals) :
	   original_row_start);

      const unsigned int * const
	original_first_after_side_diagonals
	= (row < rows-extra_off_diagonals-1 ?
	   std::upper_bound (original_row_start,
			     original_row_end,
			     row+extra_off_diagonals) :
	   original_row_end);

				       // find first free slot. the
				       // first slot in each row is
				       // the diagonal element
      unsigned int * next_free_slot = &colnums[rowstart[row]] + 1;

				       // copy elements before side-diagonals
      next_free_slot = std::copy (original_row_start,
				  original_last_before_side_diagonals,
				  next_free_slot);

				       // insert left and right side-diagonals
      for (unsigned int i=1; i<=std::min(row,extra_off_diagonals);
	   ++i, ++next_free_slot)
	*next_free_slot = row-i;
      for (unsigned int i=1; i<=std::min(extra_off_diagonals, rows-row-1);
	   ++i, ++next_free_slot)
	*next_free_slot = row+i;

				       // copy rest
      next_free_slot = std::copy (original_first_after_side_diagonals,
				  original_row_end,
				  next_free_slot);

				       // this error may happen if the
				       // sum of previous elements per row
				       // and those of the new diagonals
				       // exceeds the maximum number of
				       // elements per row given to this
				       // constructor
      Assert (next_free_slot <= &colnums[rowstart[row+1]],
	      ExcNotEnoughSpace (0,rowstart[row+1]-rowstart[row]));
    };
}



SparsityPattern::~SparsityPattern ()
{
  if (rowstart != 0)  delete[] rowstart;
  if (colnums != 0)   delete[] colnums;
}



SparsityPattern &
SparsityPattern::operator = (const SparsityPattern &s)
{
  Assert (s.rowstart == 0, ExcInvalidConstructorCall());
  Assert (s.colnums == 0, ExcInvalidConstructorCall());
  Assert (s.rows == 0, ExcInvalidConstructorCall());
  Assert (s.cols == 0, ExcInvalidConstructorCall());

  Assert (rowstart == 0, ExcInvalidConstructorCall());
  Assert (colnums == 0, ExcInvalidConstructorCall());
  Assert (rows == 0, ExcInvalidConstructorCall());
  Assert (cols == 0, ExcInvalidConstructorCall());

  return *this;
}



void
SparsityPattern::reinit (const unsigned int m,
			 const unsigned int n,
			 const unsigned int max_per_row,
			 const bool optimize_diag)
{
				   // simply map this function to the
				   // other @p{reinit} function
  const std::vector<unsigned int> row_lengths (m, max_per_row);
  reinit (m, n, row_lengths, optimize_diag);
}



void
SparsityPattern::reinit (
  const unsigned int m,
  const unsigned int n,
  const VectorSlice<const std::vector<unsigned int> >&row_lengths,
  const bool optimize_diag)
{
  AssertDimension (row_lengths.size(), m);

  rows = m;
  cols = n;

				   // delete empty matrices
  if ((m==0) || (n==0))
    {
      if (rowstart)  delete[] rowstart;
      if (colnums)   delete[] colnums;
      rowstart = 0;
      colnums = 0;
      max_vec_len = max_dim = rows = cols = 0;
				       // if dimension is zero: ignore
				       // max_per_row
      max_row_length = 0;
      compressed = false;
      return;
    }

				   // first, if the matrix is
				   // quadratic and special treatment
				   // of diagonals is requested, we
				   // will have to make sure that each
				   // row has at least one entry for
				   // the diagonal element. make this
				   // more obvious by having a
				   // variable which we can query
  diagonal_optimized = (m == n ? true : false) && optimize_diag;

  				   // find out how many entries we
				   // need in the @p{colnums} array. if
				   // this number is larger than
				   // @p{max_vec_len}, then we will need
				   // to reallocate memory
				   //
				   // note that the number of elements
				   // per row is bounded by the number
				   // of columns
  std::size_t vec_len = 0;
  for (unsigned int i=0; i<m; ++i)
    vec_len += std::min((diagonal_optimized ?
			 std::max(row_lengths[i], 1U) :
			 row_lengths[i]),
			n);

				   // sometimes, no entries are
				   // requested in the matrix (this
				   // most often happens when blocks
				   // in a block matrix are simply
				   // zero). in that case, allocate
				   // exactly one element, to have a
				   // valid pointer to some memory
  if (vec_len == 0)
    {
      vec_len = 1;
      if (colnums)
	{
	  delete[] colnums;
	  colnums = 0;
	}

      max_vec_len = vec_len;
      colnums = new unsigned int[max_vec_len];
    }

  max_row_length = (row_lengths.size() == 0 ?
		    0 :
		    std::min (*std::max_element(row_lengths.begin(),
						row_lengths.end()),
			      n));

  if (diagonal_optimized && (max_row_length==0) && (m!=0))
    max_row_length = 1;

				   // allocate memory for the rowstart
				   // values, if necessary. even
				   // though we re-set the pointers
				   // again immediately after deleting
				   // their old content, set them to
				   // zero in between because the
				   // allocation might fail, in which
				   // case we get an exception and the
				   // destructor of this object will
				   // be called -- where we look at
				   // the non-nullness of the (now
				   // invalid) pointer again and try
				   // to delete the memory a second
				   // time.
  if (rows > max_dim)
    {
      if (rowstart)
	{
	  delete[] rowstart;
	  rowstart = 0;
	}

      max_dim = rows;
      rowstart = new std::size_t[max_dim+1];
    }

				   // allocate memory for the column
				   // numbers if necessary
  if (vec_len > max_vec_len)
    {
      if (colnums)
	{
	  delete[] colnums;
	  colnums = 0;
	}

      max_vec_len = vec_len;
      colnums = new unsigned int[max_vec_len];
    }

				   // set the rowstart array
  rowstart[0] = 0;
  for (unsigned int i=1; i<=rows; ++i)
    rowstart[i] = rowstart[i-1] +
		  (diagonal_optimized ?
		   std::max(std::min(row_lengths[i-1],n),1U) :
		   std::min(row_lengths[i-1],n));
  Assert ((rowstart[rows]==vec_len)
	  ||
	  ((vec_len == 1) && (rowstart[rows] == 0)),
	  ExcInternalError());

				   // preset the column numbers by a
				   // value indicating it is not in
				   // use
  std::fill_n (&colnums[0], vec_len, invalid_entry);

				   // if diagonal elements are
				   // special: let the first entry in
				   // each row be the diagonal value
  if (diagonal_optimized)
    for (unsigned int i=0;i<rows;i++)
      colnums[rowstart[i]] = i;

  compressed = false;
}



void
SparsityPattern::compress ()
{
  Assert ((rowstart!=0) && (colnums!=0), ExcEmptyObject());

				   // do nothing if already compressed
  if (compressed)
    return;

  unsigned int next_free_entry = 0,
	       next_row_start  = 0,
	       row_length      = 0;

				   // first find out how many non-zero
				   // elements there are, in order to
				   // allocate the right amount of
				   // memory
  const std::size_t nonzero_elements
    = std::count_if (&colnums[rowstart[0]],
		     &colnums[rowstart[rows]],
		     std::bind2nd(std::not_equal_to<unsigned int>(), invalid_entry));
				   // now allocate the respective memory
  unsigned int *new_colnums = new unsigned int[nonzero_elements];


				   // reserve temporary storage to
				   // store the entries of one row
  std::vector<unsigned int> tmp_entries (max_row_length);

				   // Traverse all rows
  for (unsigned int line=0; line<rows; ++line)
    {
				       // copy used entries, break if
				       // first unused entry is reached
      row_length = 0;
      for (unsigned int j=rowstart[line]; j<rowstart[line+1]; ++j,++row_length)
	if (colnums[j] != invalid_entry)
	  tmp_entries[row_length] = colnums[j];
	else
	  break;
				       // now @p{rowstart} is
				       // the number of entries in
				       // this line

				       // Sort only beginning at the
				       // second entry, if optimized
				       // storage of diagonal entries
				       // is on.

				       // if this line is empty or has
				       // only one entry, don't sort
      if (row_length > 1)
	std::sort ((diagonal_optimized)
		   ? tmp_entries.begin()+1
		   : tmp_entries.begin(),
		   tmp_entries.begin()+row_length);

				       // insert column numbers
				       // into the new field
      for (unsigned int j=0; j<row_length; ++j)
	new_colnums[next_free_entry++] = tmp_entries[j];

				       // note new start of this and
				       // the next row
      rowstart[line] = next_row_start;
      next_row_start = next_free_entry;

				       // some internal checks: either
				       // the matrix is not quadratic,
				       // or if it is, then the first
				       // element of this row must be
				       // the diagonal element
				       // (i.e. with column
				       // index==line number)
      Assert ((!diagonal_optimized) ||
	      (new_colnums[rowstart[line]] == line),
	      ExcInternalError());
				       // assert that the first entry
				       // does not show up in
				       // the remaining ones and that
				       // the remaining ones are unique
				       // among themselves (this handles
				       // both cases, quadratic and
				       // rectangular matrices)
				       //
				       // the only exception here is
				       // if the row contains no
				       // entries at all
      Assert ((rowstart[line] == next_row_start)
	      ||
	      (std::find (&new_colnums[rowstart[line]+1],
			  &new_colnums[next_row_start],
			  new_colnums[rowstart[line]]) ==
	       &new_colnums[next_row_start]),
	      ExcInternalError());
      Assert ((rowstart[line] == next_row_start)
	      ||
	      (std::adjacent_find(&new_colnums[rowstart[line]+1],
				  &new_colnums[next_row_start]) ==
	       &new_colnums[next_row_start]),
	      ExcInternalError());
    };

				   // assert that we have used all
				   // allocated space, no more and no
				   // less
  Assert (next_free_entry == nonzero_elements,
	  ExcInternalError());

				   // set iterator-past-the-end
  rowstart[rows] = next_row_start;

				   // set colnums to the newly
				   // allocated array and delete the
				   // old one
  delete[] colnums;
  colnums = new_colnums;

				   // store the size
  max_vec_len = nonzero_elements;

  compressed = true;
}



template <typename CSP>
void
SparsityPattern::copy_from (const CSP &csp,
			    const bool optimize_diag)
{
				   // first determine row lengths for
				   // each row. if the matrix is
				   // quadratic, then we might have to
				   // add an additional entry for the
				   // diagonal, if that is not yet
				   // present. as we have to call
				   // compress anyway later on, don't
				   // bother to check whether that
				   // diagonal entry is in a certain
				   // row or not
  const bool do_diag_optimize = optimize_diag && (csp.n_rows() == csp.n_cols());
  std::vector<unsigned int> row_lengths (csp.n_rows());
  for (unsigned int i=0; i<csp.n_rows(); ++i)
    {
      row_lengths[i] = csp.row_length(i);
      if (do_diag_optimize && !csp.exists(i,i))
	++row_lengths[i];
    }
  reinit (csp.n_rows(), csp.n_cols(), row_lengths, do_diag_optimize);

				   // now enter all the elements into
				   // the matrix, if there are
				   // any. note that if the matrix is
				   // quadratic, then we already have
				   // the diagonal element
				   // preallocated
  if (n_rows() != 0 && n_cols() != 0)
    for (unsigned int row = 0; row<csp.n_rows(); ++row)
      {
	unsigned int *cols = &colnums[rowstart[row]] + (do_diag_optimize ? 1 : 0);
	typename CSP::row_iterator col_num = csp.row_begin (row),
				   end_row = csp.row_end (row);

	for (; col_num != end_row; ++col_num)
	  {
	    const unsigned int col = *col_num;
	    if ((col!=row) || !do_diag_optimize)
	      *cols++ = col;
	  }
      }

				   // do not need to compress the sparsity
				   // pattern since we already have
				   // allocated the right amount of data,
				   // and the CSP data is sorted, too.
  compressed = true;
}



template <typename number>
void SparsityPattern::copy_from (const FullMatrix<number> &matrix,
				 const bool optimize_diag)
{
				   // first init with the number of entries
				   // per row. if optimize_diag is set then we
				   // also have to allocate memory for the
				   // diagonal entry, unless we have already
				   // counted it or the matrix isn't square
  std::vector<unsigned int> entries_per_row (matrix.m(), 0);
  for (unsigned int row=0; row<matrix.m(); ++row)
    {
      for (unsigned int col=0; col<matrix.n(); ++col)
	if (matrix(row,col) != 0)
	  ++entries_per_row[row];
      if ((optimize_diag == true)
	  &&
	  (matrix(row,row) == 0)
	  &&
	  (matrix.m() == matrix.n()))
	++entries_per_row[row];
    }

  reinit (matrix.m(), matrix.n(), entries_per_row, optimize_diag);

				   // now set entries
  for (unsigned int row=0; row<matrix.m(); ++row)
    for (unsigned int col=0; col<matrix.n(); ++col)
      if (matrix(row,col) != 0)
	add (row,col);

				   // finally compress
  compress ();
}


void
SparsityPattern::reinit (
  const unsigned int m,
  const unsigned int n,
  const std::vector<unsigned int>& row_lengths,
  const bool optimize_diag)
{
  reinit(m, n, make_slice(row_lengths), optimize_diag);
}



bool
SparsityPattern::empty () const
{
				   // let's try to be on the safe side of
				   // life by using multiple possibilities in
				   // the check for emptiness... (sorry for
				   // this kludge -- emptying matrices and
				   // freeing memory was not present in the
				   // original implementation and I don't
				   // know at how many places I missed
				   // something in adding it, so I try to
				   // be cautious. wb)
  if ((rowstart==0) || (rows==0) || (cols==0))
    {
      Assert (rowstart==0, ExcInternalError());
      Assert (rows==0, ExcInternalError());
      Assert (cols==0, ExcInternalError());
      Assert (colnums==0, ExcInternalError());
      Assert (max_vec_len==0, ExcInternalError());

      return true;
    };
  return false;
}



unsigned int
SparsityPattern::max_entries_per_row () const
{
				   // if compress() has not yet been
				   // called, we can get the maximum
				   // number of elements per row using
				   // the stored value
  if (!compressed)
    return max_row_length;

				   // if compress() was called, we
				   // use a better algorithm which
				   // gives us a sharp bound
  unsigned int m = 0;
  for (unsigned int i=1; i<rows; ++i)
    m = std::max (m, static_cast<unsigned int>(rowstart[i]-rowstart[i-1]));

  return m;
}



unsigned int
SparsityPattern::operator () (const unsigned int i,
			      const unsigned int j) const
{
  Assert ((rowstart!=0) && (colnums!=0), ExcEmptyObject());
  Assert (i<rows, ExcIndexRange(i,0,rows));
  Assert (j<cols, ExcIndexRange(j,0,cols));
  Assert (compressed, ExcNotCompressed());

				   // let's see whether there is
				   // something in this line
  if (rowstart[i] == rowstart[i+1])
    return invalid_entry;

				   // If special storage of diagonals
				   // was requested, we can get the
				   // diagonal element faster by this
				   // query.
  if (diagonal_optimized && (i==j))
    return rowstart[i];

				   // all other entries are sorted, so
				   // we can use a binary search
				   // algorithm
				   //
				   // note that the entries are only
				   // sorted upon compression, so this
				   // would fail for non-compressed
				   // sparsity patterns; however, that
				   // is why the Assertion is at the
				   // top of this function, so it may
				   // not be called for noncompressed
				   // structures.
  const unsigned int * sorted_region_start = (diagonal_optimized ?
					      &colnums[rowstart[i]+1] :
					      &colnums[rowstart[i]]);
  const unsigned int * const p
    = Utilities::lower_bound<const unsigned int *> (sorted_region_start,
					&colnums[rowstart[i+1]],
					j);
  if ((p != &colnums[rowstart[i+1]])  &&  (*p == j))
    return (p - &colnums[0]);
  else
    return invalid_entry;
}



void
SparsityPattern::add (const unsigned int i,
		      const unsigned int j)
{
  Assert ((rowstart!=0) && (colnums!=0), ExcEmptyObject());
  Assert (i<rows, ExcIndexRange(i,0,rows));
  Assert (j<cols, ExcIndexRange(j,0,cols));
  Assert (compressed==false, ExcMatrixIsCompressed());

  for (unsigned int k=rowstart[i]; k<rowstart[i+1]; k++)
    {
				       // entry already exists
      if (colnums[k] == j) return;
				       // empty entry found, put new
				       // entry here
      if (colnums[k] == invalid_entry)
	{
	  colnums[k] = j;
	  return;
	};
    };

				   // if we came thus far, something went
				   // wrong: there was not enough space
				   // in this line
  Assert (false, ExcNotEnoughSpace(i, rowstart[i+1]-rowstart[i]));
}



template <typename ForwardIterator>
void
SparsityPattern::add_entries (const unsigned int row,
			      ForwardIterator    begin,
			      ForwardIterator    end,
			      const bool         /*indices_are_sorted*/)
{
  // right now, just forward to the other function.
  for (ForwardIterator it = begin; it != end; ++it)
    add (row, *it);
}


bool
SparsityPattern::exists (const unsigned int i, const unsigned int j) const
{
  Assert ((rowstart!=0) && (colnums!=0), ExcEmptyObject());
  Assert (i<rows, ExcIndexRange(i,0,rows));
  Assert (j<cols, ExcIndexRange(j,0,cols));

  for (unsigned int k=rowstart[i]; k<rowstart[i+1]; k++)
    {
				       // entry already exists
      if (colnums[k] == j) return true;
    }
  return false;
}



unsigned int
SparsityPattern::row_position (const unsigned int i, const unsigned int j) const
{
  Assert ((rowstart!=0) && (colnums!=0), ExcEmptyObject());
  Assert (i<rows, ExcIndexRange(i,0,rows));
  Assert (j<cols, ExcIndexRange(j,0,cols));

  for (unsigned int k=rowstart[i]; k<rowstart[i+1]; k++)
    {
				       // entry exists
      if (colnums[k] == j) return k-rowstart[i];
    }
  return numbers::invalid_unsigned_int;
}



std::pair<unsigned int, unsigned int>
SparsityPattern::matrix_position (const unsigned int global_index) const
{
  Assert (compressed == true, ExcNotCompressed());
  Assert (global_index < n_nonzero_elements(),
	  ExcIndexRange (global_index, 0, n_nonzero_elements()));

				   // first find the row in which the
				   // entry is located. for this note
				   // that the rowstart array indexes
				   // the global indices at which each
				   // row starts. since it is sorted,
				   // and since there is an element
				   // for the one-past-last row, we
				   // can simply use a bisection
				   // search on it
  const unsigned int row
    = (std::upper_bound (&rowstart[0], &rowstart[rows], global_index)
       - &rowstart[0] - 1);

				   // now, the column index is simple
				   // since that is what the colnums
				   // array stores:
  const unsigned int col = colnums[global_index];

				   // so return the respective pair
  return std::make_pair (row,col);
}



void
SparsityPattern::symmetrize ()
{
  Assert ((rowstart!=0) && (colnums!=0), ExcEmptyObject());
  Assert (compressed==false, ExcMatrixIsCompressed());
				   // Note that we only require a
				   // quadratic matrix here, no
				   // special treatment of diagonals
  Assert (rows==cols, ExcNotQuadratic());

				   // loop over all elements presently
				   // in the sparsity pattern and add
				   // the transpose element. note:
				   //
				   // 1. that the sparsity pattern
				   // changes which we work on, but
				   // not the present row
				   //
				   // 2. that the @p{add} function can
				   // be called on elements that
				   // already exist without any harm
  for (unsigned int row=0; row<rows; ++row)
    for (unsigned int k=rowstart[row]; k<rowstart[row+1]; k++)
      {
					 // check whether we are at
					 // the end of the entries of
					 // this row. if so, go to
					 // next row
	if (colnums[k] == invalid_entry)
	  break;

					 // otherwise add the
					 // transpose entry if this is
					 // not the diagonal (that
					 // would not harm, only take
					 // time to check up)
	if (colnums[k] != row)
	  add (colnums[k], row);
      };
}



void
SparsityPattern::print (std::ostream &out) const
{
  Assert ((rowstart!=0) && (colnums!=0), ExcEmptyObject());

  AssertThrow (out, ExcIO());

  for (unsigned int i=0; i<rows; ++i)
    {
      out << '[' << i;
      for (unsigned int j=rowstart[i]; j<rowstart[i+1]; ++j)
	if (colnums[j] != invalid_entry)
	  out << ',' << colnums[j];
      out << ']' << std::endl;
    }

  AssertThrow (out, ExcIO());
}



void
SparsityPattern::print_gnuplot (std::ostream &out) const
{
  Assert ((rowstart!=0) && (colnums!=0), ExcEmptyObject());

  AssertThrow (out, ExcIO());

  for (unsigned int i=0; i<rows; ++i)
    for (unsigned int j=rowstart[i]; j<rowstart[i+1]; ++j)
      if (colnums[j] != invalid_entry)
					 // while matrix entries are
					 // usually written (i,j),
					 // with i vertical and j
					 // horizontal, gnuplot output
					 // is x-y, that is we have to
					 // exchange the order of
					 // output
	out << colnums[j] << " " << -static_cast<signed int>(i) << std::endl;

  AssertThrow (out, ExcIO());
}



unsigned int
SparsityPattern::bandwidth () const
{
  Assert ((rowstart!=0) && (colnums!=0), ExcEmptyObject());
  unsigned int b=0;
  for (unsigned int i=0; i<rows; ++i)
    for (unsigned int j=rowstart[i]; j<rowstart[i+1]; ++j)
      if (colnums[j] != invalid_entry)
	{
	  if (static_cast<unsigned int>(std::abs(static_cast<int>(i-colnums[j]))) > b)
	    b = std::abs(static_cast<signed int>(i-colnums[j]));
	}
      else
					 // leave if at the end of
					 // the entries of this line
	break;
  return b;
}


void
SparsityPattern::block_write (std::ostream &out) const
{
  AssertThrow (out, ExcIO());

                                   // first the simple objects,
                                   // bracketed in [...]
  out << '[' << max_dim << ' '
      << rows << ' '
      << cols << ' '
      << max_vec_len << ' '
      << max_row_length << ' '
      << compressed << ' '
      << diagonal_optimized << "][";
                                   // then write out real data
  out.write (reinterpret_cast<const char*>(&rowstart[0]),
	     reinterpret_cast<const char*>(&rowstart[max_dim+1])
	     - reinterpret_cast<const char*>(&rowstart[0]));
  out << "][";
  out.write (reinterpret_cast<const char*>(&colnums[0]),
	     reinterpret_cast<const char*>(&colnums[max_vec_len])
	     - reinterpret_cast<const char*>(&colnums[0]));
  out << ']';

  AssertThrow (out, ExcIO());
}



void
SparsityPattern::block_read (std::istream &in)
{
  AssertThrow (in, ExcIO());

  char c;

                                   // first read in simple data
  in >> c;
  AssertThrow (c == '[', ExcIO());
  in >> max_dim
     >> rows
     >> cols
     >> max_vec_len
     >> max_row_length
     >> compressed
     >> diagonal_optimized;

  in >> c;
  AssertThrow (c == ']', ExcIO());
  in >> c;
  AssertThrow (c == '[', ExcIO());

                                   // reallocate space
  if (rowstart)
    delete[] rowstart;
  if (colnums)
    delete[] colnums;

  rowstart = new std::size_t[max_dim+1];
  colnums  = new unsigned int[max_vec_len];

                                   // then read data
  in.read (reinterpret_cast<char*>(&rowstart[0]),
           reinterpret_cast<char*>(&rowstart[max_dim+1])
           - reinterpret_cast<char*>(&rowstart[0]));
  in >> c;
  AssertThrow (c == ']', ExcIO());
  in >> c;
  AssertThrow (c == '[', ExcIO());
  in.read (reinterpret_cast<char*>(&colnums[0]),
           reinterpret_cast<char*>(&colnums[max_vec_len])
           - reinterpret_cast<char*>(&colnums[0]));
  in >> c;
  AssertThrow (c == ']', ExcIO());
}



std::size_t
SparsityPattern::memory_consumption () const
{
  return (max_dim * sizeof(unsigned int) +
	  sizeof(*this) +
	  max_vec_len * sizeof(unsigned int));
}



void
SparsityPattern::
partition (const unsigned int         n_partitions,
           std::vector<unsigned int> &partition_indices) const
{
  SparsityTools::partition (*this, n_partitions, partition_indices);
}


// explicit instantiations
template void SparsityPattern::copy_from<CompressedSparsityPattern> (const CompressedSparsityPattern &, bool);
template void SparsityPattern::copy_from<CompressedSetSparsityPattern> (const CompressedSetSparsityPattern &, bool);
template void SparsityPattern::copy_from<CompressedSimpleSparsityPattern> (const CompressedSimpleSparsityPattern &, bool);
template void SparsityPattern::copy_from<float> (const FullMatrix<float> &, bool);
template void SparsityPattern::copy_from<double> (const FullMatrix<double> &, bool);

template void SparsityPattern::add_entries<const unsigned int*> (const unsigned int,
								 const unsigned int*,
								 const unsigned int*,
								 const bool);
#ifndef DEAL_II_VECTOR_ITERATOR_IS_POINTER
template void SparsityPattern::add_entries<std::vector<unsigned int>::const_iterator>
(const unsigned int,
 std::vector<unsigned int>::const_iterator,
 std::vector<unsigned int>::const_iterator,
 const bool);
#endif
template void SparsityPattern::add_entries<std::vector<unsigned int>::iterator>
(const unsigned int,
 std::vector<unsigned int>::iterator,
 std::vector<unsigned int>::iterator,
 const bool);

DEAL_II_NAMESPACE_CLOSE
