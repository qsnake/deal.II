//---------------------------------------------------------------------------
//    $Id: mg_matrix.h 23876 2011-06-28 18:21:51Z kanschat $
//
//    Copyright (C) 2003, 2004, 2005, 2006, 2009, 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__mg_matrix_h
#define __deal2__mg_matrix_h

#include <deal.II/lac/vector.h>
#include <deal.II/lac/pointer_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/multigrid/mg_base.h>
#include <deal.II/base/mg_level_object.h>
#include <deal.II/base/std_cxx1x/shared_ptr.h>

DEAL_II_NAMESPACE_OPEN

/*!@addtogroup mg */
/*@{*/

namespace mg
{
/**
 * Multilevel matrix. This matrix stores an MGLevelObject of
 * PointerMatrixBase objects. It implementes the interface defined in
 * MGMatrixBase, so that it can be used as a matrix in Multigrid.
 *
 * @author Guido Kanschat
 * @date 2002, 2010
 */
  template <class VECTOR = Vector<double> >
  class Matrix
    : public MGMatrixBase<VECTOR>
  {
    public:
				       /**
					* Default constructor for an
					* empty object.
					*/
      Matrix();
      
				       /**
					* Constructor setting up
					* pointers to the matrices in
					* <tt>M</tt> by calling initialize().
					*/
      template <class MATRIX>
      Matrix(const MGLevelObject<MATRIX>& M);
      
				       /**
					* Initialize the object such
					* that the level
					* multiplication uses the
					* matrices in <tt>M</tt>
					*/
      template <class MATRIX>
      void
      initialize(const MGLevelObject<MATRIX>& M);

				       /**
					* Access matrix on a level.
					*/
      const PointerMatrixBase<VECTOR>& operator[] (unsigned int level) const;
      
      virtual void vmult (const unsigned int level, VECTOR& dst, const VECTOR& src) const;
      virtual void vmult_add (const unsigned int level, VECTOR& dst, const VECTOR& src) const;
      virtual void Tvmult (const unsigned int level, VECTOR& dst, const VECTOR& src) const;
      virtual void Tvmult_add (const unsigned int level, VECTOR& dst, const VECTOR& src) const;
      
				       /**
					* Memory used by this object.
					*/
      std::size_t memory_consumption () const;
    private:
      MGLevelObject<std_cxx1x::shared_ptr<PointerMatrixBase<VECTOR> > > matrices;
  };
  
}

/**
 * @deprecated Use the much simpler class MG::Matrix instead.
 *
 * Multilevel matrix. This class implements the interface defined by
 * MGMatrixBase, using MGLevelObject of an arbitrary
 * matrix class.
 *
 * @author Guido Kanschat, 2002
 */
template <class MATRIX = SparseMatrix<double>, class VECTOR = Vector<double> >
class MGMatrix : public MGMatrixBase<VECTOR>
{
  public:
				     /**
				      * Constructor. The argument is
				      * handed over to the
				      * @p SmartPointer
				      * constructor. The matrix object
				      * must exist longer as the
				      * @p MGMatrix object, since
				      * only a pointer is stored.
				      */
    MGMatrix (MGLevelObject<MATRIX>* = 0);

				     /**
				      * Set the matrix object to be
				      * used. The matrix object must
				      * exist longer as the
				      * @p MGMatrix object, since
				      * only a pointer is stored.
				      */
    void set_matrix (MGLevelObject<MATRIX>* M);

				     /**
				      * Matrix-vector-multiplication on
				      * a certain level.
				      */
    virtual void vmult (const unsigned int level,
			VECTOR& dst,
			const VECTOR& src) const;

				     /**
				      * Adding matrix-vector-multiplication on
				      * a certain level.
				      */
    virtual void vmult_add (const unsigned int level,
			    VECTOR& dst,
			    const VECTOR& src) const;

				     /**
				      * Transpose
				      * matrix-vector-multiplication on
				      * a certain level.
				      */
    virtual void Tvmult (const unsigned int level,
			 VECTOR& dst,
			 const VECTOR& src) const;

				     /**
				      * Adding transpose
				      * matrix-vector-multiplication on
				      * a certain level.
				      */
    virtual void Tvmult_add (const unsigned int level,
			     VECTOR& dst,
			     const VECTOR& src) const;

				     /**
				      * Memory used by this object.
				      */
    std::size_t memory_consumption () const;


  private:
				     /**
				      * Pointer to the matrix objects on each level.
				      */
    SmartPointer<MGLevelObject<MATRIX>,MGMatrix<MATRIX,VECTOR> > matrix;
};


/**
 * Multilevel matrix selecting from block matrices. This class
 * implements the interface defined by MGMatrixBase.  The
 * template parameter @p MATRIX should be a block matrix class like
 * BlockSparseMatrix or @p BlockSparseMatrixEZ. Then, this
 * class stores a pointer to a MGLevelObject of this matrix
 * class. In each @p vmult, the block selected on initialization will
 * be multiplied with the vector provided.
 *
 * @author Guido Kanschat, 2002
 */
template <class MATRIX, typename number>
class MGMatrixSelect : public MGMatrixBase<Vector<number> >
{
  public:
				     /**
				      * Constructor. @p row and
				      * @p col are the coordinate of
				      * the selected block. The other
				      * argument is handed over to the
				      * @p SmartPointer constructor.
				      */
    MGMatrixSelect (const unsigned int row = 0,
		    const unsigned int col = 0,
		    MGLevelObject<MATRIX>* = 0);

				     /**
				      * Set the matrix object to be
				      * used. The matrix object must
				      * exist longer as the
				      * @p MGMatrix object, since
				      * only a pointer is stored.
				      */
    void set_matrix (MGLevelObject<MATRIX>* M);

				     /**
				      * Select the block for
				      * multiplication.
				      */
    void select_block (const unsigned int row,
		       const unsigned int col);

				     /**
				      * Matrix-vector-multiplication on
				      * a certain level.
				      */
    virtual void vmult (const unsigned int level,
			Vector<number>& dst,
			const Vector<number>& src) const;

				     /**
				      * Adding matrix-vector-multiplication on
				      * a certain level.
				      */
    virtual void vmult_add (const unsigned int level,
			    Vector<number>& dst,
			    const Vector<number>& src) const;

				     /**
				      * Transpose
				      * matrix-vector-multiplication on
				      * a certain level.
				      */
    virtual void Tvmult (const unsigned int level,
			 Vector<number>& dst,
			 const Vector<number>& src) const;

				     /**
				      * Adding transpose
				      * matrix-vector-multiplication on
				      * a certain level.
				      */
    virtual void Tvmult_add (const unsigned int level,
			     Vector<number>& dst,
			     const Vector<number>& src) const;

  private:
				     /**
				      * Pointer to the matrix objects on each level.
				      */
    SmartPointer<MGLevelObject<MATRIX>,MGMatrixSelect<MATRIX,number> > matrix;
				     /**
				      * Row coordinate of selected block.
				      */
    unsigned int row;
				     /**
				      * Column coordinate of selected block.
				      */
    unsigned int col;

};

/*@}*/

/*----------------------------------------------------------------------*/

namespace mg
{
  template <class VECTOR>
  template <class MATRIX>
  inline
  void
  Matrix<VECTOR>::initialize (const MGLevelObject<MATRIX>& p)
  {
    matrices.resize(p.min_level(), p.max_level());
    for (unsigned int level=p.min_level();level <= p.max_level();++level)
      matrices[level] = std_cxx1x::shared_ptr<PointerMatrixBase<VECTOR> > (new_pointer_matrix_base(p[level], VECTOR()));
  }
  
  
  template <class VECTOR>
  template <class MATRIX>
  inline
  Matrix<VECTOR>::Matrix (const MGLevelObject<MATRIX>& p)
  {
    initialize(p);
  }
  
  
  template <class VECTOR>
  inline
  Matrix<VECTOR>::Matrix ()
  {}

  
  template <class VECTOR>
  inline
  const PointerMatrixBase<VECTOR>&
  Matrix<VECTOR>::operator[] (unsigned int level) const
  {
    return *matrices[level];
  }
  

  template <class VECTOR>
  void
  Matrix<VECTOR>::vmult  (
    const unsigned int level,
    VECTOR& dst,
    const VECTOR& src) const
  {
    matrices[level]->vmult(dst, src);
  }


  template <class VECTOR>
  void
  Matrix<VECTOR>::vmult_add  (
    const unsigned int level,
    VECTOR& dst,
    const VECTOR& src) const
  {
    matrices[level]->vmult_add(dst, src);
  }


  template <class VECTOR>
  void
  Matrix<VECTOR>::Tvmult  (const unsigned int level,
			   VECTOR& dst,
			   const VECTOR& src) const
  {
    matrices[level]->Tvmult(dst, src);
  }


  template <class VECTOR>
  void
  Matrix<VECTOR>::Tvmult_add  (const unsigned int level,
			       VECTOR& dst,
			       const VECTOR& src) const
  {
    matrices[level]->Tvmult_add(dst, src);
  }


  template <class VECTOR>
  inline
  std::size_t
  Matrix<VECTOR>::memory_consumption () const
  {
    return sizeof(*this) + matrices->memory_consumption();
  }
}

/*----------------------------------------------------------------------*/

template <class MATRIX, class VECTOR>
MGMatrix<MATRIX, VECTOR>::MGMatrix (MGLevelObject<MATRIX>* p)
		:
		matrix (p, typeid(*this).name())
{}


template <class MATRIX, class VECTOR>
void
MGMatrix<MATRIX, VECTOR>::set_matrix (MGLevelObject<MATRIX>* p)
{
  matrix = p;
}


template <class MATRIX, class VECTOR>
void
MGMatrix<MATRIX, VECTOR>::vmult  (const unsigned int level,
				  VECTOR& dst,
				  const VECTOR& src) const
{
  Assert(matrix != 0, ExcNotInitialized());

  const MGLevelObject<MATRIX>& m = *matrix;
  m[level].vmult(dst, src);
}


template <class MATRIX, class VECTOR>
void
MGMatrix<MATRIX, VECTOR>::vmult_add  (const unsigned int level,
				      VECTOR& dst,
				      const VECTOR& src) const
{
  Assert(matrix != 0, ExcNotInitialized());

  const MGLevelObject<MATRIX>& m = *matrix;
  m[level].vmult_add(dst, src);
}


template <class MATRIX, class VECTOR>
void
MGMatrix<MATRIX, VECTOR>::Tvmult  (const unsigned int level,
				   VECTOR& dst,
				   const VECTOR& src) const
{
  Assert(matrix != 0, ExcNotInitialized());

  const MGLevelObject<MATRIX>& m = *matrix;
  m[level].Tvmult(dst, src);
}


template <class MATRIX, class VECTOR>
void
MGMatrix<MATRIX, VECTOR>::Tvmult_add  (const unsigned int level,
				       VECTOR& dst,
				       const VECTOR& src) const
{
  Assert(matrix != 0, ExcNotInitialized());

  const MGLevelObject<MATRIX>& m = *matrix;
  m[level].Tvmult_add(dst, src);
}


template <class MATRIX, class VECTOR>
std::size_t
MGMatrix<MATRIX, VECTOR>::memory_consumption () const
{
  return sizeof(*this) + matrix->memory_consumption();
}

/*----------------------------------------------------------------------*/

template <class MATRIX, typename number>
MGMatrixSelect<MATRIX, number>::
MGMatrixSelect (const unsigned int row,
		const unsigned int col,
		MGLevelObject<MATRIX>* p)
		:
		matrix (p, typeid(*this).name()),
		row(row),
		col(col)
{}



template <class MATRIX, typename number>
void
MGMatrixSelect<MATRIX, number>::set_matrix (MGLevelObject<MATRIX>* p)
{
  matrix = p;
}


template <class MATRIX, typename number>
void
MGMatrixSelect<MATRIX, number>::
select_block (const unsigned int brow,
	      const unsigned int bcol)
{
  row = brow;
  col = bcol;
}


template <class MATRIX, typename number>
void
MGMatrixSelect<MATRIX, number>::
vmult  (const unsigned int level,
	Vector<number>& dst,
	const Vector<number>& src) const
{
  Assert(matrix != 0, ExcNotInitialized());

  const MGLevelObject<MATRIX>& m = *matrix;
  m[level].block(row, col).vmult(dst, src);
}


template <class MATRIX, typename number>
void
MGMatrixSelect<MATRIX, number>::
vmult_add  (const unsigned int level,
	    Vector<number>& dst,
	    const Vector<number>& src) const
{
  Assert(matrix != 0, ExcNotInitialized());

  const MGLevelObject<MATRIX>& m = *matrix;
  m[level].block(row, col).vmult_add(dst, src);
}


template <class MATRIX, typename number>
void
MGMatrixSelect<MATRIX, number>::
Tvmult  (const unsigned int level,
	 Vector<number>& dst,
	 const Vector<number>& src) const
{
  Assert(matrix != 0, ExcNotInitialized());

  const MGLevelObject<MATRIX>& m = *matrix;
  m[level].block(row, col).Tvmult(dst, src);
}


template <class MATRIX, typename number>
void
MGMatrixSelect<MATRIX, number>::
Tvmult_add  (const unsigned int level,
	     Vector<number>& dst,
	     const Vector<number>& src) const
{
  Assert(matrix != 0, ExcNotInitialized());

  const MGLevelObject<MATRIX>& m = *matrix;
  m[level].block(row, col).Tvmult_add(dst, src);
}

DEAL_II_NAMESPACE_CLOSE

#endif
