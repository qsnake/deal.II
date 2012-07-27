//---------------------------------------------------------------------------
//    $Id: tensor_function.h 23876 2011-06-28 18:21:51Z kanschat $
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__tensor_function_h
#define __deal2__tensor_function_h


#include <deal.II/base/config.h>
#include <deal.II/base/exceptions.h>
#include <deal.II/base/subscriptor.h>
#include <deal.II/base/smartpointer.h>
#include <deal.II/base/function.h>
#include <deal.II/base/point.h>
#include <deal.II/base/function_time.h>

#include <vector>

DEAL_II_NAMESPACE_OPEN

/**
 *  This class is a model for a tensor valued function. The interface
 *  of the class is mostly the same as that for the Function
 *  class, with the exception that it does not support vector-valued
 *  functions with several components, but that the return type is
 *  always tensor-valued. The returned values of the evaluation of
 *  objects of this type are always whole tensors, while for the
 *  <tt>Function</tt> class, one can ask for a specific component only, or
 *  use the <tt>vector_value</tt> function, which however does not return
 *  the value, but rather writes it into the address provided by its
 *  second argument. The reason for the different behaviour of the
 *  classes is that in the case if tensor valued functions, the size
 *  of the argument is known to the compiler a priori, such that the
 *  correct amount of memory can be allocated on the stack for the
 *  return value; on the other hand, for the vector valued functions,
 *  the size is not known to the compiler, so memory has to be
 *  allocated on the heap, resulting in relatively expensive copy
 *  operations. One can therefore consider this class a specialization
 *  of the <tt>Function</tt> class for which the size is known. An
 *  additional benefit is that tensors of arbitrary rank can be
 *  returned, not only vectors, as for them the size can be determined
 *  similarly simply.
 *
 *  @ingroup functions
 *  @author Guido Kanschat, 1999 
 */
template <int rank, int dim>
class TensorFunction : public FunctionTime,
		       public Subscriptor
{
  public:
				     /**
				      * Define typedefs for the return
				      * types of the <tt>value</tt>
				      * functions.
				      */
    typedef Tensor<rank,dim> value_type;

#ifdef DEAL_II_LOCAL_TYPEDEF_COMP_WORKAROUND
				     /**
				      * Define a typedef for the
				      * return value of the gradient
				      * function.
				      *
				      * The construct here is only
				      * used in case we hit a certain
				      * bug in Sun's Forte
				      * compiler. See the respective
				      * macro in the aclocal.m4 file
				      * for a full description of the
				      * bug, or the documentation to
				      * the quadrature class.
				      *
				      * For better readability we
				      * later typedef this so-created
				      * type to one in the enclosing
				      * class.
				      */
    template <int dim2>
    struct GradientTypeHelper
    {
	typedef Tensor<rank+1,dim> type;
    };

				     /**
				      * Typedef the kludge declared
				      * above to a type in the class
				      * in which we would like to use
				      * it.
				      *
				      * This typedef is only used if
				      * the respective bug in the
				      * compiler is encountered,
				      * otherwise the proper typedef
				      * below is used.
				      */
    typedef typename GradientTypeHelper<dim>::type gradient_type;
#else
    typedef Tensor<rank+1,dim> gradient_type;
#endif    
    
				     /**
				      * Constructor. May take an
				      * initial value for the time
				      * variable, which defaults to
				      * zero.  
				      */
    TensorFunction (const double initial_time = 0.0);
    
				     /**
				      * Virtual destructor; absolutely
				      * necessary in this case, as
				      * classes are usually not used
				      * by their true type, but rather
				      * through pointers to this base
				      * class.  
				      */
    virtual ~TensorFunction ();
    
				     /**
				      * Return the value of the function
				      * at the given point.
				      */
    virtual value_type value (const Point<dim> &p) const;

				     /**
				      * Set <tt>values</tt> to the point
				      * values of the function at the
				      * <tt>points</tt>.  It is assumed
				      * that <tt>values</tt> already has
				      * the right size, i.e.  the same
				      * size as the <tt>points</tt> array.  
				      */
    virtual void value_list (const std::vector<Point<dim> > &points,
			     std::vector<value_type> &values) const;

				     /**
				      * Return the gradient of the
				      * function at the given point.
				      */
    virtual gradient_type gradient (const Point<dim> &p) const;

				     /**
				      * Set <tt>gradients</tt> to the
				      * gradients of the function at
				      * the <tt>points</tt>.  It is assumed
				      * that <tt>values</tt> already has
				      * the right size, i.e.  the same
				      * size as the <tt>points</tt> array.  
				      */
    virtual void gradient_list (const std::vector<Point<dim> >   &points,
				std::vector<gradient_type> &gradients) const;

				     /**
				      * Exception
				      */
    DeclException0 (ExcPureFunctionCalled);
				     /**
				      * Exception
				      */
    DeclException2 (ExcVectorHasWrongSize,
		    int, int,
		    << "The vector has size " << arg1 << " but should have "
		    << arg2 << " elements.");
    
};

DEAL_II_NAMESPACE_CLOSE

#endif
