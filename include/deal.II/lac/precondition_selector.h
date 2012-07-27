//---------------------------------------------------------------------------
//    $Id: precondition_selector.h 23876 2011-06-28 18:21:51Z kanschat $
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2009 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__precondition_selector_h
#define __deal2__precondition_selector_h


#include <deal.II/base/config.h>
#include <deal.II/base/smartpointer.h>
#include <string>

DEAL_II_NAMESPACE_OPEN

template <class number> class Vector;
template <class number> class SparseMatrix;


/*! @addtogroup Preconditioners
 *@{
 */

/**
 * Selects the preconditioner. The constructor of this class takes 
 * the name of the preconditioning and the damping parameter 
 * @p omega of the preconditioning and the @p use_matrix function takes
 * the matrix that is used
 * by the matrix-builtin precondition functions. Each time, the <tt>operator()</tt> function
 * is called, this preselected preconditioner, this matrix and
 * this @p omega is used
 * for the preconditioning. This class is designed for being used as
 * argument of the @p solve function of a @p Solver and it covers the
 * selection of all matrix-builtin precondition functions. The selection
 * of other preconditioners, like BlockSOR or ILU should be handled in
 * derived classes by the user.
 *
 * <h3>Usage</h3>
 * The simplest use of this class is the following:
 * @verbatim
 *                                  // generate a @p SolverControl and
 *                                  // a @p VectorMemory
 * SolverControl control;
 * VectorMemory<Vector<double> > memory;
 *                                  // generate a solver
 * SolverCG<SparseMatrix<double>, Vector<double> > solver(control, memory);
 *                                  // generate a @p PreconditionSelector
 * PreconditionSelector<SparseMatrix<double>, Vector<double> >
 *   preconditioning("jacobi", 1.);
 *                                  // give a matrix whose diagonal entries
 *                                  // are to be used for the preconditioning.
 *                                  // Generally the matrix of the linear
 *                                  // equation system Ax=b.
 * preconditioning.use_matrix(A);
 *                                  // call the @p solve function with this
 *                                  // preconditioning as last argument
 * solver.solve(A,x,b,preconditioning);
 * @endverbatim
 * The same example where also the @p SolverSelector class is used reads
 * @verbatim
 *                                  // generate a @p SolverControl and
 *                                  // a @p VectorMemory
 * SolverControl control;
 * VectorMemory<Vector<double> > memory;
 *                                  // generate a @p SolverSelector that
 *                                  // calls the @p SolverCG
 * SolverSelector<SparseMatrix<double>, Vector<double> > 
 *   solver_selector("cg", control, memory);
 *                                  // generate a @p PreconditionSelector
 * PreconditionSelector<SparseMatrix<double>, Vector<double> >
 *   preconditioning("jacobi", 1.);
 *
 * preconditioning.use_matrix(A);
 *
 * solver_selector.solve(A,x,b,preconditioning);
 * @endverbatim
 * Now the use of the @p SolverSelector in combination with the @p PreconditionSelector
 * allows the user to select both, the solver and the preconditioner, at the
 * beginning of his program and each time the
 * solver is started (that is several times e.g. in a nonlinear iteration) this
 * preselected solver and preconditioner is called.
 *
 * @author Ralf Hartmann, 1999
 */
template <class MATRIX = SparseMatrix<double>,
          class VECTOR = dealii::Vector<double> >
class PreconditionSelector : public Subscriptor
{
  public:
    
				     /**
				      * Constructor. @p omega denotes
				      * the damping parameter of
				      * the preconditioning.
				      */
    PreconditionSelector(const std::string                 &preconditioning,
			 const typename VECTOR::value_type &omega=1.);
    
				     /**
				      * Destructor.
				      */
    virtual ~PreconditionSelector();

				     /**
				      * Takes the matrix that is needed
				      * for preconditionings that involves a
				      * matrix. e.g. for @p precondition_jacobi,
				      * <tt>~_sor</tt>, <tt>~_ssor</tt>.
				      */
    void use_matrix(const MATRIX &M);

				     /**
				      * Precondition procedure. Calls the
				      * preconditioning that was specified in
				      * the constructor.
				      */    
    virtual void vmult (VECTOR &dst, const VECTOR&src) const;

				     /**
				      * Get the names of all implemented
				      * preconditionings.
				      */
    static std::string get_precondition_names();

				     /** @addtogroup Exceptions
				      * @{ */


				     /**
				      * Exception.
				      */
    DeclException0 (ExcNoMatrixGivenToUse);

				     //@}
  protected:
    
				     /**
				      * Stores the name of the
				      * preconditioning.
				      */
    std::string preconditioning;

  private:
				     /**
				      * Matrix that is used for the
				      * matrix-builtin preconditioning function.
				      * cf. also @p PreconditionUseMatrix.
				      */
    SmartPointer<const MATRIX,PreconditionSelector<MATRIX,VECTOR> > A;
    
				     /**
				      * Stores the damping parameter
				      * of the preconditioner.
				      */
    const typename VECTOR::value_type omega;
};

/*@}*/
/* --------------------- Inline and template functions ------------------- */


template <class MATRIX, class VECTOR>
PreconditionSelector<MATRIX,VECTOR>
::PreconditionSelector(const std::string                 &preconditioning,
		       const typename VECTOR::value_type &omega) :
		preconditioning(preconditioning),
		omega(omega)  {}


template <class MATRIX, class VECTOR>
PreconditionSelector<MATRIX,VECTOR>::~PreconditionSelector()
{
				   // release the matrix A
  A=0;
}


template <class MATRIX, class VECTOR>
void PreconditionSelector<MATRIX,VECTOR>::use_matrix(const MATRIX &M)
{
  A=&M;
}

template <class MATRIX, class VECTOR>
void PreconditionSelector<MATRIX,VECTOR>::vmult (VECTOR &dst,
						 const VECTOR &src) const
{
  if (preconditioning=="none")
    {
      dst=src;
    }
  else 
    {
      Assert(A!=0, ExcNoMatrixGivenToUse());
      
      if (preconditioning=="jacobi")
	{
	  A->precondition_Jacobi(dst,src,omega);
	} 
      else if (preconditioning=="sor")
	{
	  A->precondition_SOR(dst,src,omega);      
	}
      else if (preconditioning=="ssor")
	{
	  A->precondition_SSOR(dst,src,omega);      
	}
      else 
	Assert(false,ExcNotImplemented());
    }
}


template <class MATRIX, class VECTOR>
std::string PreconditionSelector<MATRIX,VECTOR>::get_precondition_names()
{
  return "none|jacobi|sor|ssor";
}


DEAL_II_NAMESPACE_CLOSE

#endif
