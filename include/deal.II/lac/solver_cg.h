//---------------------------------------------------------------------------
//    $Id: solver_cg.h 23876 2011-06-28 18:21:51Z kanschat $
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__solver_cg_h
#define __deal2__solver_cg_h


#include <deal.II/base/config.h>
#include <deal.II/lac/tridiagonal_matrix.h>
#include <deal.II/lac/solver.h>
#include <deal.II/lac/solver_control.h>
#include <deal.II/base/exceptions.h>
#include <deal.II/base/logstream.h>
#include <deal.II/base/subscriptor.h>
#include <cmath>

DEAL_II_NAMESPACE_OPEN

/*!@addtogroup Solvers */
/*@{*/

/**
 * Preconditioned cg method for symmetric positive definite matrices.
 *
 * For the requirements on matrices and vectors in order to work with
 * this class, see the documentation of the Solver base class.
 *
 * Like all other solver classes, this class has a local structure
 * called @p AdditionalData which is used to pass additional
 * parameters to the solver, like damping parameters or the number of
 * temporary vectors. For this class, there is a switch allowing for
 * additional output for the computation of eigenvalues of the matrix.
 *
 * <h3>Eigenvalue computation</h3>
 *
 * See Y. Saad: "Iterative methods for Sparse Linear Systems", section
 * 6.7.3 for details.
 *
 * The cg-method performs an orthogonal projection of the original
 * preconditioned linear system to another system of smaller
 * dimension. Furthermore, the projected matrix @p T is
 * tri-diagonal. Since the projection is orthogonal, the eigenvalues
 * of @p T approximate those of the original preconditioned matrix
 * @p PA. In fact, after @p n steps, where @p n is the dimension of
 * the original system, the eigenvalues of both matrices are
 * equal. But, even for small numbers of iteration steps, the
 * condition number of @p T is a good estimate for the one of @p PA.
 *
 * With the coefficients @p alpha and @p beta written to the log
 * file if <tt>AdditionalData::log_coefficients = true</tt>, the matrix
 * @p T_m after @p m steps is the tri-diagonal matrix with diagonal
 * elements <tt>1/alpha_0</tt>, <tt>1/alpha_1 + beta_0/alpha_0</tt>, ...,
 * <tt>1/alpha_{m-1</tt>+beta_{m-2}/alpha_{m-2}} and off-diagonal elements
 * <tt>sqrt(beta_0)/alpha_0</tt>, ..., <tt>sqrt(beta_{m-2</tt>)/alpha_{m-2}}.
 * The eigenvalues of this matrix can be computed by postprocessing.
 *
 * This version of CG is taken from Braess: "Finite Elements". It
 * requires a symmetric preconditioner, i.e. SOR is not feasible.
 *
 * @author W. Bangerth, G. Kanschat, R. Becker and F.-T. Suttmeier
 */
template <class VECTOR = Vector<double> >
class SolverCG : public Solver<VECTOR>
{
  public:
    				     /**
				      * Standardized data struct to pipe
				      * additional data to the solver.
				      */
    struct AdditionalData
    {
                                         /**
                                          * Write coefficients alpha and beta
                                          * to the log file for later use in
                                          * eigenvalue estimates.
                                          */
        bool log_coefficients;

                                         /**
					  * Compute the condition
					  * number of the projected
					  * matrix.
					  *
					  * @note Requires LAPACK support.
					  */
	bool compute_condition_number;

                                         /**
					  * Compute the condition
					  * number of the projected
					  * matrix in each step.
					  *
					  * @note Requires LAPACK support.
					  */
	bool compute_all_condition_numbers;

					 /**
					  * Compute all eigenvalues of
					  * the projected matrix.
					  *
					  * @note Requires LAPACK support.
					  */
	bool compute_eigenvalues;

                                         /**
                                          * Constructor. Initialize data
                                          * fields.  Confer the description of
                                          * those.
                                          */
        AdditionalData (const bool log_coefficients = false,
			const bool compute_condition_number = false,
			const bool compute_all_condition_numbers = false,
			const bool compute_eigenvalues = false);
    };

				     /**
				      * Constructor.
				      */
    SolverCG (SolverControl        &cn,
	      VectorMemory<VECTOR> &mem,
	      const AdditionalData &data = AdditionalData());

				     /**
				      * Constructor. Use an object of
				      * type GrowingVectorMemory as
				      * a default to allocate memory.
				      */
    SolverCG (SolverControl        &cn,
	      const AdditionalData &data=AdditionalData());

				     /**
				      * Virtual destructor.
				      */
    virtual ~SolverCG ();

				     /**
				      * Solve the linear system $Ax=b$
				      * for x.
				      */
    template <class MATRIX, class PRECONDITIONER>
    void
    solve (const MATRIX         &A,
	   VECTOR               &x,
	   const VECTOR         &b,
	   const PRECONDITIONER &precondition);

  protected:
				     /**
				      * Implementation of the computation of
				      * the norm of the residual. This can be
				      * replaced by a more problem oriented
				      * functional in a derived class.
				      */
    virtual double criterion();

				     /**
				      * Interface for derived class.
				      * This function gets the current
				      * iteration vector, the residual
				      * and the update vector in each
				      * step. It can be used for a
				      * graphical output of the
				      * convergence history.
				      */
    virtual void print_vectors(const unsigned int step,
			       const VECTOR& x,
			       const VECTOR& r,
			       const VECTOR& d) const;

				     /**
				      * Temporary vectors, allocated through
				      * the @p VectorMemory object at the start
				      * of the actual solution process and
				      * deallocated at the end.
				      */
    VECTOR *Vr;
    VECTOR *Vp;
    VECTOR *Vz;
    VECTOR *VAp;

				     /**
				      * Within the iteration loop, the
				      * square of the residual vector is
				      * stored in this variable. The
				      * function @p criterion uses this
				      * variable to compute the convergence
				      * value, which in this class is the
				      * norm of the residual vector and thus
				      * the square root of the @p res2 value.
				      */
    double res2;

				     /**
				      * Additional parameters.
				      */
    AdditionalData additional_data;

  private:
    void cleanup();
};

/*@}*/

/*------------------------- Implementation ----------------------------*/

#ifndef DOXYGEN

template <class VECTOR>
inline
SolverCG<VECTOR>::AdditionalData::
AdditionalData (const bool log_coefficients,
		const bool compute_condition_number,
		const bool compute_all_condition_numbers,
		const bool compute_eigenvalues)
                :
                log_coefficients (log_coefficients),
		compute_condition_number(compute_condition_number),
		compute_all_condition_numbers(compute_all_condition_numbers),
		compute_eigenvalues(compute_eigenvalues)
{}



template <class VECTOR>
SolverCG<VECTOR>::SolverCG (SolverControl        &cn,
			    VectorMemory<VECTOR> &mem,
			    const AdditionalData &data)
		:
                Solver<VECTOR>(cn,mem),
                additional_data(data)
{}



template <class VECTOR>
SolverCG<VECTOR>::SolverCG (SolverControl        &cn,
			    const AdditionalData &data)
		:
                Solver<VECTOR>(cn),
                additional_data(data)
{}



template <class VECTOR>
SolverCG<VECTOR>::~SolverCG ()
{}



template <class VECTOR>
double
SolverCG<VECTOR>::criterion()
{
  return std::sqrt(res2);
}



template <class VECTOR>
void
SolverCG<VECTOR>::cleanup()
{
  this->memory.free(Vr);
  this->memory.free(Vp);
  this->memory.free(Vz);
  this->memory.free(VAp);
  deallog.pop();
}



template <class VECTOR>
void
SolverCG<VECTOR>::print_vectors(const unsigned int,
				const VECTOR&,
				const VECTOR&,
				const VECTOR&) const
{}



template <class VECTOR>
template <class MATRIX, class PRECONDITIONER>
void
SolverCG<VECTOR>::solve (const MATRIX         &A,
			 VECTOR               &x,
			 const VECTOR         &b,
			 const PRECONDITIONER &precondition)
{
  SolverControl::State conv=SolverControl::iterate;

  deallog.push("cg");

				   // Memory allocation
  Vr  = this->memory.alloc();
  Vp  = this->memory.alloc();
  Vz  = this->memory.alloc();
  VAp = this->memory.alloc();
				   // Should we build the matrix for
				   // eigenvalue computations?
  bool do_eigenvalues = additional_data.compute_condition_number
			| additional_data.compute_all_condition_numbers
			| additional_data.compute_eigenvalues;
  double eigen_beta_alpha = 0;

				   // vectors used for eigenvalue
				   // computations
  std::vector<double> diagonal;
  std::vector<double> offdiagonal;

  try {
				     // define some aliases for simpler access
    VECTOR& g  = *Vr;
    VECTOR& h  = *Vp;
    VECTOR& d  = *Vz;
    VECTOR& Ad = *VAp;
				     // resize the vectors, but do not set
				     // the values since they'd be overwritten
				     // soon anyway.
    g.reinit(x, true);
    h.reinit(x, true);
    d.reinit(x, true);
    Ad.reinit(x, true);
				     // Implementation taken from the DEAL
				     // library
    int  it=0;
    double res,gh,alpha,beta;

				     // compute residual. if vector is
				     // zero, then short-circuit the
				     // full computation
    if (!x.all_zero())
      {
	A.vmult(g,x);
	g.add(-1.,b);
      }
    else
      g.equ(-1.,b);
    res = g.l2_norm();

    conv = this->control().check(0,res);
    if (conv)
      {
	cleanup();
	return;
      }

    precondition.vmult(h,g);

    d.equ(-1.,h);

    gh = g*h;

    while (conv == SolverControl::iterate)
      {
	it++;
	A.vmult(Ad,d);

	alpha = d*Ad;
	Assert(alpha != 0., ExcDivideByZero());
	alpha = gh/alpha;

	g.add(alpha,Ad);
	x.add(alpha,d );
	res = g.l2_norm();

	print_vectors(it, x, g, d);

	conv = this->control().check(it,res);
	if (conv != SolverControl::iterate)
	  break;

	precondition.vmult(h,g);

	beta = gh;
	Assert(beta != 0., ExcDivideByZero());
	gh   = g*h;
	beta = gh/beta;

	if (additional_data.log_coefficients)
	  deallog << "alpha-beta:" << alpha << '\t' << beta << std::endl;
					 // set up the vectors
					 // containing the diagonal
					 // and the off diagonal of
					 // the projected matrix.
	if (do_eigenvalues)
	  {
	    diagonal.push_back(1./alpha + eigen_beta_alpha);
	    eigen_beta_alpha = beta/alpha;
	    offdiagonal.push_back(std::sqrt(beta)/alpha);
	  }
	
	if (additional_data.compute_all_condition_numbers && (diagonal.size()>1))
	  {
	    TridiagonalMatrix<double> T(diagonal.size(), true);
	    for (unsigned int i=0;i<diagonal.size();++i)
	      {
		T(i,i) = diagonal[i];
		if (i< diagonal.size()-1)
		  T(i,i+1) = offdiagonal[i];
	      }
	    T.compute_eigenvalues();
	    deallog << "Condition number estimate: " <<
	      T.eigenvalue(T.n()-1)/T.eigenvalue(0) << std::endl;
	  }

	d.sadd(beta,-1.,h);
      }
  }
  catch (...)
    {
      cleanup();
      throw;
    }

				   // Write eigenvalues or condition number
  if (do_eigenvalues)
    {
      TridiagonalMatrix<double> T(diagonal.size(), true);
      for (unsigned int i=0;i<diagonal.size();++i)
        {
          T(i,i) = diagonal[i];
          if (i< diagonal.size()-1)
            T(i,i+1) = offdiagonal[i];
        }
      T.compute_eigenvalues();
      if (additional_data.compute_condition_number
          && ! additional_data.compute_all_condition_numbers
	  && (diagonal.size() > 1))
        deallog << "Condition number estimate: " <<
          T.eigenvalue(T.n()-1)/T.eigenvalue(0) << std::endl;
      if (additional_data.compute_eigenvalues)
        {
          for (unsigned int i=0;i<T.n();++i)
            deallog << ' ' << T.eigenvalue(i);
          deallog << std::endl;
        }
    }

				   // Deallocate Memory
  cleanup();
				   // in case of failure: throw
				   // exception
  if (this->control().last_check() != SolverControl::success)
    throw SolverControl::NoConvergence (this->control().last_step(),
					this->control().last_value());
				   // otherwise exit as normal
}

#endif // DOXYGEN

DEAL_II_NAMESPACE_CLOSE

#endif
