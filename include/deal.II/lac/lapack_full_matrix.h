//---------------------------------------------------------------------------
//    $Id: lapack_full_matrix.h 23876 2011-06-28 18:21:51Z kanschat $
//
//    Copyright (C) 2005, 2006, 2008, 2009, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__lapack_full_matrix_h
#define __deal2__lapack_full_matrix_h


#include <deal.II/base/config.h>
#include <deal.II/base/smartpointer.h>
#include <deal.II/base/table.h>
#include <deal.II/lac/lapack_support.h>
#include <deal.II/lac/vector_memory.h>

#include <deal.II/base/std_cxx1x/shared_ptr.h>
#include <vector>
#include <complex>

DEAL_II_NAMESPACE_OPEN

// forward declarations
template<typename number> class Vector;
template<typename number> class BlockVector;
template<typename number> class FullMatrix;


/**
 * A variant of FullMatrix using LAPACK functions wherever
 * possible. In order to do this, the matrix is stored in transposed
 * order. The element access functions hide this fact by reverting the
 * transposition.
 *
 * @note In order to perform LAPACK functions, the class contains a lot of
 * auxiliary data in the private section. The names of these data
 * vectors are usually the names chosen for the arguments in the
 * LAPACK documentation.
 *
 * @ingroup Matrix1
 * @author Guido Kanschat, 2005
 */
template <typename number>
class LAPACKFullMatrix : public TransposeTable<number>
{
  public:
				     /**
				      * Constructor. Initialize the
				      * matrix as a square matrix with
				      * dimension <tt>n</tt>.
				      *
				      * In order to avoid the implicit
				      * conversion of integers and
				      * other types to a matrix, this
				      * constructor is declared
				      * <tt>explicit</tt>.
				      *
				      * By default, no memory is
				      * allocated.
				      */
    explicit LAPACKFullMatrix (const unsigned int n = 0);

				     /**
				      * Constructor. Initialize the
				      * matrix as a rectangular
				      * matrix.
				      */
    LAPACKFullMatrix (const unsigned int rows,
		      const unsigned int cols);

				     /**
				      * Copy constructor. This
				      * constructor does a deep copy
				      * of the matrix. Therefore, it
				      * poses a possible efficiency
				      * problem, if for example,
				      * function arguments are passed
				      * by value rather than by
				      * reference. Unfortunately, we
				      * can't mark this copy
				      * constructor <tt>explicit</tt>,
				      * since that prevents the use of
				      * this class in containers, such
				      * as <tt>std::vector</tt>. The
				      * responsibility to check
				      * performance of programs must
				      * therefore remain with the
				      * user of this class.
				      */
    LAPACKFullMatrix (const LAPACKFullMatrix&);

				     /**
				      * Assignment operator.
				      */
    LAPACKFullMatrix<number> &
    operator = (const LAPACKFullMatrix<number>&);

				     /**
				      * Assignment operator for a
				      * regular FullMatrix.
				      */
    template <typename number2>
    LAPACKFullMatrix<number> &
    operator = (const FullMatrix<number2>&);

				     /**
				      * This operator assigns a scalar
				      * to a matrix. To avoid
				      * confusion with constructors,
				      * zero is the only value allowed
				      * for <tt>d</tt>
				      */
    LAPACKFullMatrix<number> &
    operator = (const double d);

                                     /**
				      * Assignment from different
				      * matrix classes. This
				      * assignment operator uses
				      * iterators of the class
				      * MATRIX. Therefore, sparse
				      * matrices are possible sources.
				      */
    template <class MATRIX>
    void copy_from (const MATRIX&);

				     /**
				      * Fill rectangular block.
				      *
				      * A rectangular block of the
				      * matrix <tt>src</tt> is copied into
				      * <tt>this</tt>. The upper left
				      * corner of the block being
				      * copied is
				      * <tt>(src_offset_i,src_offset_j)</tt>.
				      * The upper left corner of the
				      * copied block is
				      * <tt>(dst_offset_i,dst_offset_j)</tt>.
				      * The size of the rectangular
				      * block being copied is the
				      * maximum size possible,
				      * determined either by the size
				      * of <tt>this</tt> or <tt>src</tt>.
				      *
				      * The final two arguments allow
				      * to enter a multiple of the
				      * source or its transpose.
				      */
    template<class MATRIX>
    void fill (const MATRIX &src,
	       const unsigned int dst_offset_i = 0,
	       const unsigned int dst_offset_j = 0,
	       const unsigned int src_offset_i = 0,
	       const unsigned int src_offset_j = 0,
	       const number factor = 1.,
	       const bool transpose = false);


				     /**
				      * Matrix-vector-multiplication.
				      *
				      * The optional parameter
				      * <tt>adding</tt> determines, whether the
				      * result is stored in <tt>w</tt> or added
				      * to <tt>w</tt>.
				      *
				      * if (adding)
				      *  <i>w += A*v</i>
				      *
				      * if (!adding)
				      *  <i>w = A*v</i>
                                      *
                                      * Source and destination must
                                      * not be the same vector.
				      *
				      * @note This template only
				      * exists for compile-time
				      * compatibility with
				      * FullMatrix. Implementation is
				      * only available for <tt>VECTOR=Vector&lt;number&gt;</tt>
				      */
    template <class VECTOR>
    void vmult(VECTOR& dst, const VECTOR& src, const bool adding = false) const;
				     /**
				      * Adding Matrix-vector-multiplication.
				      *  <i>w += A*v</i>
                                      *
                                      * Source and destination must
                                      * not be the same vector.
				      *
				      * @note This template only
				      * exists for compile-time
				      * compatibility with
				      * FullMatrix. Implementation is
				      * only available for <tt>VECTOR=Vector&lt;number&gt;</tt>
				      */
    template <class VECTOR>
    void vmult_add (VECTOR& w, const VECTOR& v) const;

				     /**
				      * Transpose
				      * matrix-vector-multiplication.
				      *
				      * The optional parameter
				      * <tt>adding</tt> determines, whether the
				      * result is stored in <tt>w</tt> or added
				      * to <tt>w</tt>.
				      *
				      * if (adding)
				      *  <i>w += A<sup>T</sup>*v</i>
				      *
				      * if (!adding)
				      *  <i>w = A<sup>T</sup>*v</i>
                                      *
                                      *
                                      * Source and destination must
                                      * not be the same vector.
				      *
				      * @note This template only
				      * exists for compile-time
				      * compatibility with
				      * FullMatrix. Implementation is
				      * only available for <tt>VECTOR=Vector&lt;number&gt;</tt>
				      */
    template <class VECTOR>
    void Tvmult (VECTOR& w, const VECTOR& v,
		 const bool            adding=false) const;

				     /**
				      * Adding transpose
				      * matrix-vector-multiplication.
				      *  <i>w += A<sup>T</sup>*v</i>
                                      *
                                      * Source and destination must
                                      * not be the same vector.
				      *
				      * @note This template only
				      * exists for compile-time
				      * compatibility with
				      * FullMatrix. Implementation is
				      * only available for <tt>VECTOR=Vector&lt;number&gt;</tt>
				      */
    template <class VECTOR>
    void Tvmult_add (VECTOR& w, const VECTOR& v) const;
    
    void vmult (Vector<number>   &w,
		const Vector<number> &v,
		const bool            adding=false) const;
    void vmult_add (Vector<number>       &w,
		    const Vector<number> &v) const;
    void Tvmult (Vector<number>       &w,
		 const Vector<number> &v,
		 const bool            adding=false) const;
    void Tvmult_add (Vector<number>       &w,
		     const Vector<number> &v) const;
				     /**
				      * Compute the LU factorization
				      * of the matrix using LAPACK
				      * function Xgetrf.
				      */
    void compute_lu_factorization ();

				     /**
				      * Invert the matrix by first computing
				      * an LU factorization with the LAPACK
				      * function Xgetrf and then building
				      * the actual inverse using Xgetri.
				      */
    void invert ();

				     /**
				      * Solve the linear system with
				      * right hand side given by
				      * applying forward/backward
				      * substitution to the previously
				      * computed LU
				      * factorization. Uses LAPACK
				      * function Xgetrs.
				      */
    void apply_lu_factorization (Vector<number>& v,
				 const bool transposed) const;

				     /**
				      * Compute eigenvalues of the
				      * matrix. After this routine has
				      * been called, eigenvalues can
				      * be retrieved using the
				      * eigenvalue() function. The
				      * matrix itself will be
				      * LAPACKSupport::unusable after
				      * this operation.
				      *
				      * The optional arguments allow
				      * to compute left and right
				      * eigenvectors as well.
				      *
				      * Note that the function does
				      * not return the computed
				      * eigenvalues right away since
				      * that involves copying data
				      * around between the output
				      * arrays of the LAPACK functions
				      * and any return array. This is
				      * often unnecessary since one
				      * may not be interested in all
				      * eigenvalues at once, but for
				      * example only the extreme
				      * ones. In that case, it is
				      * cheaper to just have this
				      * function compute the
				      * eigenvalues and have a
				      * separate function that returns
				      * whatever eigenvalue is
				      * requested.
				      *
				      * @note Calls the LAPACK
				      * function Xgeev.
				      */
    void compute_eigenvalues (const bool right_eigenvectors = false,
			      const bool left_eigenvectors = false);

				     /**
				      * Compute generalized eigenvalues
				      * and (optionally) eigenvectors of
				      * a real generalized symmetric
				      * eigenproblem of the form
				      * itype = 1: $Ax=\lambda B x$
				      * itype = 2: $ABx=\lambda x$
				      * itype = 3: $BAx=\lambda x$,
				      * where A is this matrix.
				      * A and B are assumed to be symmetric,
				      * and B has to be positive definite.
				      * After this routine has
				      * been called, eigenvalues can
				      * be retrieved using the
				      * eigenvalue() function. The
				      * matrix itself will be
				      * LAPACKSupport::unusable after
				      * this operation. The number of
				      * computed eigenvectors is equal
				      * to eigenvectors.size()
				      *
				      * Note that the function does
				      * not return the computed
				      * eigenvalues right away since
				      * that involves copying data
				      * around between the output
				      * arrays of the LAPACK functions
				      * and any return array. This is
				      * often unnecessary since one
				      * may not be interested in all
				      * eigenvalues at once, but for
				      * example only the extreme
				      * ones. In that case, it is
				      * cheaper to just have this
				      * function compute the
				      * eigenvalues and have a
				      * separate function that returns
				      * whatever eigenvalue is
				      * requested.
				      *
				      * @note Calls the LAPACK
				      * function Xsygv. For this to
				      * work, ./configure has to
				      * be told to use LAPACK.
				      */
    void compute_generalized_eigenvalues_symmetric (
                              LAPACKFullMatrix<number> & B,
                              std::vector<Vector<number> > & eigenvectors,
			      const int itype = 1);

				     /**
				      * Compute the singular value
				      * decomposition of the
				      * matrix using LAPACK function
				      * Xgesdd.
				      *
				      * Requires that the #state is
				      * LAPACKSupport::matrix, fills
				      * the data members #wr, #svd_u,
				      * and #svd_vt, and leaves the
				      * object in the #state
				      * LAPACKSupport::svd.
				      */
    void compute_svd();
				     /**
				      * Compute the inverse of the
				      * matrix by singular value
				      * decomposition.
				      *
				      * Requires that #state is either
				      * LAPACKSupport::matrix or
				      * LAPACKSupport::svd. In the
				      * first case, this function
				      * calls compute_svd(). After
				      * this function, the object will
				      * have the #state
				      * LAPACKSupport::inverse_svd.
				      *
				      * For a singular value
				      * decomposition, the inverse is
				      * simply computed by replacing
				      * all singular values by their
				      * reciprocal values. If the
				      * matrix does not have maximal
				      * rank, singular values 0 are
				      * not touched, thus computing
				      * the minimal norm right inverse
				      * of the matrix.
				      *
				      * The parameter
				      * <tt>threshold</tt> determines,
				      * when a singular value should
				      * be considered zero. It is the
				      * ratio of the smallest to the
				      * largest nonzero singular
				      * value
				      * <i>s</i><sub>max</sub>. Thus,
				      * the inverses of all singular
				      * values less than
				      * <i>s</i><sub>max</sub>/<tt>threshold</tt>
				      * will be set to zero.
				      */
    void compute_inverse_svd (const double threshold = 0.);
    
				     /**
				      * Retrieve eigenvalue after
				      * compute_eigenvalues() was
				      * called.
				      */
    std::complex<number>
    eigenvalue (const unsigned int i) const;

				     /**
				      * Retrieve singular values after
				      * compute_svd() or
				      * compute_inverse_svd() was
				      * called.
				      */
    number
    singular_value (const unsigned int i) const;
    
				     /**
				      * Print the matrix and allow
				      * formatting of entries.
				      *
				      * The parameters allow for a
				      * flexible setting of the output
				      * format:
				      *
				      * @arg <tt>precision</tt>
				      * denotes the number of trailing
				      * digits.
				      *
				      * @arg <tt>scientific</tt> is
				      * used to determine the number
				      * format, where
				      * <tt>scientific</tt> =
				      * <tt>false</tt> means fixed
				      * point notation.
				      *
				      * @arg <tt>width</tt> denotes
				      * the with of each column. A
				      * zero entry for <tt>width</tt>
				      * makes the function compute a
				      * width, but it may be changed
				      * to a positive value, if output
				      * is crude.
				      *
				      * @arg <tt>zero_string</tt>
				      * specifies a string printed for
				      * zero entries.
				      *
				      * @arg <tt>denominator</tt>
				      * Multiply the whole matrix by
				      * this common denominator to get
				      * nicer numbers.
				      *
				      * @arg <tt>threshold</tt>: all
				      * entries with absolute value
				      * smaller than this are
				      * considered zero.
				     */
    void print_formatted (std::ostream       &out,
			  const unsigned int  presicion=3,
			  const bool          scientific  = true,
			  const unsigned int  width       = 0,
			  const char         *zero_string = " ",
			  const double        denominator = 1.,
			  const double        threshold   = 0.) const;

  private:
				     /**
				      * Since LAPACK operations
				      * notoriously change the meaning
				      * of the matrix entries, we
				      * record the current state after
				      * the last operation here.
				      */
    LAPACKSupport::State state;
				     /**
				      * Additional properties of the
				      * matrix which may help to
				      * select more efficient LAPACK
				      * functions.
				      */
    LAPACKSupport::Properties properties;

				     /**
				      * The working array used for
				      * some LAPACK functions.
				      */
    mutable std::vector<number> work;

				     /**
				      * The vector storing the
				      * permutations applied for
				      * pivoting in the
				      * LU-factorization.
				      *
				      * Also used as the scratch array
				      * IWORK for LAPACK functions
				      * needing it.
				      */
    std::vector<int> ipiv;

				     /**
				      * Workspace for calculating the
				      * inverse matrix from an LU
				      * factorization.
				      */
    std::vector<number> inv_work;

				     /**
				      * Real parts of eigenvalues or
				      * the singular values. Filled by
				      * compute_eigenvalues() or compute_svd().
				      */
    std::vector<number> wr;

				     /**
				      * Imaginary parts of
				      * eigenvalues. Filled by
				      * compute_eigenvalues.
				      */
    std::vector<number> wi;

				     /**
				      * Space where left eigenvectors
				      * can be stored.
				      */
    std::vector<number> vl;

				     /**
				      * Space where right eigenvectors
				      * can be stored.
				      */
    std::vector<number> vr;
    
				     /**
				      * The matrix <i>U</i> in the
				      * singular value decomposition
				      * <i>USV<sup>T</sup></i>.
				      */
    std_cxx1x::shared_ptr<LAPACKFullMatrix<number> > svd_u;
				     /**
				      * The matrix
				      * <i>V<sup>T</sup></i> in the
				      * singular value decomposition
				      * <i>USV<sup>T</sup></i>.
				      */
    std_cxx1x::shared_ptr<LAPACKFullMatrix<number> > svd_vt;
};



/**
 * A preconditioner based on the LU-factorization of LAPACKFullMatrix.
 *
 * @ingroup Preconditioners
 * @author Guido Kanschat, 2006
 */
template <typename number>
class PreconditionLU
  :
  public Subscriptor
{
  public:
    void initialize(const LAPACKFullMatrix<number>&);
    void initialize(const LAPACKFullMatrix<number>&,
		    VectorMemory<Vector<number> >&);
    void vmult(Vector<number>&, const Vector<number>&) const;
    void Tvmult(Vector<number>&, const Vector<number>&) const;
    void vmult(BlockVector<number>&,
	       const BlockVector<number>&) const;
    void Tvmult(BlockVector<number>&,
		const BlockVector<number>&) const;
  private:
    SmartPointer<const LAPACKFullMatrix<number>,PreconditionLU<number> > matrix;
    SmartPointer<VectorMemory<Vector<number> >,PreconditionLU<number> > mem;
};



template <typename number>
template <class MATRIX>
inline void
LAPACKFullMatrix<number>::copy_from (const MATRIX& M)
{
  this->reinit (M.m(), M.n());
  const typename MATRIX::const_iterator end = M.end();
  for (typename MATRIX::const_iterator entry = M.begin();
       entry != end; ++entry)
    this->el(entry->row(), entry->column()) = entry->value();

  state = LAPACKSupport::matrix;
}



template <typename number>
template <class MATRIX>
inline void
LAPACKFullMatrix<number>::fill (
  const MATRIX& M,
  const unsigned int dst_offset_i,
  const unsigned int dst_offset_j,
  const unsigned int src_offset_i,
  const unsigned int src_offset_j,
  const number factor,
  const bool transpose)
{
  const typename MATRIX::const_iterator end = M.end();
  for (typename MATRIX::const_iterator entry = M.begin(src_offset_i);
       entry != end; ++entry)
    {
      const unsigned int i = transpose ? entry->column() : entry->row();
      const unsigned int j = transpose ? entry->row() : entry->column();

      const unsigned int dst_i=dst_offset_i+i-src_offset_i;
      const unsigned int dst_j=dst_offset_j+j-src_offset_j;
      if (dst_i<this->n_rows() && dst_j<this->n_cols())
	(*this)(dst_i, dst_j) = factor * entry->value();
    }

  state = LAPACKSupport::matrix;
}


template <typename number>
template <class VECTOR>
inline void
LAPACKFullMatrix<number>::vmult(VECTOR&, const VECTOR&, bool) const
{
  Assert(false, ExcNotImplemented());
}


template <typename number>
template <class VECTOR>
inline void
LAPACKFullMatrix<number>::vmult_add(VECTOR&, const VECTOR&) const
{
  Assert(false, ExcNotImplemented());
}


template <typename number>
template <class VECTOR>
inline void
LAPACKFullMatrix<number>::Tvmult(VECTOR&, const VECTOR&, bool) const
{
  Assert(false, ExcNotImplemented());
}


template <typename number>
template <class VECTOR>
inline void
LAPACKFullMatrix<number>::Tvmult_add(VECTOR&, const VECTOR&) const
{
  Assert(false, ExcNotImplemented());
}


template <typename number>
inline std::complex<number>
LAPACKFullMatrix<number>::eigenvalue (const unsigned int i) const
{
  Assert (state & LAPACKSupport::eigenvalues, ExcInvalidState());
  Assert (wr.size() == this->n_rows(), ExcInternalError());
  Assert (wi.size() == this->n_rows(), ExcInternalError());
  Assert (i<this->n_rows(), ExcIndexRange(i,0,this->n_rows()));

  return std::complex<number>(wr[i], wi[i]);
}


template <typename number>
inline number
LAPACKFullMatrix<number>::singular_value (const unsigned int i) const
{
  Assert (state == LAPACKSupport::svd || state == LAPACKSupport::inverse_svd, LAPACKSupport::ExcState(state));
  AssertIndexRange(i,wr.size());

  return wr[i];
}



DEAL_II_NAMESPACE_CLOSE

#endif
