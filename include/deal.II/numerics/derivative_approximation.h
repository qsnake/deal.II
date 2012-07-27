
//---------------------------------------------------------------------------
//    $Id: derivative_approximation.h 23876 2011-06-28 18:21:51Z kanschat $
//
//    Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__gradient_estimator_h
#define __deal2__gradient_estimator_h

#include <deal.II/base/config.h>
#include <deal.II/base/exceptions.h>
#include <deal.II/fe/fe_update_flags.h>
#include <deal.II/fe/mapping.h>
#include <utility>

DEAL_II_NAMESPACE_OPEN

template <int dim, int spacedim> class DoFHandler;
namespace hp
{
  template <int dim, int spacedim> class DoFHandler;
}



/**
 * This class computes a cell-wise approximation of the norm of a
 * derivative of a finite element field by taking difference quotients
 * between neighboring cells. This is a rather simple but efficient
 * form to get an error indicator, since it can be computed with
 * relatively little numerical effort and yet gives a reasonable
 * approximation.
 *
 * The way the difference quotients are computed on cell $K$ is the
 * following (here described for the approximation of the gradient of
 * a finite element field, but see below for higher derivatived): let
 * $K'$ be a neighboring cell, and let $y_{K'}=x_{K'}-x_K$ be the
 * distance vector between the centers of the two cells, then
 *   $ \frac{u_h(x_{K'}) - u_h(x_K)}{ \|y_{K'}\| }$
 * is an approximation of the directional derivative
 *   $ \nabla u(x_K) \cdot \frac{y_{K'}}{ \|y_{K'}\| }.$
 * By multiplying both terms by $\frac{y_{K'}}{ \|y_{K'}\| }$ from the 
 * left and summing over all neighbors $K'$, we obtain
 *   $ \sum_{K'} \left( \frac{y_{K'}}{ \|y_{K'}\|} 
 *                      \frac{y_{K'}^T}{ \|y_{K'}\| } \right) \nabla u(x_K)
 *     \approx
 *     \sum_{K'} \left( \frac{y_{K'}}{ \|y_{K'}\|} 
 *                      \frac{u_h(x_{K'}) - u_h(x_K)}{ \|y_{K'}\| }  \right).$
 *
 * Thus, if the matrix
 *   $ Y =  \sum_{K'} \left( \frac{y_{K'}}{\|y_{K'}\|} 
 *                           \frac{y_{K'}^T}{ \|y_{K'}\| } \right)$ is
 * regular (which is the case when the vectors $y_{K'}$ to all neighbors span
 * the whole space), we can obtain an approximation to the true gradient by
 *   $ \nabla u(x_K)
 *     \approx
 *     Y^{-1} \sum_{K'} \left( \frac{y_{K'}}{\|y_{K'}\|} 
 *                             \frac{u_h(x_{K'}) - u_h(x_K)}{ \|y_{K'}\| }
 *                      \right).$
 * This is a quantity that is easily computed. The value returned for
 * each cell when calling the @p approximate_gradient function of
 * this class is the $l_2$ norm of this approximation to the
 * gradient. To make this a useful quantity, you may want to scale
 * each element by the correct power of the respective cell size.
 *
 * The computation of this quantity must fail if a cell has only
 * neighbors for which the direction vectors $y_K$ do not span the
 * whole space, since then the matrix $Y$ is no longer invertible. If
 * this happens, you will get an error similar to this one:
 * @code
 * --------------------------------------------------------
 * An error occurred in line <749> of file <source/numerics/derivative_approximation.cc> in function
 *     static void DerivativeApproximation::approximate(const Mapping<dim,spacedim>&, const DH<dim,spacedim>&, const InputVector&, unsigned int, const
 *  std::pair<unsigned int, unsigned int>&, Vector<float>&) [with DerivativeDescription = DerivativeApproximation::Gradient<3>, int
 * dim = 3, DH = DoFHandler, InputVector = Vector<double>]
 * The violated condition was:
 *     determinant(Y) != 0
 * The name and call sequence of the exception was:
 *     ExcInsufficientDirections()
 * Additional Information:
 * (none)
 * --------------------------------------------------------
 * @endcode
 * As can easily be verified, this can only happen on very
 * coarse grids, when some cells and all their neighbors have not been
 * refined even once. You should therefore only call the functions of
 * this class if all cells are at least once refined. In practice this
 * is not much of a restriction.
 *
 *
 * <h3>Approximation of higher derivatives</h3>
 *
 * Similar to the reasoning above, approximations to higher
 * derivatives can be computed in a similar fashion. For example, the
 * tensor of second derivatives is approximated by the formula
 *   $ \nabla^2 u(x_K)
 *     \approx
 *     Y^{-1}
 *     \sum_{K'}
 *        \left(
 *           \frac{y_{K'}}{\|y_{K'}\|} \otimes
 *           \frac{\nabla u_h(x_{K'}) - \nabla u_h(x_K)}{ \|y_{K'}\| }
 *        \right),
 *   $ 
 * where $\otimes$ denotes the outer product of two vectors. Note that
 * unlike the true tensor of second derivatives, its approximation is
 * not necessarily symmetric. This is due to the fact that in the
 * derivation, it is not clear whether we shall consider as projected
 * second derivative the term $\nabla^2 u y_{KK'}$ or $y_{KK'}^T
 * \nabla^2 u$. Depending on which choice we take, we obtain one
 * approximation of the tensor of second derivatives or its
 * transpose. To avoid this ambiguity, as result we take the
 * symmetrized form, which is the mean value of the approximation and
 * its transpose.
 *
 * The returned value on each cell is the spectral norm of the
 * approximated tensor of second derivatives, i.e. the largest
 * eigenvalue by absolute value. This equals the largest curvature of
 * the finite element field at each cell, and the spectral norm is the
 * matrix norm associated to the $l_2$ vector norm.
 *
 * Even higher than the second derivative can be obtained along the
 * same lines as exposed above.
 *
 *
 * <h3>Refinement indicators based on the derivatives</h3>
 *
 * If you would like to base a refinement criterion upon these
 * approximation of the derivatives, you will have to scale the results
 * of this class by an appropriate power of the mesh width. For
 * example, since
 * $\|u-u_h\|^2_{L_2} \le C h^2 \|\nabla u\|^2_{L_2}$, it might be the
 * right thing to scale the indicators as $\eta_K = h \|\nabla u\|_K$,
 * i.e. $\eta_K = h^{1+d/2} \|\nabla u\|_{\infty;K}$, i.e. the right
 * power is $1+d/2$.
 *
 * Likewise, for the second derivative, one should choose a power of
 * the mesh size $h$ one higher than for the gradient.
 *
 *
 * <h3>Implementation</h3>
 *
 * The formulae for the computation of approximations to the gradient
 * and to the tensor of second derivatives shown above are very much
 * alike. The basic difference is that in one case the finite
 * difference quotiont is a scalar, while in the other case it is a
 * vector. For higher derivatives, this would be a tensor of even
 * higher rank. We then have to form the outer product of this
 * difference quotient with the distance vector $y_{KK'}$, symmetrize
 * it, contract it with the matrix $Y^{-1}$ and compute its norm. To
 * make the implementation simpler and to allow for code reuse, all
 * these operations that are dependent on the actual order of the
 * derivatives to be approximated, as well as the computation of the
 * quantities entering the difference quotient, have been separated
 * into auxiliary nested classes (names @p Gradient and
 * @p SecondDerivative) and the main algorithm is simply passed one
 * or the other data types and asks them to perform the order
 * dependent operations. The main framework that is independent of
 * this, such as finding all active neighbors, or setting up the
 * matrix $Y$ is done in the main function @p approximate.
 *
 * Due to this way of operation, the class may be easily extended for
 * higher oder derivatives than are presently implemented. Basically,
 * only an additional class along the lines of the derivative
 * descriptor classes @p Gradient and @p SecondDerivative has to be
 * implemented, with the respective typedefs and functions replaced by
 * the appropriate analogues for the derivative that is to be
 * approximated.
 *
 * @ingroup numerics
 * @author Wolfgang Bangerth, 2000
 */
class DerivativeApproximation
{
  public:
				     /**
				      * This function is used to
				      * obtain an approximation of the
				      * gradient. Pass it the DoF
				      * handler object that describes
				      * the finite element field, a
				      * nodal value vector, and
				      * receive the cell-wise
				      * Euclidian norm of the
				      * approximated gradient.
				      *
				      * The last parameter denotes the
				      * solution component, for which the
				      * gradient is to be computed. It
				      * defaults to the first component. For
				      * scalar elements, this is the only
				      * valid choice; for vector-valued ones,
				      * any component between zero and the
				      * number of vector components can be
				      * given here.
				      */
    template <int dim, template <int, int> class DH, class InputVector, int spacedim>
    static void
    approximate_gradient (const Mapping<dim,spacedim>    &mapping,
			  const DH<dim,spacedim>         &dof,
			  const InputVector     &solution,
			  Vector<float>         &derivative_norm,
			  const unsigned int     component = 0);

    				     /**
				      * Calls the @p interpolate
				      * function, see above, with
				      * <tt>mapping=MappingQ1@<dim@>()</tt>.
				      */
    template <int dim, template <int, int> class DH, class InputVector, int spacedim>
    static void
    approximate_gradient (const DH<dim,spacedim>         &dof,
			  const InputVector     &solution,
			  Vector<float>         &derivative_norm,
			  const unsigned int     component = 0);
    
				     /**
				      * This function is the analogue
				      * to the one above, computing
				      * finite difference
				      * approximations of the tensor
				      * of second derivatives. Pass it
				      * the DoF handler object that
				      * describes the finite element
				      * field, a nodal value vector,
				      * and receive the cell-wise
				      * spectral norm of the
				      * approximated tensor of second
				      * derivatives. The spectral norm
				      * is the matrix norm associated
				      * to the $l_2$ vector norm.
				      *
				      * The last parameter denotes the
				      * solution component, for which
				      * the gradient is to be
				      * computed. It defaults to the
				      * first component. For
				      * scalar elements, this is the only
				      * valid choice; for vector-valued ones,
				      * any component between zero and the
				      * number of vector components can be
				      * given here.
				      */
    template <int dim, template <int, int> class DH, class InputVector, int spacedim>
    static void
    approximate_second_derivative (const Mapping<dim,spacedim>    &mapping,
				   const DH<dim,spacedim>         &dof,
				   const InputVector     &solution,
				   Vector<float>         &derivative_norm,
				   const unsigned int     component = 0);
    
    				     /**
				      * Calls the @p interpolate
				      * function, see above, with
				      * <tt>mapping=MappingQ1@<dim@>()</tt>.
				      */
    template <int dim, template <int, int> class DH, class InputVector, int spacedim>
    static void
    approximate_second_derivative (const DH<dim,spacedim>         &dof,
				   const InputVector     &solution,
				   Vector<float>         &derivative_norm,
				   const unsigned int     component = 0);

				     /**
				      * This function calculates the
				      * <tt>order</tt>-th order approximate
				      * derivative and returns the full tensor
				      * for a single cell.
				      *
				      * The last parameter denotes the
				      * solution component, for which
				      * the gradient is to be
				      * computed. It defaults to the
				      * first component. For
				      * scalar elements, this is the only
				      * valid choice; for vector-valued ones,
				      * any component between zero and the
				      * number of vector components can be
				      * given here.
				      */

    template <int dim, template <int, int> class DH, class InputVector, int order, int spacedim>
    static void
    approximate_derivative_tensor (const Mapping<dim,spacedim>                           &mapping,
				   const DH<dim,spacedim>                                &dof,
				   const InputVector                            &solution,
				   const typename DH<dim,spacedim>::active_cell_iterator &cell,
				   Tensor<order,dim>                            &derivative,
				   const unsigned int                            component = 0);

				     /**
				      * Same as above, with
				      * <tt>mapping=MappingQ1@<dim@>()</tt>.
				      */

    template <int dim, template <int, int> class DH, class InputVector, int order, int spacedim>
    static void
    approximate_derivative_tensor (const DH<dim,spacedim>                                &dof,
				   const InputVector                            &solution,
				   const typename DH<dim,spacedim>::active_cell_iterator &cell,
				   Tensor<order,dim>                            &derivative,
				   const unsigned int                            component = 0);

				     /**
				      * Return the norm of the derivative.
				      */
    template <int dim, int order>
    static double
    derivative_norm(const Tensor<order,dim> &derivative);
    
				     /**
				      * Exception
				      */
    DeclException2 (ExcInvalidVectorLength,
		    int, int,
		    << "Vector has length " << arg1 << ", but should have "
		    << arg2);
				     /**
				      * Exception
				      */
    DeclException0 (ExcInsufficientDirections);

  private:

				     /**
				      * The following class is used to
				      * describe the data needed to
				      * compute the finite difference
				      * approximation to the gradient
				      * on a cell. See the general
				      * documentation of this class
				      * for more information on
				      * implementational details.
				      *
				      * @author Wolfgang Bangerth, 2000
				      */
    template <int dim>
    class Gradient 
    {
      public:
					 /**
					  * Declare which data fields have
					  * to be updated for the function
					  * @p get_projected_derivative
					  * to work.
					  */
	static const UpdateFlags update_flags;

					 /**
					  * Declare the data type which
					  * holds the derivative described
					  * by this class.
					  */
	typedef Tensor<1,dim> Derivative;

					 /**
					  * Likewise declare the data type
					  * that holds the derivative
					  * projected to a certain
					  * directions.
					  */
	typedef double        ProjectedDerivative;

					 /**
					  * Given an FEValues object
					  * initialized to a cell, and a
					  * solution vector, extract the
					  * desired derivative at the
					  * first quadrature point (which
					  * is the only one, as we only
					  * evaluate the finite element
					  * field at the center of each
					  * cell).
					  */
	template <class InputVector, int spacedim>
	static ProjectedDerivative
	get_projected_derivative (const FEValues<dim,spacedim>  &fe_values,
				  const InputVector    &solution,
				  const unsigned int    component);
    
					 /**
					  * Return the norm of the
					  * derivative object. Here, for
					  * the gradient, we choose the
					  * Euclidian norm of the gradient
					  * vector.
					  */
	static double derivative_norm (const Derivative &d);

					 /**
					  * If for the present derivative
					  * order, symmetrization of the
					  * derivative tensor is
					  * necessary, then do so on the
					  * argument.
					  *
					  * For the first derivatives, no
					  * such thing is necessary, so
					  * this function is a no-op.
					  */
	static void symmetrize (Derivative &derivative_tensor);
    };



				     /**
				      * The following class is used to
				      * describe the data needed to
				      * compute the finite difference
				      * approximation to the second
				      * derivatives on a cell. See the
				      * general documentation of this
				      * class for more information on
				      * implementational details.
				      *
				      * @author Wolfgang Bangerth, 2000
				      */
    template <int dim>
    class SecondDerivative
    {
      public:
					 /**
					  * Declare which data fields have
					  * to be updated for the function
					  * @p get_projected_derivative
					  * to work.
					  */
	static const UpdateFlags update_flags;

					 /**
					  * Declare the data type which
					  * holds the derivative described
					  * by this class.
					  */
	typedef Tensor<2,dim> Derivative;

					 /**
					  * Likewise declare the data type
					  * that holds the derivative
					  * projected to a certain
					  * directions.
					  */
	typedef Tensor<1,dim> ProjectedDerivative;

					 /**
					  * Given an FEValues object
					  * initialized to a cell, and a
					  * solution vector, extract the
					  * desired derivative at the
					  * first quadrature point (which
					  * is the only one, as we only
					  * evaluate the finite element
					  * field at the center of each
					  * cell).
					  */
	template <class InputVector, int spacedim>
	static ProjectedDerivative
	get_projected_derivative (const FEValues<dim,spacedim>  &fe_values,
				  const InputVector    &solution,
				  const unsigned int    component);
	
					 /**
					  * Return the norm of the
					  * derivative object. Here, for
					  * the (symmetric) tensor of
					  * second derivatives, we choose
					  * the absolute value of the
					  * largest eigenvalue, which is
					  * the matrix norm associated to
					  * the $l_2$ norm of vectors. It
					  * is also the largest value of
					  * the curvature of the solution.
					  */
	static double derivative_norm (const Derivative &d);

					 /**
					  * If for the present derivative
					  * order, symmetrization of the
					  * derivative tensor is
					  * necessary, then do so on the
					  * argument.
					  *
					  * For the second derivatives,
					  * each entry of the tensor is
					  * set to the mean of its value
					  * and the value of the transpose
					  * element.
					  *
					  * Note that this function
					  * actually modifies its
					  * argument.
					  */
	static void symmetrize (Derivative &derivative_tensor);
    };

    template <int dim>
    class ThirdDerivative
    {
      public:
					 /**
					  * Declare which data fields have
					  * to be updated for the function
					  * @p get_projected_derivative
					  * to work.
					  */
	static const UpdateFlags update_flags;

					 /**
					  * Declare the data type which
					  * holds the derivative described
					  * by this class.
					  */
	typedef Tensor<3,dim> Derivative;

					 /**
					  * Likewise declare the data type
					  * that holds the derivative
					  * projected to a certain
					  * directions.
					  */
	typedef Tensor<2,dim> ProjectedDerivative;

					 /**
					  * Given an FEValues object
					  * initialized to a cell, and a
					  * solution vector, extract the
					  * desired derivative at the
					  * first quadrature point (which
					  * is the only one, as we only
					  * evaluate the finite element
					  * field at the center of each
					  * cell).
					  */
	template <class InputVector, int spacedim>
	static ProjectedDerivative
	get_projected_derivative (const FEValues<dim,spacedim>  &fe_values,
				  const InputVector    &solution,
				  const unsigned int    component);
	
					 /**
					  * Return the norm of the
					  * derivative object. Here, for
					  * the (symmetric) tensor of
					  * second derivatives, we choose
					  * the absolute value of the
					  * largest eigenvalue, which is
					  * the matrix norm associated to
					  * the $l_2$ norm of vectors. It
					  * is also the largest value of
					  * the curvature of the solution.
					  */
	static double derivative_norm (const Derivative &d);

					 /**
					  * If for the present derivative
					  * order, symmetrization of the
					  * derivative tensor is
					  * necessary, then do so on the
					  * argument.
					  *
					  * For the second derivatives,
					  * each entry of the tensor is
					  * set to the mean of its value
					  * and the value of the transpose
					  * element.
					  *
					  * Note that this function
					  * actually modifies its
					  * argument.
					  */
	static void symmetrize (Derivative &derivative_tensor);
    };
    
    template <int order, int dim>
    class DerivativeSelector
    {
      public:
					 /**
					  * typedef to select the
					  * DerivativeDescription corresponding
					  * to the <tt>order</tt>th
					  * derivative. In this general template
					  * we set an unvalid typedef to void,
					  * the real typedefs have to be
					  * specialized.
					  */
	typedef void DerivDescr;
	
    };

    template <int dim>
    class DerivativeSelector<1,dim>
    {
      public:

	typedef Gradient<dim> DerivDescr;
    };
    
    template <int dim>
    class DerivativeSelector<2,dim>
    {
      public:
	
	typedef SecondDerivative<dim> DerivDescr;
    };
    
    template <int dim>
    class DerivativeSelector<3,dim>
    {
      public:
	
	typedef ThirdDerivative<dim> DerivDescr;
    };
    
    
    
    
  private:
    
				     /**
				      * Convenience typedef denoting
				      * the range of indices on which
				      * a certain thread shall
				      * operate.
				      */
    typedef std::pair<unsigned int,unsigned int> IndexInterval;

				     /**
				      * Kind of the main function of
				      * this class. It is called by
				      * the public entry points to
				      * this class with the correct
				      * template first argument and
				      * then simply calls the
				      * @p approximate function,
				      * after setting up several
				      * threads and doing some
				      * administration that is
				      * independent of the actual
				      * derivative to be computed.
				      *
				      * The @p component argument
				      * denotes which component of the
				      * solution vector we are to work
				      * on.
				      */
    template <class DerivativeDescription, int dim,
              template <int, int> class DH, class InputVector, int spacedim>
    static void
    approximate_derivative (const Mapping<dim,spacedim>    &mapping,
			    const DH<dim,spacedim>         &dof,
			    const InputVector     &solution,
			    const unsigned int     component,
			    Vector<float>         &derivative_norm);

				     /**
				      * Compute the derivative
				      * approximation on the cells in
				      * the range given by the third
				      * parameter.
				      * Fill the @p derivative_norm vector with
				      * the norm of the computed derivative
				      * tensors on each cell.
				      */
    template <class DerivativeDescription, int dim,
              template <int, int> class DH, class InputVector, int spacedim>
    static void
    approximate (const Mapping<dim,spacedim>    &mapping,
		 const DH<dim,spacedim>         &dof,
		 const InputVector     &solution,
		 const unsigned int     component,
		 const IndexInterval   &index_interval,
		 Vector<float>         &derivative_norm);    

				     /**
				      * Compute the derivative approximation on
				      * one cell. This computes the full
				      * derivative tensor.
				      */
    template <class DerivativeDescription, int dim,
              template <int, int> class DH, class InputVector, int spacedim>
    static void
    approximate_cell (const Mapping<dim,spacedim>                            &mapping,
		      const DH<dim,spacedim>                                 &dof,
		      const InputVector                             &solution,
		      const unsigned int                             component,
		      const typename DH<dim,spacedim>::active_cell_iterator  &cell,
		      typename DerivativeDescription::Derivative    &derivative);    
};


/* -------------- declaration of explicit specializations ------------- */

template <>
double
DerivativeApproximation::SecondDerivative<1>::derivative_norm (const Derivative &d);

template <>
double
DerivativeApproximation::SecondDerivative<2>::derivative_norm (const Derivative &d);

template <>
double
DerivativeApproximation::SecondDerivative<3>::derivative_norm (const Derivative &d);


DEAL_II_NAMESPACE_CLOSE

#endif
