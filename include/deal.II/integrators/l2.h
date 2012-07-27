//---------------------------------------------------------------------------
//    $Id: l2.h 24271 2011-09-06 15:56:41Z kanschat $
//
//    Copyright (C) 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__integrators_l2_h
#define __deal2__integrators_l2_h


#include <deal.II/base/config.h>
#include <deal.II/base/exceptions.h>
#include <deal.II/base/quadrature.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/fe/mapping.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/meshworker/dof_info.h>

DEAL_II_NAMESPACE_OPEN

namespace LocalIntegrators
{
/**
 * @brief Local integrators related to <i>L<sup>2</sup></i>-inner products.
 *
 * @ingroup Integrators
 * @author Guido Kanschat
 * @date 2010
 */
  namespace L2
  {
/**
 * The mass matrix for scalar or vector values finite elements.
 * \f[
 * \int_Z uv\,dx \quad \text{or} \quad \int_Z \mathbf u\cdot \mathbf v\,dx
 * \f]
 *
 * Likewise, this term can be used on faces, where it computes  the integrals
 * \f[
 * \int_F uv\,ds \quad \text{or} \quad \int_F \mathbf u\cdot \mathbf v\,ds
 * \f]
 *
 * @ingroup Integrators
 * @author Guido Kanschat
 * @date 2008, 2009, 2010
 */
    template <int dim>
    void mass_matrix (
      FullMatrix<double>& M,
      const FEValuesBase<dim>& fe,
      const double factor = 1.)
    {
      const unsigned int n_dofs = fe.dofs_per_cell;
      const unsigned int n_components = fe.get_fe().n_components();
      
      for (unsigned k=0;k<fe.n_quadrature_points;++k)
	{
	  const double dx = fe.JxW(k) * factor;
	  
	  for (unsigned i=0;i<n_dofs;++i)
	    for (unsigned j=0;j<n_dofs;++j)
	      for (unsigned int d=0;d<n_components;++d)
		M(i,j) += dx
			  * fe.shape_value_component(j,k,d)
			  * fe.shape_value_component(i,k,d);
	}
    }

/**
 * <i>L<sup>2</sup></i>-inner product for scalar functions.
 *
 * \f[
 * \int_Z fv\,dx \quad \text{or} \quad \int_F fv\,ds
 * \f]
 */
    template <int dim>
    void L2 (
      Vector<double>& result,
      const FEValuesBase<dim>& fe,
      const std::vector<double>& input,
      const double factor = 1.)
    {
      const unsigned int n_dofs = fe.dofs_per_cell;
      AssertDimension(result.size(), n_dofs);
      AssertDimension(fe.get_fe().n_components(), 1);
      AssertDimension(input.size(), fe.n_quadrature_points);
      
      for (unsigned k=0;k<fe.n_quadrature_points;++k)
	for (unsigned i=0;i<n_dofs;++i)
	  result(i) += fe.JxW(k) * factor * input[k] * fe.shape_value(i,k);
    }

/**
 * <i>L<sup>2</sup></i>-inner product for a slice of a vector valued
 * right hand side.
 * \f[
 * \int_Z \mathbf f\cdot \mathbf v\,dx
 * \quad \text{or} \quad
 * \int_F \mathbf f\cdot \mathbf v\,ds
 * \f]
 *
 * @ingroup Integrators
 * @author Guido Kanschat
 * @date 2008, 2009, 2010
 */
    template <int dim>
    void L2 (
      Vector<double>& result,
      const FEValuesBase<dim>& fe,
      const VectorSlice<const std::vector<std::vector<double> > >& input,
      const double factor = 1.)
    {
      const unsigned int n_dofs = fe.dofs_per_cell;
      const unsigned int fe_components = fe.get_fe().n_components();
      const unsigned int n_components = input.size();
	
      AssertDimension(result.size(), n_dofs);
      AssertDimension(input.size(), fe_components);
	
      for (unsigned k=0;k<fe.n_quadrature_points;++k)
	for (unsigned i=0;i<n_dofs;++i)
	  for (unsigned int d=0;d<n_components;++d)
	    result(i) += fe.JxW(k) * factor * fe.shape_value_component(i,k,d) * input[d][k];
    }

/**
 * The jump matrix between two cells for scalar or vector values
 * finite elements. Note that the factor $\gamma$ can be used to
 * implement weighted jumps.
 * \f[
 * \int_F [\gamma u][\gamma v]\,ds
 * \quad \text{or}
 * \int_F [\gamma \mathbf u]\cdot [\gamma \mathbf v]\,ds
 * \f]
 *
 * Using appropriate weights, this term can be used to penalize
 * violation of conformity in <i>H<sup>1</sup></i>.
 *
 * @ingroup Integrators
 * @author Guido Kanschat
 * @date 2008, 2009, 2010
 */
    template <int dim>
    void jump_matrix (
      FullMatrix<double>& M11,
      FullMatrix<double>& M12,
      FullMatrix<double>& M21,
      FullMatrix<double>& M22,
      const FEValuesBase<dim>& fe1,
      const FEValuesBase<dim>& fe2,
      const double factor1 = 1.,
      const double factor2 = 1.)
    {
      const unsigned int n1_dofs = fe1.dofs_per_cell;
      const unsigned int n2_dofs = fe2.dofs_per_cell;
      const unsigned int n_components = fe1.get_fe().n_components();
      
      Assert(n1_dofs == n2_dofs, ExcNotImplemented());
      AssertDimension(n_components, fe2.get_fe().n_components());
      AssertDimension(M11.m(), n1_dofs);
      AssertDimension(M12.m(), n1_dofs);
      AssertDimension(M21.m(), n2_dofs);
      AssertDimension(M22.m(), n2_dofs);
      AssertDimension(M11.n(), n1_dofs);
      AssertDimension(M12.n(), n2_dofs);
      AssertDimension(M21.n(), n1_dofs);
      AssertDimension(M22.n(), n2_dofs);
      
      for (unsigned k=0;k<fe1.n_quadrature_points;++k)
	{
	  const double dx = fe1.JxW(k);
	  
	  for (unsigned i=0;i<n1_dofs;++i)
	    for (unsigned j=0;j<n1_dofs;++j)
	      for (unsigned int d=0;d<n_components;++d)
		{
		  const double u1 = factor1*fe1.shape_value_component(j,k,d);
		  const double u2 =-factor2*fe2.shape_value_component(j,k,d);
		  const double v1 = factor1*fe1.shape_value_component(i,k,d);
		  const double v2 =-factor2*fe2.shape_value_component(i,k,d);		  
		  
		  M11(i,j) += dx * u1*v1;
		  M12(i,j) += dx * u2*v1;
		  M21(i,j) += dx * u1*v2;
		  M22(i,j) += dx * u2*v2;
		}
	}
    }
  }
}

DEAL_II_NAMESPACE_CLOSE

#endif