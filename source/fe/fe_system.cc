//---------------------------------------------------------------------------
//    $Id: fe_system.cc 24072 2011-08-15 12:19:02Z kanschat $
//    Version: $Name$
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <deal.II/base/memory_consumption.h>
#include <deal.II/base/quadrature.h>
#include <deal.II/base/qprojector.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/dofs/dof_accessor.h>
#include <deal.II/fe/mapping.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/fe/fe_values.h>

#include <sstream>


DEAL_II_NAMESPACE_OPEN

/* ----------------------- FESystem::InternalData ------------------- */


template <int dim, int spacedim>
FESystem<dim,spacedim>::InternalData::InternalData(const unsigned int n_base_elements):
		base_fe_datas(n_base_elements),
		base_fe_values_datas(n_base_elements)
{}



template <int dim, int spacedim>
FESystem<dim,spacedim>::InternalData::~InternalData()
{
				   // delete pointers and set them to
				   // zero to avoid inadvertant use
  for (unsigned int i=0; i<base_fe_datas.size(); ++i)
    if (base_fe_datas[i])
      {
	delete base_fe_datas[i];
	base_fe_datas[i] = 0;
      };

  for (unsigned int i=0; i<base_fe_values_datas.size(); ++i)
    if (base_fe_values_datas[i])
      {
	delete base_fe_values_datas[i];
	base_fe_values_datas[i] = 0;
      };
}



template <int dim, int spacedim>
typename FiniteElement<dim,spacedim>::InternalDataBase &
FESystem<dim,spacedim>::
InternalData::get_fe_data (const unsigned int base_no) const
{
  Assert(base_no<base_fe_datas.size(),
	 ExcIndexRange(base_no,0,base_fe_datas.size()));
  return *base_fe_datas[base_no];
}



template <int dim, int spacedim>
void
FESystem<dim,spacedim>::
InternalData::set_fe_data (const unsigned int base_no,
			   typename FiniteElement<dim,spacedim>::InternalDataBase *ptr)
{
  Assert(base_no<base_fe_datas.size(),
	 ExcIndexRange(base_no,0,base_fe_datas.size()));
  base_fe_datas[base_no]=ptr;
}



template <int dim, int spacedim>
FEValuesData<dim,spacedim> &
FESystem<dim,spacedim>::
InternalData::get_fe_values_data (const unsigned int base_no) const
{
  Assert(base_no<base_fe_values_datas.size(),
	 ExcIndexRange(base_no,0,base_fe_values_datas.size()));
  Assert(base_fe_values_datas[base_no]!=0, ExcInternalError());
  return *base_fe_values_datas[base_no];
}



template <int dim, int spacedim>
void
FESystem<dim,spacedim>::
InternalData::set_fe_values_data (const unsigned int base_no,
				  FEValuesData<dim,spacedim> *ptr)
{
  Assert(base_no<base_fe_values_datas.size(),
	 ExcIndexRange(base_no,0,base_fe_values_datas.size()));
  base_fe_values_datas[base_no]=ptr;
}



template <int dim, int spacedim>
void
FESystem<dim,spacedim>::
InternalData::delete_fe_values_data (const unsigned int base_no)
{
  Assert(base_no<base_fe_values_datas.size(),
	 ExcIndexRange(base_no,0,base_fe_values_datas.size()));
  Assert(base_fe_values_datas[base_no]!=0, ExcInternalError());
  delete base_fe_values_datas[base_no];
  base_fe_values_datas[base_no]=0;
}



template <int dim, int spacedim>
void
FESystem<dim,spacedim>::InternalData::clear_first_cell ()
{
                                   // call respective function of base
                                   // class
  FiniteElement<dim,spacedim>::InternalDataBase::clear_first_cell ();
                                   // then the functions of all the
                                   // sub-objects
  for (unsigned int i=0; i<base_fe_datas.size(); ++i)
    base_fe_datas[i]->clear_first_cell ();
}


/* ---------------------------------- FESystem ------------------- */


template <int dim, int spacedim>
const unsigned int FESystem<dim,spacedim>::invalid_face_number;


template <int dim, int spacedim>
FESystem<dim,spacedim>::FESystem (const FiniteElement<dim,spacedim> &fe,
				  const unsigned int n_elements) :
		FiniteElement<dim,spacedim> (multiply_dof_numbers(fe, n_elements),
				    compute_restriction_is_additive_flags (fe, n_elements),
				    compute_nonzero_components(fe, n_elements)),
                base_elements(1)
{
  this->base_to_block_indices.reinit(0, 0);
  this->base_to_block_indices.push_back(n_elements);
  base_elements[0] = ElementPair(fe.clone(), n_elements);
  base_elements[0].first->subscribe (typeid(*this).name());
  initialize ();
}



template <int dim, int spacedim>
FESystem<dim,spacedim>::FESystem (const FiniteElement<dim,spacedim> &fe1,
				  const unsigned int        n1,
				  const FiniteElement<dim,spacedim> &fe2,
				  const unsigned int        n2) :
		FiniteElement<dim,spacedim> (multiply_dof_numbers(fe1, n1, fe2, n2),
				    compute_restriction_is_additive_flags (fe1, n1,
									   fe2, n2),
				    compute_nonzero_components(fe1, n1,
							       fe2, n2)),
                base_elements(2)
{
  this->base_to_block_indices.reinit(0, 0);
  this->base_to_block_indices.push_back(n1);
  this->base_to_block_indices.push_back(n2);
  base_elements[0] = ElementPair(fe1.clone(), n1);
  base_elements[0].first->subscribe (typeid(*this).name());
  base_elements[1] = ElementPair(fe2.clone(), n2);
  base_elements[1].first->subscribe (typeid(*this).name());
  initialize ();
}



template <int dim, int spacedim>
FESystem<dim,spacedim>::FESystem (const FiniteElement<dim,spacedim> &fe1,
				  const unsigned int        n1,
				  const FiniteElement<dim,spacedim> &fe2,
				  const unsigned int        n2,
				  const FiniteElement<dim,spacedim> &fe3,
				  const unsigned int        n3) :
       FiniteElement<dim,spacedim> (multiply_dof_numbers(fe1, n1,
							 fe2, n2,
							 fe3, n3),
				    compute_restriction_is_additive_flags (fe1, n1,
									   fe2, n2,
									   fe3, n3),
				    compute_nonzero_components(fe1, n1,
							       fe2, n2,
							       fe3, n3)),
                base_elements(3)
{
  this->base_to_block_indices.reinit(0, 0);
  this->base_to_block_indices.push_back(n1);
  this->base_to_block_indices.push_back(n2);
  this->base_to_block_indices.push_back(n3);
  base_elements[0] = ElementPair(fe1.clone(), n1);
  base_elements[0].first->subscribe (typeid(*this).name());
  base_elements[1] = ElementPair(fe2.clone(), n2);
  base_elements[1].first->subscribe (typeid(*this).name());
  base_elements[2] = ElementPair(fe3.clone(), n3);
  base_elements[2].first->subscribe (typeid(*this).name());
  initialize ();
}


template <int dim, int spacedim>
FESystem<dim,spacedim>::FESystem (const FiniteElement<dim,spacedim> &fe1,
				  const unsigned int        n1,
				  const FiniteElement<dim,spacedim> &fe2,
				  const unsigned int        n2,
				  const FiniteElement<dim,spacedim> &fe3,
				  const unsigned int        n3,
				  const FiniteElement<dim,spacedim> &fe4,
				  const unsigned int        n4) :
  FiniteElement<dim,spacedim> (multiply_dof_numbers(fe1, n1,
						    fe2, n2,
						    fe3, n3,
						    fe4, n4),
			       compute_restriction_is_additive_flags (fe1, n1,
								      fe2, n2,
								      fe3, n3,
								      fe4, n4),
			       compute_nonzero_components(fe1, n1,
							  fe2, n2,
							  fe3, n3,
							  fe4 ,n4)),
  base_elements(4)
{
  this->base_to_block_indices.reinit(0, 0);
  this->base_to_block_indices.push_back(n1);
  this->base_to_block_indices.push_back(n2);
  this->base_to_block_indices.push_back(n3);
  this->base_to_block_indices.push_back(n4);
  base_elements[0] = ElementPair(fe1.clone(), n1);
  base_elements[0].first->subscribe (typeid(*this).name());
  base_elements[1] = ElementPair(fe2.clone(), n2);
  base_elements[1].first->subscribe (typeid(*this).name());
  base_elements[2] = ElementPair(fe3.clone(), n3);
  base_elements[2].first->subscribe (typeid(*this).name());
  base_elements[3] = ElementPair(fe4.clone(), n4);
  base_elements[3].first->subscribe (typeid(*this).name());
  initialize ();
}

template <int dim, int spacedim>
FESystem<dim,spacedim>::FESystem (const FiniteElement<dim,spacedim> &fe1,
				  const unsigned int        n1,
				  const FiniteElement<dim,spacedim> &fe2,
				  const unsigned int        n2,
				  const FiniteElement<dim,spacedim> &fe3,
				  const unsigned int        n3,
				  const FiniteElement<dim,spacedim> &fe4,
				  const unsigned int        n4,
				  const FiniteElement<dim,spacedim> &fe5,
				  const unsigned int        n5) :
  FiniteElement<dim,spacedim> (multiply_dof_numbers(fe1, n1,
						    fe2, n2,
						    fe3, n3,
						    fe4, n4,
						    fe5, n5),
			       compute_restriction_is_additive_flags (fe1, n1,
								      fe2, n2,
								      fe3, n3,
								      fe4, n4,
								      fe5, n5),
			       compute_nonzero_components(fe1, n1,
							  fe2, n2,
							  fe3, n3,
							  fe4 ,n4,
							  fe5, n5)),
  base_elements(5)
{
  this->base_to_block_indices.reinit(0, 0);
  this->base_to_block_indices.push_back(n1);
  this->base_to_block_indices.push_back(n2);
  this->base_to_block_indices.push_back(n3);
  this->base_to_block_indices.push_back(n4);
  this->base_to_block_indices.push_back(n5);
  
  base_elements[0] = ElementPair(fe1.clone(), n1);
  base_elements[0].first->subscribe (typeid(*this).name());
  base_elements[1] = ElementPair(fe2.clone(), n2);
  base_elements[1].first->subscribe (typeid(*this).name());
  base_elements[2] = ElementPair(fe3.clone(), n3);
  base_elements[2].first->subscribe (typeid(*this).name());
  base_elements[3] = ElementPair(fe4.clone(), n4);
  base_elements[3].first->subscribe (typeid(*this).name());
  base_elements[4] = ElementPair(fe5.clone(), n5);
  base_elements[4].first->subscribe (typeid(*this).name());
  initialize ();
}


template <int dim, int spacedim>
FESystem<dim,spacedim>::~FESystem ()
{
				   // delete base elements created in
				   // the constructor
  for (unsigned i=0; i<base_elements.size(); ++i)
    {
      base_elements[i].first->unsubscribe(typeid(*this).name());
      delete base_elements[i].first;
      base_elements[i].first = 0;
    }
}



template <int dim, int spacedim>
std::string
FESystem<dim,spacedim>::get_name () const
{
				   // note that the
				   // FETools::get_fe_from_name
				   // function depends on the
				   // particular format of the string
				   // this function returns, so they
				   // have to be kept in synch

  std::ostringstream namebuf;

  namebuf << "FESystem<" << dim << ">[";
  for (unsigned int i=0; i< this->n_base_elements(); ++i)
    {
      namebuf << base_element(i).get_name();
      if (this->element_multiplicity(i) != 1)
	namebuf << '^' << this->element_multiplicity(i);
      if (i != this->n_base_elements()-1)
	namebuf << '-';
    }
  namebuf << ']';

  return namebuf.str();
}



template <int dim, int spacedim>
FiniteElement<dim,spacedim>*
FESystem<dim,spacedim>::clone() const
{
  switch (this->n_base_elements())
    {
    case 1:
      return new FESystem(base_element(0),
			  this->element_multiplicity(0));
    case 2:
      return new FESystem(base_element(0),
			  this->element_multiplicity(0),
			  base_element(1),
			  this->element_multiplicity(1));
    case 3:
      return new FESystem(base_element(0),
			  this->element_multiplicity(0),
			  base_element(1),
			  this->element_multiplicity(1),
			  base_element(2),
			  this->element_multiplicity(2));
    case 4:
      return new FESystem(base_element(0),
			  this->element_multiplicity(0),
			  base_element(1),
			  this->element_multiplicity(1),
			  base_element(2),
			  this->element_multiplicity(2),
			  base_element(3),
			  this->element_multiplicity(3));
    case 5:
      return new FESystem(base_element(0),
			  this->element_multiplicity(0),
			  base_element(1),
			  this->element_multiplicity(1),
			  base_element(2),
			  this->element_multiplicity(2),
			  base_element(3),
			  this->element_multiplicity(3),
			  base_element(4),
			  this->element_multiplicity(4));
      default:
			  Assert(false, ExcNotImplemented());
    }
  return 0;
}



template <int dim, int spacedim>
double
FESystem<dim,spacedim>::shape_value (const unsigned int i,
				     const Point<dim> &p) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i, 0, this->dofs_per_cell));
  Assert (this->is_primitive(i),
	  (typename FiniteElement<dim,spacedim>::ExcShapeFunctionNotPrimitive(i)));

  return (base_element(this->system_to_base_table[i].first.first)
	  .shape_value(this->system_to_base_table[i].second, p));
}



template <int dim, int spacedim>
double
FESystem<dim,spacedim>::shape_value_component (const unsigned int i,
					       const Point<dim>  &p,
					       const unsigned int component) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i, 0, this->dofs_per_cell));
  Assert (component < this->n_components(),
	  ExcIndexRange (component, 0, this->n_components()));

                                   // if this value is supposed to be
                                   // zero, then return right away...
  if (this->nonzero_components[i][component] == false)
    return 0;

                                   // ...otherwise: first find out to
				   // which of the base elements this
				   // desired component belongs, and
				   // which component within this base
				   // element it is
  const unsigned int base              = this->component_to_base_index(component).first;
  const unsigned int component_in_base = this->component_to_base_index(component).second;

				   // then get value from base
				   // element. note that that will
				   // throw an error should the
				   // respective shape function not be
				   // primitive; thus, there is no
				   // need to check this here
  return (base_element(base).
	  shape_value_component(this->system_to_base_table[i].second,
				p,
				component_in_base));
}



template <int dim, int spacedim>
Tensor<1,dim>
FESystem<dim,spacedim>::shape_grad (const unsigned int i,
				    const Point<dim> &p) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i, 0, this->dofs_per_cell));
  Assert (this->is_primitive(i),
	  (typename FiniteElement<dim,spacedim>::ExcShapeFunctionNotPrimitive(i)));

  return (base_element(this->system_to_base_table[i].first.first)
	  .shape_grad(this->system_to_base_table[i].second, p));
}



template <int dim, int spacedim>
Tensor<1,dim>
FESystem<dim,spacedim>::shape_grad_component (const unsigned int i,
				     const Point<dim>  &p,
				     const unsigned int component) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i, 0, this->dofs_per_cell));
  Assert (component < this->n_components(),
	  ExcIndexRange (component, 0, this->n_components()));

                                   // if this value is supposed to be
                                   // zero, then return right away...
  if (this->nonzero_components[i][component] == false)
    return Tensor<1,dim>();

                                   // ...otherwise: first find out to
    				   // which of the base elements this
    				   // desired component belongs, and
    				   // which component within this base
    				   // element it is
  const unsigned int base              = this->component_to_base_index(component).first;
  const unsigned int component_in_base = this->component_to_base_index(component).second;

				   // then get value from base
				   // element. note that that will
				   // throw an error should the
				   // respective shape function not be
				   // primitive; thus, there is no
				   // need to check this here
  return (base_element(base).
	  shape_grad_component(this->system_to_base_table[i].second,
			       p,
			       component_in_base));
}



template <int dim, int spacedim>
Tensor<2,dim>
FESystem<dim,spacedim>::shape_grad_grad (const unsigned int i,
					 const Point<dim> &p) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i, 0, this->dofs_per_cell));
  Assert (this->is_primitive(i),
	  (typename FiniteElement<dim,spacedim>::ExcShapeFunctionNotPrimitive(i)));

  return (base_element(this->system_to_base_table[i].first.first)
	  .shape_grad_grad(this->system_to_base_table[i].second, p));
}



template <int dim, int spacedim>
Tensor<2,dim>
FESystem<dim,spacedim>::shape_grad_grad_component (const unsigned int i,
					  const Point<dim>  &p,
					  const unsigned int component) const
{
  Assert (i<this->dofs_per_cell, ExcIndexRange(i, 0, this->dofs_per_cell));
  Assert (component < this->n_components(),
	  ExcIndexRange (component, 0, this->n_components()));

                                   // if this value is supposed to be
                                   // zero, then return right away...
  if (this->nonzero_components[i][component] == false)
    return Tensor<2,dim>();

                                   // ...otherwise: first find out to
				   // which of the base elements this
				   // desired component belongs, and
				   // which component within this base
				   // element it is
  const unsigned int base              = this->component_to_base_index(component).first;
  const unsigned int component_in_base = this->component_to_base_index(component).second;

				   // then get value from base
				   // element. note that that will
				   // throw an error should the
				   // respective shape function not be
				   // primitive; thus, there is no
				   // need to check this here
  return (base_element(base).
	  shape_grad_grad_component(this->system_to_base_table[i].second,
				    p,
				    component_in_base));
}



template <int dim, int spacedim>
void
FESystem<dim,spacedim>::get_interpolation_matrix (
  const FiniteElement<dim,spacedim> &x_source_fe,
  FullMatrix<double>           &interpolation_matrix) const
{
  Assert (interpolation_matrix.m() == this->dofs_per_cell,
	  ExcDimensionMismatch (interpolation_matrix.m(),
				this->dofs_per_cell));
  Assert (interpolation_matrix.n() == x_source_fe.dofs_per_cell,
	  ExcDimensionMismatch (interpolation_matrix.m(),
				x_source_fe.dofs_per_cell));

                                   // there are certain conditions
                                   // that the two elements have to
                                   // satisfy so that this can work.
                                   //
                                   // condition 1: the other element
                                   // must also be a system element

  typedef FiniteElement<dim,spacedim> FEL;
  AssertThrow ((x_source_fe.get_name().find ("FESystem<") == 0)
               ||
               (dynamic_cast<const FESystem<dim,spacedim>*>(&x_source_fe) != 0),
               typename FEL::
               ExcInterpolationNotImplemented());

				   // ok, source is a system element,
				   // so we may be able to do the work
  const FESystem<dim,spacedim> &source_fe
    = dynamic_cast<const FESystem<dim,spacedim>&>(x_source_fe);

                                   // condition 2: same number of
                                   // basis elements
  AssertThrow (this->n_base_elements() == source_fe.n_base_elements(),
               typename FEL::
               ExcInterpolationNotImplemented());

                                   // condition 3: same number of
                                   // basis elements
  for (unsigned int i=0; i< this->n_base_elements(); ++i)
    AssertThrow (this->element_multiplicity(i) ==
                 source_fe.element_multiplicity(i),
                 typename FEL::
                 ExcInterpolationNotImplemented());

                                   // ok, so let's try whether it
                                   // works:

                                   // first let's see whether all the
                                   // basis elements actually generate
                                   // their interpolation matrices. if
                                   // we get past the following loop,
                                   // then apparently none of the
                                   // called base elements threw an
                                   // exception, so we're fine
                                   // continuing and assembling the
                                   // one big matrix from the small
                                   // ones of the base elements
  std::vector<FullMatrix<double> > base_matrices (this->n_base_elements());
  for (unsigned int i=0; i<this->n_base_elements(); ++i)
    {
      base_matrices[i].reinit (base_element(i).dofs_per_cell,
                               source_fe.base_element(i).dofs_per_cell);
      base_element(i).get_interpolation_matrix (source_fe.base_element(i),
                                                base_matrices[i]);
    }

                                   // first clear big matrix, to make
                                   // sure that entries that would
                                   // couple different bases (or
                                   // multiplicity indices) are really
                                   // zero. then assign entries
  interpolation_matrix = 0;
  for (unsigned int i=0; i<this->dofs_per_cell; ++i)
    for (unsigned int j=0; j<source_fe.dofs_per_cell; ++j)
      if (this->system_to_base_table[i].first ==
          source_fe.system_to_base_table[j].first)
        interpolation_matrix(i,j)
          = (base_matrices[this->system_to_base_table[i].first.first]
             (this->system_to_base_table[i].second,
              source_fe.system_to_base_table[j].second));
}



//---------------------------------------------------------------------------
// Data field initialization
//---------------------------------------------------------------------------



template <int dim, int spacedim>
UpdateFlags
FESystem<dim,spacedim>::update_once (const UpdateFlags flags) const
{
  UpdateFlags out = update_default;
				   // generate maximal set of flags
				   // that are necessary
  for (unsigned int base_no=0; base_no<this->n_base_elements(); ++base_no)
    out |= base_element(base_no).update_once(flags);
  return out;
}



template <int dim, int spacedim>
UpdateFlags
FESystem<dim,spacedim>::update_each (const UpdateFlags flags) const
{
  UpdateFlags out = update_default;
				   // generate maximal set of flags
				   // that are necessary
  for (unsigned int base_no=0; base_no<this->n_base_elements(); ++base_no)
    out |= base_element(base_no).update_each(flags);

				   // second derivatives are handled
				   // by the top-level finite element,
				   // rather than by the base elements
				   // since it is generated by finite
				   // differencing. if second
				   // derivatives are requested, we
				   // therefore have to set the
				   // respective flag since the base
				   // elements don't have them
  if (flags & update_hessians)
    out |= update_hessians | update_covariant_transformation;

  return out;
}



template <int dim, int spacedim>
typename Mapping<dim,spacedim>::InternalDataBase *
FESystem<dim,spacedim>::get_data (const UpdateFlags      flags_,
				  const Mapping<dim,spacedim>    &mapping,
				  const Quadrature<dim> &quadrature) const
{
  UpdateFlags flags = flags_;
  InternalData* data = new InternalData(this->n_base_elements());

  data->update_once = update_once (flags);
  data->update_each = update_each (flags);
  flags = data->update_once | data->update_each;

  UpdateFlags sub_flags = flags;
				   // if second derivatives through
				   // finite differencing are required,
				   // then initialize some objects for
				   // that
  data->compute_hessians = flags & update_hessians;
  if (data->compute_hessians)
    {
				       // delete
				       // update_hessians
				       // from flags list
      sub_flags = UpdateFlags (sub_flags ^ update_hessians);
      data->initialize_2nd (this, mapping, quadrature);
    }

				   // get data objects from each of
				   // the base elements and store them
  for (unsigned int base_no=0; base_no<this->n_base_elements(); ++base_no)
    {
      typename Mapping<dim,spacedim>::InternalDataBase *base_fe_data_base =
	base_element(base_no).get_data(sub_flags, mapping, quadrature);

      typename FiniteElement<dim,spacedim>::InternalDataBase *base_fe_data =
	dynamic_cast<typename FiniteElement<dim,spacedim>::InternalDataBase *>
	(base_fe_data_base);
      Assert (base_fe_data != 0, ExcInternalError());

      data->set_fe_data(base_no, base_fe_data);

				       // make sure that *we* compute
				       // second derivatives, base
				       // elements should not do it
      Assert (!(base_fe_data->update_each & update_hessians),
	      ExcInternalError());
      Assert (!(base_fe_data->update_once & update_hessians),
	      ExcInternalError());

				       // The FEValuesData @p{data}
				       // given to the
				       // @p{fill_fe_values} function
				       // includes the FEValuesDatas
				       // of the FESystem. Here the
				       // FEValuesDatas @p{*base_data}
				       // needs to be created that
				       // later will be given to the
				       // @p{fill_fe_values} functions
				       // of the base
				       // elements. @p{base_data->initialize}
				       // cannot be called earlier as
				       // in the @p{fill_fe_values}
				       // function called for the
				       // first cell. This is because
				       // the initialize function
				       // needs the update flags as
				       // argument.
				       //
				       // The pointers @p{base_data}
				       // are stored into the
				       // FESystem::InternalData
				       // @p{data}, similar to the
				       // storing of the
				       // @p{base_fe_data}s.
      FEValuesData<dim,spacedim> *base_data = new FEValuesData<dim,spacedim>();
      data->set_fe_values_data(base_no, base_data);
    }
  data->update_flags = data->update_once |
		       data->update_each;
  return data;
}

// The following function is a clone of get_data, with the exception
// that get_face_data of the base elements is called.

template <int dim, int spacedim>
typename Mapping<dim,spacedim>::InternalDataBase *
FESystem<dim,spacedim>::get_face_data (
  const UpdateFlags      flags_,
  const Mapping<dim,spacedim>    &mapping,
  const Quadrature<dim-1> &quadrature) const
{
  UpdateFlags flags = flags_;
  InternalData* data = new InternalData(this->n_base_elements());

  data->update_once = update_once (flags);
  data->update_each = update_each (flags);
  flags = data->update_once | data->update_each;

  UpdateFlags sub_flags = flags;
  data->compute_hessians = flags & update_hessians;
  if (data->compute_hessians)
    {
      sub_flags = UpdateFlags (sub_flags ^ update_hessians);
      data->initialize_2nd (this, mapping, QProjector<dim>::project_to_all_faces(quadrature));
    }

  for (unsigned int base_no=0; base_no<this->n_base_elements(); ++base_no)
    {
      typename Mapping<dim,spacedim>::InternalDataBase *base_fe_data_base =
	base_element(base_no).get_face_data(sub_flags, mapping, quadrature);

      typename FiniteElement<dim,spacedim>::InternalDataBase *base_fe_data =
	dynamic_cast<typename FiniteElement<dim,spacedim>::InternalDataBase *>
	(base_fe_data_base);
      Assert (base_fe_data != 0, ExcInternalError());

      data->set_fe_data(base_no, base_fe_data);

      Assert (!(base_fe_data->update_each & update_hessians),
	      ExcInternalError());
      Assert (!(base_fe_data->update_once & update_hessians),
	      ExcInternalError());

      FEValuesData<dim,spacedim> *base_data = new FEValuesData<dim,spacedim>();
      data->set_fe_values_data(base_no, base_data);
    }
  data->update_flags = data->update_once |
		       data->update_each;
  return data;
}



// The following function is a clone of get_data, with the exception
// that get_subface_data of the base elements is called.

template <int dim, int spacedim>
typename Mapping<dim,spacedim>::InternalDataBase *
FESystem<dim,spacedim>::get_subface_data (
  const UpdateFlags      flags_,
  const Mapping<dim,spacedim>    &mapping,
  const Quadrature<dim-1> &quadrature) const
{
  UpdateFlags flags = flags_;
  InternalData* data = new InternalData(this->n_base_elements());

  data->update_once = update_once (flags);
  data->update_each = update_each (flags);
  flags = data->update_once | data->update_each;

  UpdateFlags sub_flags = flags;
  data->compute_hessians = flags & update_hessians;
  if (data->compute_hessians)
    {
      sub_flags = UpdateFlags (sub_flags ^ update_hessians);
      data->initialize_2nd (this, mapping, QProjector<dim>::project_to_all_subfaces(quadrature));
    }

  for (unsigned int base_no=0; base_no<this->n_base_elements(); ++base_no)
    {
      typename Mapping<dim,spacedim>::InternalDataBase *base_fe_data_base =
	base_element(base_no).get_subface_data(sub_flags, mapping, quadrature);

      typename FiniteElement<dim,spacedim>::InternalDataBase *base_fe_data =
	dynamic_cast<typename FiniteElement<dim,spacedim>::InternalDataBase *>
	(base_fe_data_base);
      Assert (base_fe_data != 0, ExcInternalError());

      data->set_fe_data(base_no, base_fe_data);

      Assert (!(base_fe_data->update_each & update_hessians),
	      ExcInternalError());
      Assert (!(base_fe_data->update_once & update_hessians),
	      ExcInternalError());

      FEValuesData<dim,spacedim> *base_data = new FEValuesData<dim,spacedim>();
      data->set_fe_values_data(base_no, base_data);
    }
  data->update_flags = data->update_once |
		       data->update_each;
  return data;
}



template <int dim, int spacedim>
void
FESystem<dim,spacedim>::fill_fe_values (
  const Mapping<dim,spacedim>                      &mapping,
  const typename Triangulation<dim,spacedim>::cell_iterator &cell,
  const Quadrature<dim>                            &quadrature,
  typename Mapping<dim,spacedim>::InternalDataBase &mapping_data,
  typename Mapping<dim,spacedim>::InternalDataBase &fe_data,
  FEValuesData<dim,spacedim>                       &data,
  CellSimilarity::Similarity                  &cell_similarity) const
{
  compute_fill(mapping, cell, invalid_face_number, invalid_face_number,
	       quadrature, cell_similarity, mapping_data, fe_data, data);
}



template <int dim, int spacedim>
void
FESystem<dim,spacedim>::fill_fe_face_values (
  const Mapping<dim,spacedim>                   &mapping,
  const typename Triangulation<dim,spacedim>::cell_iterator &cell,
  const unsigned int                    face_no,
  const Quadrature<dim-1>              &quadrature,
  typename Mapping<dim,spacedim>::InternalDataBase &mapping_data,
  typename Mapping<dim,spacedim>::InternalDataBase &fe_data,
  FEValuesData<dim,spacedim>                    &data) const
{
  compute_fill (mapping, cell, face_no, invalid_face_number, quadrature,
                CellSimilarity::none, mapping_data, fe_data, data);
}




template <int dim, int spacedim>
void
FESystem<dim,spacedim>::fill_fe_subface_values (
  const Mapping<dim,spacedim>                      &mapping,
  const typename Triangulation<dim,spacedim>::cell_iterator &cell,
  const unsigned int                                face_no,
  const unsigned int                                sub_no,
  const Quadrature<dim-1>                          &quadrature,
  typename Mapping<dim,spacedim>::InternalDataBase &mapping_data,
  typename Mapping<dim,spacedim>::InternalDataBase &fe_data,
  FEValuesData<dim,spacedim>                       &data) const
{
  compute_fill (mapping, cell, face_no, sub_no, quadrature,
                CellSimilarity::none, mapping_data, fe_data, data);
}



template <int dim, int spacedim>
template <int dim_1>
void
FESystem<dim,spacedim>::compute_fill (
  const Mapping<dim,spacedim>                      &mapping,
  const typename Triangulation<dim,spacedim>::cell_iterator &cell,
  const unsigned int                                face_no,
  const unsigned int                                sub_no,
  const Quadrature<dim_1>                          &quadrature,
  CellSimilarity::Similarity                   cell_similarity,
  typename Mapping<dim,spacedim>::InternalDataBase &mapping_data,
  typename Mapping<dim,spacedim>::InternalDataBase &fedata,
  FEValuesData<dim,spacedim>                       &data) const
{
  const unsigned int n_q_points = quadrature.size();

				   // convert data object to internal
				   // data for this class. fails with
				   // an exception if that is not
				   // possible
  Assert (dynamic_cast<InternalData*> (&fedata) != 0, ExcInternalError());
  InternalData & fe_data = static_cast<InternalData&> (fedata);

				   // Either dim_1==dim
				   // (fill_fe_values) or dim_1==dim-1
				   // (fill_fe_(sub)face_values)
  Assert(dim_1==dim || dim_1==dim-1, ExcInternalError());
  const UpdateFlags flags(dim_1==dim ?
			  fe_data.current_update_flags() :
			  fe_data.update_flags);


  if (flags & (update_values | update_gradients))
    {
      if (fe_data.is_first_cell())
	{
					   // Initialize the
					   // FEValuesDatas for the
					   // base elements.
					   // Originally this was the
					   // task of
					   // FEValues::FEValues() but
					   // the latter initializes
					   // the FEValuesDatas only
					   // of the FESystem, not of
					   // the FEValuesDatas needed
					   // by the base elements
					   // (and: how should it know
					   // of their existence,
					   // after all).
	  for (unsigned int base_no=0; base_no<this->n_base_elements(); ++base_no)
	    {
					       // Pointer needed to get
					       // the update flags of the
					       // base element
	      typename Mapping<dim,spacedim>::InternalDataBase &
		base_fe_data = fe_data.get_fe_data(base_no);

					       // compute update flags ...
	      const UpdateFlags base_update_flags
                = mapping_data.update_flags | base_fe_data.update_flags;

					       // Initialize the FEValuesDatas
					       // for the base elements.
	      FEValuesData<dim,spacedim> &base_data=fe_data.get_fe_values_data(base_no);
	      const FiniteElement<dim,spacedim> &base_fe=base_element(base_no);
	      base_data.initialize (n_q_points, base_fe, base_update_flags);
	    }
	}

				       // fill_fe_face_values needs
				       // argument Quadrature<dim-1>
				       // for both cases
				       // dim_1==dim-1 and
				       // dim_1=dim. Hence the
				       // following workaround
      const Quadrature<dim>   *cell_quadrature = 0;
      const Quadrature<dim-1> *face_quadrature = 0;

				       // static cast to the
				       // common base class of
				       // quadrature being either
				       // Quadrature<dim> or
				       // Quadrature<dim-1>:
      const Subscriptor* quadrature_base_pointer = &quadrature;

      if (face_no==invalid_face_number)
	{
	  Assert(dim_1==dim, ExcDimensionMismatch(dim_1,dim));

					   // Some Mac OS X versions
					   // have a bug that makes
					   // the following assertion
					   // fail spuriously. In that
					   // case, just disable the
					   // Assert, as this is not
					   // something that depends
					   // on input parameters
					   // provided by users; the
					   // validity of the
					   // assertion is still
					   // checked on all systems
					   // that don't have this bug
#ifndef DEAL_II_HAVE_DARWIN_DYNACAST_BUG
	  Assert (dynamic_cast<const Quadrature<dim> *>(quadrature_base_pointer) != 0,
		  ExcInternalError());
#endif
	  cell_quadrature
            = static_cast<const Quadrature<dim> *>(quadrature_base_pointer);
	}
      else
	{
	  Assert(dim_1==dim-1, ExcDimensionMismatch(dim_1,dim-1));
#ifndef DEAL_II_HAVE_DARWIN_DYNACAST_BUG
	  Assert (dynamic_cast<const Quadrature<dim-1> *>(quadrature_base_pointer) != 0,
		  ExcInternalError());
#endif
	  face_quadrature
            = static_cast<const Quadrature<dim-1> *>(quadrature_base_pointer);
	}

                                       // let base elements update the
                                       // necessary data
      for (unsigned int base_no=0; base_no<this->n_base_elements(); ++base_no)
	{
	  const FiniteElement<dim,spacedim> &
            base_fe      = base_element(base_no);
	  typename FiniteElement<dim,spacedim>::InternalDataBase &
	    base_fe_data = fe_data.get_fe_data(base_no);
	  FEValuesData<dim,spacedim> &
            base_data    = fe_data.get_fe_values_data(base_no);

					   //TODO: Think about a smarter alternative
					   // Copy quadrature points. These
					   // are required for computing the
					   // determinant in the FEPolyTensor
					   // class. The determinant is
					   // one ingredient of the Piola
					   // transformation, which is applied
					   // to correctly map the RT space from
					   // the reference element to the global
					   // coordinate system.
	  if (cell_similarity != CellSimilarity::translation)
	    base_data.JxW_values = data.JxW_values;


                                           // Make sure that in the
                                           // case of fill_fe_values
                                           // the data is only copied
                                           // from base_data to data
                                           // if base_data is
                                           // changed. therefore use
                                           // fe_fe_data.current_update_flags()
                                           //
                                           // for the case of
                                           // fill_fe_(sub)face_values
                                           // the data needs to be
                                           // copied from base_data to
                                           // data on each face,
                                           // therefore use
                                           // base_fe_data.update_flags.
	  if (face_no==invalid_face_number)
	    base_fe.fill_fe_values(mapping, cell, *cell_quadrature, mapping_data,
				   base_fe_data, base_data, cell_similarity);
	  else if (sub_no==invalid_face_number)
	    base_fe.fill_fe_face_values(mapping, cell, face_no,
					*face_quadrature, mapping_data, base_fe_data, base_data);
	  else
	    base_fe.fill_fe_subface_values(mapping, cell, face_no, sub_no,
					   *face_quadrature, mapping_data, base_fe_data, base_data);

                                           // now data has been
                                           // generated, so copy
                                           // it. we used to work by
                                           // looping over all base
                                           // elements (i.e. this
                                           // outer loop), then over
                                           // multiplicity, then over
                                           // the shape functions from
                                           // that base element, but
                                           // that requires that we
                                           // can infer the global
                                           // number of a shape
                                           // function from its number
                                           // in the base element. for
                                           // that we had the
                                           // component_to_system_table.
                                           //
                                           // however, this does of
                                           // course no longer work
                                           // since we have
                                           // non-primitive
                                           // elements. so we go the
                                           // other way round: loop
                                           // over all shape functions
                                           // of the composed element,
                                           // and here only treat
                                           // those shape functions
                                           // that belong to a given
                                           // base element
//TODO: Introduce the needed table and loop only over base element shape functions. This here is not efficient at all AND very bad style	  
          const UpdateFlags base_flags(dim_1==dim ?
                                       base_fe_data.current_update_flags() :
                                       base_fe_data.update_flags);

				           // if the current cell is just a
				           // translation of the previous
				           // one, the underlying data has
				           // not changed, and we don't even
				           // need to enter this section
	  if (cell_similarity != CellSimilarity::translation)
	    for (unsigned int system_index=0; system_index<this->dofs_per_cell;
		 ++system_index)
	      if (this->system_to_base_table[system_index].first.first == base_no)
		{
		  const unsigned int
		    base_index = this->system_to_base_table[system_index].second;
		  Assert (base_index<base_fe.dofs_per_cell, ExcInternalError());

                                                 // now copy. if the
                                                 // shape function is
                                                 // primitive, then
                                                 // there is only one
                                                 // value to be copied,
                                                 // but for
                                                 // non-primitive
                                                 // elements, there
                                                 // might be more values
                                                 // to be copied
                                                 //
                                                 // so, find out from
                                                 // which index to take
                                                 // this one value, and
                                                 // to which index to
                                                 // put
		  unsigned int out_index = 0;
		  for (unsigned int i=0; i<system_index; ++i)
		    out_index += this->n_nonzero_components(i);
		  unsigned int in_index = 0;
		  for (unsigned int i=0; i<base_index; ++i)
		    in_index += base_fe.n_nonzero_components(i);

                                                 // then loop over the
                                                 // number of components
                                                 // to be copied
		  Assert (this->n_nonzero_components(system_index) ==
			  base_fe.n_nonzero_components(base_index),
			  ExcInternalError());
		  for (unsigned int s=0; s<this->n_nonzero_components(system_index); ++s)
		    {
		      if (base_flags & update_values)
			for (unsigned int q=0; q<n_q_points; ++q)
			  data.shape_values[out_index+s][q] =
			    base_data.shape_values(in_index+s,q);
		      
		      if (base_flags & update_gradients)
			for (unsigned int q=0; q<n_q_points; ++q)
			  data.shape_gradients[out_index+s][q]=
			    base_data.shape_gradients[in_index+s][q];

                                                     // _we_ handle
                                                     // computation of
                                                     // second
                                                     // derivatives,
                                                     // so the base
                                                     // elements
                                                     // should not
                                                     // have computed
                                                     // them!
		      Assert (!(base_flags & update_hessians),
			      ExcInternalError());
		    };
		};
        };
    }

  if (fe_data.compute_hessians && cell_similarity != CellSimilarity::translation)
    {
      unsigned int offset = 0;
      if (face_no != invalid_face_number)
	{
	  if (sub_no == invalid_face_number)
	    offset=QProjector<dim>::DataSetDescriptor
		   ::face(face_no,
			  cell->face_orientation(face_no),
			  cell->face_flip(face_no),
			  cell->face_rotation(face_no),
			  n_q_points);
	  else
	    offset=QProjector<dim>::DataSetDescriptor
		   ::subface(face_no, sub_no,
			     cell->face_orientation(face_no),
			     cell->face_flip(face_no),
			     cell->face_rotation(face_no),
			     n_q_points,
			     cell->subface_case(face_no));
	}

      this->compute_2nd (mapping, cell, offset, mapping_data, fe_data, data);
    }
}



template <int dim, int spacedim>
void
FESystem<dim,spacedim>::build_cell_tables()
{
				   // If the system is not primitive,
				   // these have not been initialized
				   // by FiniteElement
  this->system_to_component_table.resize(this->dofs_per_cell);
  this->face_system_to_component_table.resize(this->dofs_per_face);

  unsigned total_index = 0;
  this->block_indices_data.reinit(0,0);
  
  for (unsigned int base=0; base < this->n_base_elements(); ++base)
    for (unsigned int m = 0; m < this->element_multiplicity(base); ++m)
      {
	this->block_indices_data.push_back(base_element(base).dofs_per_cell);
	for (unsigned int k=0; k<base_element(base).n_components(); ++k)
	  this->component_to_base_table[total_index++]
	    = std::make_pair(std::make_pair(base,k), m);
      }
  Assert (total_index == this->component_to_base_table.size(),
	  ExcInternalError());
  
				   // Initialize index tables.
				   // Multi-component base elements
				   // have to be thought of. For
				   // non-primitive shape functions,
				   // have a special invalid index.
  const std::pair<unsigned int, unsigned int>
    non_primitive_index (numbers::invalid_unsigned_int,
			 numbers::invalid_unsigned_int);

				   // First enumerate vertex indices,
				   // where we first enumerate all
				   // indices on the first vertex in
				   // the order of the base elements,
				   // then of the second vertex, etc
  total_index = 0;
  for (unsigned int vertex_number=0;
       vertex_number<GeometryInfo<dim>::vertices_per_cell;
       ++vertex_number)
    {
      unsigned comp_start = 0;
      for(unsigned int base=0; base<this->n_base_elements(); ++base)
	for (unsigned int m=0; m<this->element_multiplicity(base);
	     ++m, comp_start+=base_element(base).n_components())
	  for (unsigned int local_index = 0;
	       local_index < base_element(base).dofs_per_vertex;
	       ++local_index, ++total_index)
	    {
	      const unsigned int index_in_base
		= (base_element(base).dofs_per_vertex*vertex_number +
		   local_index);

	      this->system_to_base_table[total_index]
		= std::make_pair (std::make_pair(base, m), index_in_base);

	      if (base_element(base).is_primitive(index_in_base))
		{
		  const unsigned int comp_in_base
		    = base_element(base).system_to_component_index(index_in_base).first;
		  const unsigned int comp
		    = comp_start + comp_in_base;
		  const unsigned int index_in_comp
		    = base_element(base).system_to_component_index(index_in_base).second;
		  this->system_to_component_table[total_index]
		    = std::make_pair (comp, index_in_comp);
		}
	      else
		this->system_to_component_table[total_index] = non_primitive_index;
	    }
    }

				   // 2. Lines
  if (GeometryInfo<dim>::lines_per_cell > 0)
    for (unsigned int line_number= 0;
	 line_number != GeometryInfo<dim>::lines_per_cell;
	 ++line_number)
      {
	unsigned comp_start = 0;
	for (unsigned int base=0; base<this->n_base_elements(); ++base)
	  for (unsigned int m=0; m<this->element_multiplicity(base);
	       ++m, comp_start+=base_element(base).n_components())
	    for (unsigned int local_index = 0;
		 local_index < base_element(base).dofs_per_line;
		 ++local_index, ++total_index)
	      {
		const unsigned int index_in_base
		  = (base_element(base).dofs_per_line*line_number +
		     local_index +
		     base_element(base).first_line_index);

		this->system_to_base_table[total_index]
		  = std::make_pair (std::make_pair(base,m), index_in_base);

		if (base_element(base).is_primitive(index_in_base))
		  {
		    const unsigned int comp_in_base
		      = base_element(base).system_to_component_index(index_in_base).first;
		    const unsigned int comp
		      = comp_start + comp_in_base;
		    const unsigned int index_in_comp
		      = base_element(base).system_to_component_index(index_in_base).second;
		    this->system_to_component_table[total_index]
		      = std::make_pair (comp, index_in_comp);
		  }
		else
		  this->system_to_component_table[total_index] = non_primitive_index;
	      }
      }

				   // 3. Quads
  if (GeometryInfo<dim>::quads_per_cell > 0)
    for (unsigned int quad_number= 0;
	 quad_number != GeometryInfo<dim>::quads_per_cell;
	 ++quad_number)
      {
	unsigned int comp_start = 0;
	for (unsigned int base=0; base<this->n_base_elements(); ++base)
	  for (unsigned int m=0; m<this->element_multiplicity(base);
	       ++m, comp_start += base_element(base).n_components())
	    for (unsigned int local_index = 0;
		 local_index < base_element(base).dofs_per_quad;
		 ++local_index, ++total_index)
	      {
		const unsigned int index_in_base
		  = (base_element(base).dofs_per_quad*quad_number +
		     local_index +
		     base_element(base).first_quad_index);

		this->system_to_base_table[total_index]
		  = std::make_pair (std::make_pair(base,m), index_in_base);

		if (base_element(base).is_primitive(index_in_base))
		  {
		    const unsigned int comp_in_base
		      = base_element(base).system_to_component_index(index_in_base).first;
		    const unsigned int comp
		      = comp_start + comp_in_base;
		    const unsigned int index_in_comp
		      = base_element(base).system_to_component_index(index_in_base).second;
		    this->system_to_component_table[total_index]
		      = std::make_pair (comp, index_in_comp);
		  }
		else
		  this->system_to_component_table[total_index] = non_primitive_index;
	      }
      }

				   // 4. Hexes
  if (GeometryInfo<dim>::hexes_per_cell > 0)
    for (unsigned int hex_number= 0;
	 hex_number != GeometryInfo<dim>::hexes_per_cell;
	 ++hex_number)
      {
	unsigned int comp_start = 0;
	for(unsigned int base=0; base<this->n_base_elements(); ++base)
	  for (unsigned int m=0; m<this->element_multiplicity(base);
	       ++m, comp_start+=base_element(base).n_components())
	    for (unsigned int local_index = 0;
		 local_index < base_element(base).dofs_per_hex;
		 ++local_index, ++total_index)
	      {
		const unsigned int index_in_base
		  = (base_element(base).dofs_per_hex*hex_number +
		     local_index +
		     base_element(base).first_hex_index);

		this->system_to_base_table[total_index]
		  = std::make_pair (std::make_pair(base,m), index_in_base);

		if (base_element(base).is_primitive(index_in_base))
		  {
		    const unsigned int comp_in_base
		      = base_element(base).system_to_component_index(index_in_base).first;
		    const unsigned int comp
		      = comp_start + comp_in_base;
		    const unsigned int index_in_comp
		      = base_element(base).system_to_component_index(index_in_base).second;
		    this->system_to_component_table[total_index]
		      = std::make_pair (comp, index_in_comp);
		  }
		else
		  this->system_to_component_table[total_index] = non_primitive_index;
	      }
      }
}



template <int dim, int spacedim>
void
FESystem<dim,spacedim>::build_face_tables()
{
				   // Initialize index tables. do this
				   // in the same way as done for the
				   // cell tables, except that we now
				   // loop over the objects of faces

				   // For non-primitive shape
				   // functions, have a special
				   // invalid index
  const std::pair<unsigned int, unsigned int>
    non_primitive_index (numbers::invalid_unsigned_int,
			 numbers::invalid_unsigned_int);

				   // 1. Vertices
  unsigned int total_index = 0;
  for (unsigned int vertex_number=0;
       vertex_number<GeometryInfo<dim>::vertices_per_face;
       ++vertex_number)
    {
      unsigned int comp_start = 0;
      for(unsigned int base=0; base<this->n_base_elements(); ++base)
	for (unsigned int m=0; m<this->element_multiplicity(base);
	     ++m, comp_start += base_element(base).n_components())
	  for (unsigned int local_index = 0;
	       local_index < base_element(base).dofs_per_vertex;
	       ++local_index, ++total_index)
	    {
					       // get (cell) index of
					       // this shape function
					       // inside the base
					       // element to see
					       // whether the shape
					       // function is
					       // primitive (assume
					       // that all shape
					       // functions on
					       // vertices share the
					       // same primitivity
					       // property; assume
					       // likewise for all
					       // shape functions
					       // located on lines,
					       // quads, etc. this
					       // way, we can ask for
					       // primitivity of only
					       // _one_ shape
					       // function, which is
					       // taken as
					       // representative for
					       // all others located
					       // on the same type of
					       // object):
	      const unsigned int index_in_base
		= (base_element(base).dofs_per_vertex*vertex_number +
		   local_index);

	      const unsigned int face_index_in_base
		= (base_element(base).dofs_per_vertex*vertex_number +
		   local_index);

	      this->face_system_to_base_table[total_index]
		= std::make_pair (std::make_pair(base,m), face_index_in_base);

	      if (base_element(base).is_primitive(index_in_base))
		{
		  const unsigned int comp_in_base
		    = base_element(base).face_system_to_component_index(face_index_in_base).first;
		  const unsigned int comp
		    = comp_start + comp_in_base;
		  const unsigned int face_index_in_comp
		    = base_element(base).face_system_to_component_index(face_index_in_base).second;
		  this->face_system_to_component_table[total_index]
		    = std::make_pair (comp, face_index_in_comp);
		}
	      else
		this->face_system_to_component_table[total_index] = non_primitive_index;
	    }
    }

				   // 2. Lines
  if (GeometryInfo<dim>::lines_per_face > 0)
    for (unsigned line_number= 0;
	 line_number != GeometryInfo<dim>::lines_per_face;
	 ++line_number)
      {
	unsigned comp_start = 0;
	for(unsigned base = 0; base < this->n_base_elements(); ++base)
	  for (unsigned m=0; m<this->element_multiplicity(base);
	       ++m, comp_start += base_element(base).n_components())
	    for (unsigned local_index = 0;
		 local_index < base_element(base).dofs_per_line;
		 ++local_index, ++total_index)
	      {
						 // do everything
						 // alike for this
						 // type of object
		const unsigned int index_in_base
		  = (base_element(base).dofs_per_line*line_number +
		     local_index +
		     base_element(base).first_line_index);

		const unsigned int face_index_in_base
		  = (base_element(base).first_face_line_index +
		     base_element(base).dofs_per_line * line_number +
		     local_index);

		this->face_system_to_base_table[total_index]
		  = std::make_pair (std::make_pair(base,m), face_index_in_base);

		if (base_element(base).is_primitive(index_in_base))
		  {
		    const unsigned int comp_in_base
		      = base_element(base).face_system_to_component_index(face_index_in_base).first;
		    const unsigned int comp
		      = comp_start + comp_in_base;
		    const unsigned int face_index_in_comp
		      = base_element(base).face_system_to_component_index(face_index_in_base).second;
		    this->face_system_to_component_table[total_index]
		      = std::make_pair (comp, face_index_in_comp);
		  }
		else
		  this->face_system_to_component_table[total_index] = non_primitive_index;
	      }
      }

				   // 3. Quads
  if (GeometryInfo<dim>::quads_per_face > 0)
    for (unsigned quad_number= 0;
	 quad_number != GeometryInfo<dim>::quads_per_face;
	 ++quad_number)
      {
	unsigned comp_start = 0;
	for(unsigned base=0; base<this->n_base_elements(); ++base)
	  for (unsigned m=0; m<this->element_multiplicity(base);
	       ++m, comp_start += base_element(base).n_components())
	    for (unsigned local_index = 0;
		 local_index < base_element(base).dofs_per_quad;
		 ++local_index, ++total_index)
	      {
						 // do everything
						 // alike for this
						 // type of object
		const unsigned int index_in_base
		  = (base_element(base).dofs_per_quad*quad_number +
		     local_index +
		     base_element(base).first_quad_index);

		const unsigned int face_index_in_base
		  = (base_element(base).first_face_quad_index +
		     base_element(base).dofs_per_quad * quad_number +
		     local_index);

		this->face_system_to_base_table[total_index]
		  = std::make_pair (std::make_pair(base,m), face_index_in_base);

		if (base_element(base).is_primitive(index_in_base))
		  {
		    const unsigned int comp_in_base
		      = base_element(base).face_system_to_component_index(face_index_in_base).first;
		    const unsigned int comp
		      = comp_start + comp_in_base;
		    const unsigned int face_index_in_comp
		      = base_element(base).face_system_to_component_index(face_index_in_base).second;
		    this->face_system_to_component_table[total_index]
		      = std::make_pair (comp, face_index_in_comp);
		  }
		else
		  this->face_system_to_component_table[total_index] = non_primitive_index;
	      }
      }
  Assert (total_index == this->dofs_per_face, ExcInternalError());
  Assert (total_index == this->face_system_to_component_table.size(),
	  ExcInternalError());
  Assert (total_index == this->face_system_to_base_table.size(),
	  ExcInternalError());
}



template <int dim, int spacedim>
void FESystem<dim,spacedim>::build_interface_constraints ()
{
                                   // check whether all base elements
                                   // implement their interface
                                   // constraint matrices. if this is
                                   // not the case, then leave the
                                   // interface costraints of this
                                   // composed element empty as well;
                                   // however, the rest of the element
                                   // is usable
  for (unsigned int base=0; base<this->n_base_elements(); ++base)
    if (base_element(base).constraints_are_implemented() == false)
      return;

  this->interface_constraints.
    TableBase<2,double>::reinit (this->interface_constraints_size());

				   // the layout of the constraints
				   // matrix is described in the
				   // FiniteElement class. you may
				   // want to look there first before
				   // trying to understand the
				   // following, especially the
				   // mapping of the @p{m} index.
				   //
				   // in order to map it to the
				   // fe-system class, we have to know
				   // which base element a degree of
				   // freedom within a vertex, line,
				   // etc belongs to. this can be
				   // accomplished by the
				   // system_to_component_index
				   // function in conjunction with the
				   // numbers
				   // first_{line,quad,...}_index
  for (unsigned int n=0; n<this->interface_constraints.n(); ++n)
    for (unsigned int m=0; m<this->interface_constraints.m(); ++m)
      {
					 // for the pair (n,m) find
					 // out which base element
					 // they belong to and the
					 // number therein
					 //
					 // first for the n
					 // index. this is simple
					 // since the n indices are in
					 // the same order as they are
					 // usually on a face. note
					 // that for the data type,
					 // first value in pair is
					 // (base element,instance of
					 // base element), second is
					 // index within this instance
	const std::pair<std::pair<unsigned int,unsigned int>, unsigned int> n_index
	  = this->face_system_to_base_table[n];

					 // likewise for the m
					 // index. this is more
					 // complicated due to the
					 // strange ordering we have
					 // for the dofs on the
					 // refined faces.
	std::pair<std::pair<unsigned int,unsigned int>, unsigned int> m_index;
	switch (dim)
	  {
	    case 1:
            {
                                               // we should never get here!
                                               // (in 1d, the constraints matrix
                                               // should be of size zero)
              Assert (false, ExcInternalError());
              break;
            };

	    case 2:
            {
                                               // the indices m=0..d_v-1 are
                                               // from the center vertex.
                                               // their order is the same
                                               // as for the first vertex
                                               // of the whole cell, so we
                                               // can use the
                                               // system_to_base_table
                                               // variable (using the
                                               // face_s_t_base_t function would
                                               // yield the same)
              if (m < this->dofs_per_vertex)
                m_index = this->system_to_base_table[m];
              else
                                                 // then come the two sets of
                                                 // line indices
                {
                  const unsigned int index_in_line
                    = (m-this->dofs_per_vertex) % this->dofs_per_line;
                  const unsigned int sub_line
                    = (m-this->dofs_per_vertex) / this->dofs_per_line;
                  Assert (sub_line < 2, ExcInternalError());

                                                   // from this
                                                   // information, try
                                                   // to get base
                                                   // element and
                                                   // instance of base
                                                   // element. we do
                                                   // so by
                                                   // constructing the
                                                   // corresponding
                                                   // face index of m
                                                   // in the present
                                                   // element, then
                                                   // use
                                                   // face_system_to_base_table
                  const unsigned int tmp1 = 2*this->dofs_per_vertex+index_in_line;
                  m_index.first = this->face_system_to_base_table[tmp1].first;

                                                   // what we are
                                                   // still missing is
                                                   // the index of m
                                                   // within the base
                                                   // elements
                                                   // interface_constraints
                                                   // table
                                                   //
                                                   // here, the second
                                                   // value of
                                                   // face_system_to_base_table
                                                   // can help: it
                                                   // denotes the face
                                                   // index of that
                                                   // shape function
                                                   // within the base
                                                   // element. since
                                                   // we know that it
                                                   // is a line dof,
                                                   // we can construct
                                                   // the rest: tmp2
                                                   // will denote the
                                                   // index of this
                                                   // shape function
                                                   // among the line
                                                   // shape functions:
                  Assert (this->face_system_to_base_table[tmp1].second >=
                          2*base_element(m_index.first.first).dofs_per_vertex,
                          ExcInternalError());
                  const unsigned int tmp2 = this->face_system_to_base_table[tmp1].second -
                                            2*base_element(m_index.first.first).dofs_per_vertex;
                  Assert (tmp2 < base_element(m_index.first.first).dofs_per_line,
                          ExcInternalError());
                  m_index.second = base_element(m_index.first.first).dofs_per_vertex +
                                   base_element(m_index.first.first).dofs_per_line*sub_line +
                                   tmp2;
                };
              break;
            };

	    case 3:
            {
                                               // same way as above,
                                               // although a little
                                               // more complicated...

                                               // the indices
                                               // m=0..5*d_v-1 are
                                               // from the center and
                                               // the four subline
                                               // vertices.  their
                                               // order is the same as
                                               // for the first vertex
                                               // of the whole cell,
                                               // so we can use the
                                               // simple arithmetic
              if (m < 5*this->dofs_per_vertex)
                m_index = this->system_to_base_table[m];
              else
                                                 // then come the 12 sets of
                                                 // line indices
                if (m < 5*this->dofs_per_vertex + 12*this->dofs_per_line)
                  {
                                                     // for the
                                                     // meaning of all
                                                     // this, see the
                                                     // 2d part
                    const unsigned int index_in_line
                      = (m-5*this->dofs_per_vertex) % this->dofs_per_line;
                    const unsigned int sub_line
                      = (m-5*this->dofs_per_vertex) / this->dofs_per_line;
                    Assert (sub_line < 12, ExcInternalError());

                    const unsigned int tmp1 = 4*this->dofs_per_vertex+index_in_line;
                    m_index.first = this->face_system_to_base_table[tmp1].first;

                    Assert (this->face_system_to_base_table[tmp1].second >=
                            4*base_element(m_index.first.first).dofs_per_vertex,
                            ExcInternalError());
                    const unsigned int tmp2 = this->face_system_to_base_table[tmp1].second -
                                              4*base_element(m_index.first.first).dofs_per_vertex;
                    Assert (tmp2 < base_element(m_index.first.first).dofs_per_line,
                            ExcInternalError());
                    m_index.second = 5*base_element(m_index.first.first).dofs_per_vertex +
                                     base_element(m_index.first.first).dofs_per_line*sub_line +
                                     tmp2;
                  }
                else
                                                   // on one of the four sub-quads
                  {
                                                     // for the
                                                     // meaning of all
                                                     // this, see the
                                                     // 2d part
                    const unsigned int index_in_quad
                      = (m-5*this->dofs_per_vertex-12*this->dofs_per_line) %
                      this->dofs_per_quad;
                    Assert (index_in_quad < this->dofs_per_quad,
                            ExcInternalError());
                    const unsigned int sub_quad
                      = ((m-5*this->dofs_per_vertex-12*this->dofs_per_line) /
                         this->dofs_per_quad);
                    Assert (sub_quad < 4, ExcInternalError());

                    const unsigned int tmp1 = 4*this->dofs_per_vertex +
                                              4*this->dofs_per_line +
                                              index_in_quad;
                    Assert (tmp1 < this->face_system_to_base_table.size(),
                            ExcInternalError());
                    m_index.first = this->face_system_to_base_table[tmp1].first;

                    Assert (this->face_system_to_base_table[tmp1].second >=
                            4*base_element(m_index.first.first).dofs_per_vertex +
                            4*base_element(m_index.first.first).dofs_per_line,
                            ExcInternalError());
                    const unsigned int tmp2 = this->face_system_to_base_table[tmp1].second -
                                              4*base_element(m_index.first.first).dofs_per_vertex -
                                              4*base_element(m_index.first.first).dofs_per_line;
                    Assert (tmp2 < base_element(m_index.first.first).dofs_per_quad,
                            ExcInternalError());
                    m_index.second = 5*base_element(m_index.first.first).dofs_per_vertex +
                                     12*base_element(m_index.first.first).dofs_per_line +
                                     base_element(m_index.first.first).dofs_per_quad*sub_quad +
                                     tmp2;
                  };

              break;
            };

	    default:
                  Assert (false, ExcNotImplemented());
	  };

					 // now that we gathered all
					 // information: use it to
					 // build the matrix. note
					 // that if n and m belong to
					 // different base elements or
					 // instances, then there
					 // definitely will be no
					 // coupling
	if (n_index.first == m_index.first)
	  this->interface_constraints(m,n)
	    = (base_element(n_index.first.first).constraints()(m_index.second,
							       n_index.second));
      };
}



template <int dim, int spacedim>
void FESystem<dim,spacedim>::initialize ()
{
  build_cell_tables();
  build_face_tables();

  for (unsigned int ref_case=RefinementCase<dim>::cut_x;
       ref_case<RefinementCase<dim>::isotropic_refinement+1;
       ++ref_case)
    {
				       // Check if some of the matrices of
				       // the base elements are void.
				       // repeat this check for each RefineCase
      bool do_restriction = true;
      bool do_prolongation = true;

      for (unsigned int i=0; i<this->n_base_elements(); ++i)
	{
	  if (base_element(i).restriction[ref_case-1][0].n() == 0)
	    do_restriction = false;
	  if (base_element(i).prolongation[ref_case-1][0].n() == 0)
	    do_prolongation = false;
	}

				       // if we did not encounter void
				       // matrices, initialize the
				       // respective matrix sizes
      if (do_restriction)
	for (unsigned int i=0;i<GeometryInfo<dim>::n_children(RefinementCase<dim>(ref_case));++i)
	  this->restriction[ref_case-1][i].reinit(this->dofs_per_cell,
						  this->dofs_per_cell);
      if (do_prolongation)
	for (unsigned int i=0;i<GeometryInfo<dim>::n_children(RefinementCase<dim>(ref_case));++i)
	  this->prolongation[ref_case-1][i].reinit(this->dofs_per_cell,
						   this->dofs_per_cell);

				       // distribute the matrices of the
				       // base finite elements to the
				       // matrices of this object. for
				       // this, loop over all degrees of
				       // freedom and take the respective
				       // entry of the underlying base
				       // element.
				       //
				       // note that we by definition of a
				       // base element, they are
				       // independent, i.e. do not
				       // couple. only DoFs that belong to
				       // the same instance of a base
				       // element may couple
      for (unsigned int i=0; i<this->dofs_per_cell; ++i)
	for (unsigned int j=0; j<this->dofs_per_cell; ++j)
	  {
					     // first find out to which
					     // base element indices i and
					     // j belong, and which
					     // instance thereof in case
					     // the base element has a
					     // multiplicity greater than
					     // one. if they should not
					     // happen to belong to the
					     // same instance of a base
					     // element, then they cannot
					     // couple, so go on with the
					     // next index
	    if (this->system_to_base_table[i].first !=
		this->system_to_base_table[j].first)
	      continue;

					     // so get the common base
					     // element and the indices
					     // therein:
	    const unsigned int
	      base = this->system_to_base_table[i].first.first;

	    const unsigned int
	      base_index_i = this->system_to_base_table[i].second,
	      base_index_j = this->system_to_base_table[j].second;

					     // if we are sure that DoFs i
					     // and j may couple, then
					     // copy entries of the
					     // matrices:
	    for (unsigned int child=0; child<GeometryInfo<dim>::n_children(RefinementCase<dim>(ref_case)); ++child)
	      {
		if (do_restriction)
		  this->restriction[ref_case-1][child] (i,j)
		    = (base_element(base)
		       .get_restriction_matrix(child, RefinementCase<dim>(ref_case))(
			 base_index_i,base_index_j));

		if (do_prolongation)
		  this->prolongation[ref_case-1][child] (i,j)
		    = (base_element(base)
		       .get_prolongation_matrix(child, RefinementCase<dim>(ref_case))(
			 base_index_i,base_index_j));
	      };
	  };
    }
				   // now set up the interface constraints.
				   // this is kind'o hairy, so don't try
				   // to do it dimension independent
  build_interface_constraints ();

				   // finally fill in support points
				   // on cell and face
  initialize_unit_support_points ();
  initialize_unit_face_support_points ();

  initialize_quad_dof_index_permutation ();
}



template <int dim, int spacedim>
FiniteElementData<dim>
FESystem<dim,spacedim>::multiply_dof_numbers (const FiniteElementData<dim> &fe_data,
				     const unsigned int            N)
{
  std::vector<unsigned int> dpo;
  dpo.push_back(fe_data.dofs_per_vertex * N);
  dpo.push_back(fe_data.dofs_per_line * N);
  if (dim>1) dpo.push_back(fe_data.dofs_per_quad * N);
  if (dim>2) dpo.push_back(fe_data.dofs_per_hex * N);

  return FiniteElementData<dim> (dpo,
				 fe_data.n_components() * N,
				 fe_data.tensor_degree(),
				 fe_data.conforming_space,
				 N);
}




template <int dim, int spacedim>
void
FESystem<dim,spacedim>::
initialize_unit_support_points ()
{
				   // if one of the base elements
				   // has no support points, then
				   // it makes no sense to define
				   // support points for the
				   // composed element, so return
				   // an empty array to
				   // demonstrate that
				   // fact
  for (unsigned int base_el=0; base_el<this->n_base_elements(); ++base_el)
    if (!base_element(base_el).has_support_points())
      {
	this->unit_support_points.resize(0);
	return;
      };

				   // generate unit support points
				   // from unit support points of sub
				   // elements
  this->unit_support_points.resize(this->dofs_per_cell);

  for (unsigned int i=0; i<this->dofs_per_cell; ++i)
    {
      const unsigned int
	base       = this->system_to_base_table[i].first.first,
	base_index = this->system_to_base_table[i].second;
      Assert (base<this->n_base_elements(), ExcInternalError());
      Assert (base_index<base_element(base).unit_support_points.size(),
	      ExcInternalError());
      this->unit_support_points[i] = base_element(base).unit_support_points[base_index];
    };
}



template <int dim, int spacedim>
void
FESystem<dim,spacedim>::
initialize_unit_face_support_points ()
{
				// Nothing to do in 1D
  if (dim == 1)
    return;

				   // if one of the base elements has no
				   // support points, then it makes no sense
				   // to define support points for the
				   // composed element. In that case, return
				   // an empty array to demonstrate that fact
				   // (note that we ask whether the base
				   // element has no support points at all,
				   // not only none on the face!)
				   //
				   // on the other hand, if there is an
				   // element that simply has no degrees of
				   // freedom on the face at all, then we
				   // don't care whether it has support points
				   // or not. this is, for example, the case
				   // for the stable Stokes element Q(p)^dim
				   // \times DGP(p-1).
  for (unsigned int base_el=0; base_el<this->n_base_elements(); ++base_el)
    if (!base_element(base_el).has_support_points()
	&&
	(base_element(base_el).dofs_per_face > 0))
      {
	this->unit_face_support_points.resize(0);
	return;
      }


				   // generate unit face support points
				   // from unit support points of sub
				   // elements
  this->unit_face_support_points.resize(this->dofs_per_face);

  for (unsigned int i=0; i<this->dofs_per_face; ++i)
    {
      const unsigned int base_i = this->face_system_to_base_table[i].first.first;
      const unsigned int index_in_base = this->face_system_to_base_table[i].second;

      Assert (index_in_base < base_element(base_i).unit_face_support_points.size(),
	      ExcInternalError());

      this->unit_face_support_points[i]
	= base_element(base_i).unit_face_support_points[index_in_base];
    }
}



template <int dim, int spacedim>
void
FESystem<dim,spacedim>::initialize_quad_dof_index_permutation ()
{
				// nothing to do in other dimensions than 3
  if (dim < 3)
    return;

				   // the array into which we want to write
				   // should have the correct size already.
  Assert (this->adjust_quad_dof_index_for_face_orientation_table.n_elements()==
	  8*this->dofs_per_quad, ExcInternalError());

				   // to obtain the shifts for this composed
				   // element, copy the shift information of
				   // the base elements
  unsigned int index = 0;
  for (unsigned int b=0; b<this->n_base_elements();++b)
    {
      const Table<2,int> &temp
	= this->base_element(b).adjust_quad_dof_index_for_face_orientation_table;
      for (unsigned int c=0; c<this->element_multiplicity(b); ++c)
	{
	  for (unsigned int i=0; i<temp.size(0); ++i)
	    for (unsigned int j=0; j<8; ++j)
	      this->adjust_quad_dof_index_for_face_orientation_table(index+i,j)=
		temp(i,j);
	  index += temp.size(0);
	}
    }
  Assert (index == this->dofs_per_quad,
	  ExcInternalError());

				   // aditionally compose the permutation
				   // information for lines
  Assert (this->adjust_line_dof_index_for_line_orientation_table.size()==
	  this->dofs_per_line, ExcInternalError());
  index = 0;
  for (unsigned int b=0; b<this->n_base_elements();++b)
    {
      const std::vector<int> &temp2
	= this->base_element(b).adjust_line_dof_index_for_line_orientation_table;
      for (unsigned int c=0; c<this->element_multiplicity(b); ++c)
	{
	  std::copy(temp2.begin(), temp2.end(),
		    this->adjust_line_dof_index_for_line_orientation_table.begin()
		    +index);
	  index += temp2.size();
	}
    }
  Assert (index == this->dofs_per_line,
	  ExcInternalError());
}



template <int dim, int spacedim>
bool
FESystem<dim,spacedim>::
hp_constraints_are_implemented () const
{
  for (unsigned int b=0; b<this->n_base_elements(); ++b)
    if (base_element(b).hp_constraints_are_implemented() == false)
      return false;

  return true;
}



template <int dim, int spacedim>
void
FESystem<dim,spacedim>::
get_face_interpolation_matrix (const FiniteElement<dim,spacedim> &x_source_fe,
			       FullMatrix<double>       &interpolation_matrix) const
{
  typedef FiniteElement<dim,spacedim> FEL;
  AssertThrow ((x_source_fe.get_name().find ("FE_System<") == 0)
	       ||
	       (dynamic_cast<const FESystem<dim,spacedim>*>(&x_source_fe) != 0),
	       typename FEL::
	       ExcInterpolationNotImplemented());

  Assert (interpolation_matrix.n() == this->dofs_per_face,
	  ExcDimensionMismatch (interpolation_matrix.n(),
				this->dofs_per_face));
  Assert (interpolation_matrix.m() == x_source_fe.dofs_per_face,
	  ExcDimensionMismatch (interpolation_matrix.m(),
				x_source_fe.dofs_per_face));

				   // since dofs for each base are
				   // independent, we only have to stack
				   // things up from base element to base
				   // element
				   //
				   // the problem is that we have to work with
				   // two FEs (this and fe_other). only deal
				   // with the case that both are FESystems
				   // and that they both have the same number
				   // of bases (counting multiplicity) each of
				   // which match in their number of
				   // components. this covers
				   // FESystem(FE_Q(p),1,FE_Q(q),2) vs
				   // FESystem(FE_Q(r),2,FE_Q(s),1), but not
				   // FESystem(FE_Q(p),1,FE_Q(q),2) vs
				   // FESystem(FESystem(FE_Q(r),2),1,FE_Q(s),1)
  const FESystem<dim,spacedim> *fe_other_system
    = dynamic_cast<const FESystem<dim,spacedim>*>(&x_source_fe);

				   // clear matrix, since we will not get to
				   // set all elements
  interpolation_matrix = 0;

				     // loop over all the base elements of
				     // this and the other element, counting
				     // their multiplicities
  unsigned int base_index       = 0,
	       base_index_other = 0;
  unsigned int multiplicity       = 0,
	       multiplicity_other = 0;

  FullMatrix<double> base_to_base_interpolation;

  while (true)
    {
      const FiniteElement<dim,spacedim>
	&base       = base_element(base_index),
	&base_other = fe_other_system->base_element(base_index_other);

      Assert (base.n_components() == base_other.n_components(),
	      ExcNotImplemented());

				       // get the interpolation from the bases
      base_to_base_interpolation.reinit (base_other.dofs_per_face,
					 base.dofs_per_face);
      base.get_face_interpolation_matrix (base_other,
					  base_to_base_interpolation);

				       // now translate entries. we'd like to
				       // have something like
				       // face_base_to_system_index, but that
				       // doesn't exist. rather, all we have
				       // is the reverse. well, use that then
      for (unsigned int i=0; i<this->dofs_per_face; ++i)
	if (this->face_system_to_base_index(i).first
	    ==
	    std::make_pair (base_index, multiplicity))
	  for (unsigned int j=0; j<fe_other_system->dofs_per_face; ++j)
	    if (fe_other_system->face_system_to_base_index(j).first
		==
		std::make_pair (base_index_other, multiplicity_other))
	      interpolation_matrix(j, i)
		= base_to_base_interpolation(fe_other_system->face_system_to_base_index(j).second,
					     this->face_system_to_base_index(i).second);

				       // advance to the next base element
				       // for this and the other
				       // fe_system; see if we can simply
				       // advance the multiplicity by one,
				       // or if have to move on to the
				       // next base element
      ++multiplicity;
      if (multiplicity == this->element_multiplicity(base_index))
	{
	  multiplicity = 0;
	  ++base_index;
	}
      ++multiplicity_other;
      if (multiplicity_other ==
	  fe_other_system->element_multiplicity(base_index_other))
	{
	  multiplicity_other = 0;
	  ++base_index_other;
	}

				       // see if we have reached the end
				       // of the present element. if so,
				       // we should have reached the end
				       // of the other one as well
      if (base_index == this->n_base_elements())
	{
	  Assert (base_index_other == fe_other_system->n_base_elements(),
		  ExcInternalError());
	  break;
	}

				       // if we haven't reached the end of
				       // this element, we shouldn't have
				       // reached the end of the other one
				       // either
      Assert (base_index_other != fe_other_system->n_base_elements(),
	      ExcInternalError());
    }
}



template <int dim, int spacedim>
void
FESystem<dim,spacedim>::
get_subface_interpolation_matrix (const FiniteElement<dim,spacedim> &x_source_fe,
				  const unsigned int        subface,
				  FullMatrix<double>       &interpolation_matrix) const
{
  typedef FiniteElement<dim,spacedim> FEL;
  AssertThrow ((x_source_fe.get_name().find ("FE_System<") == 0)
	       ||
	       (dynamic_cast<const FESystem<dim,spacedim>*>(&x_source_fe) != 0),
	       typename FEL::
	       ExcInterpolationNotImplemented());

  Assert (interpolation_matrix.n() == this->dofs_per_face,
	  ExcDimensionMismatch (interpolation_matrix.n(),
				this->dofs_per_face));
  Assert (interpolation_matrix.m() == x_source_fe.dofs_per_face,
	  ExcDimensionMismatch (interpolation_matrix.m(),
				x_source_fe.dofs_per_face));

				   // since dofs for each base are
				   // independent, we only have to stack
				   // things up from base element to base
				   // element
				   //
				   // the problem is that we have to work with
				   // two FEs (this and fe_other). only deal
				   // with the case that both are FESystems
				   // and that they both have the same number
				   // of bases (counting multiplicity) each of
				   // which match in their number of
				   // components. this covers
				   // FESystem(FE_Q(p),1,FE_Q(q),2) vs
				   // FESystem(FE_Q(r),2,FE_Q(s),1), but not
				   // FESystem(FE_Q(p),1,FE_Q(q),2) vs
				   // FESystem(FESystem(FE_Q(r),2),1,FE_Q(s),1)
  const FESystem<dim,spacedim> *fe_other_system
    = dynamic_cast<const FESystem<dim,spacedim>*>(&x_source_fe);

				   // clear matrix, since we will not get to
				   // set all elements
  interpolation_matrix = 0;

				     // loop over all the base elements of
				     // this and the other element, counting
				     // their multiplicities
  unsigned int base_index       = 0,
	       base_index_other = 0;
  unsigned int multiplicity       = 0,
	       multiplicity_other = 0;

  FullMatrix<double> base_to_base_interpolation;

  while (true)
    {
      const FiniteElement<dim,spacedim>
	&base       = base_element(base_index),
	&base_other = fe_other_system->base_element(base_index_other);

      Assert (base.n_components() == base_other.n_components(),
	      ExcNotImplemented());

				       // get the interpolation from the bases
      base_to_base_interpolation.reinit (base_other.dofs_per_face,
					 base.dofs_per_face);
      base.get_subface_interpolation_matrix (base_other,
					     subface,
					     base_to_base_interpolation);

				       // now translate entries. we'd like to
				       // have something like
				       // face_base_to_system_index, but that
				       // doesn't exist. rather, all we have
				       // is the reverse. well, use that then
      for (unsigned int i=0; i<this->dofs_per_face; ++i)
	if (this->face_system_to_base_index(i).first
	    ==
	    std::make_pair (base_index, multiplicity))
	  for (unsigned int j=0; j<fe_other_system->dofs_per_face; ++j)
	    if (fe_other_system->face_system_to_base_index(j).first
		==
		std::make_pair (base_index_other, multiplicity_other))
	      interpolation_matrix(j, i)
		= base_to_base_interpolation(fe_other_system->face_system_to_base_index(j).second,
					     this->face_system_to_base_index(i).second);

				       // advance to the next base element
				       // for this and the other
				       // fe_system; see if we can simply
				       // advance the multiplicity by one,
				       // or if have to move on to the
				       // next base element
      ++multiplicity;
      if (multiplicity == this->element_multiplicity(base_index))
	{
	  multiplicity = 0;
	  ++base_index;
	}
      ++multiplicity_other;
      if (multiplicity_other ==
	  fe_other_system->element_multiplicity(base_index_other))
	{
	  multiplicity_other = 0;
	  ++base_index_other;
	}

				       // see if we have reached the end
				       // of the present element. if so,
				       // we should have reached the end
				       // of the other one as well
      if (base_index == this->n_base_elements())
	{
	  Assert (base_index_other == fe_other_system->n_base_elements(),
		  ExcInternalError());
	  break;
	}

				       // if we haven't reached the end of
				       // this element, we shouldn't have
				       // reached the end of the other one
				       // either
      Assert (base_index_other != fe_other_system->n_base_elements(),
	      ExcInternalError());
    }
}



template <int dim, int spacedim>
template <int structdim>
std::vector<std::pair<unsigned int, unsigned int> >
FESystem<dim,spacedim>::hp_object_dof_identities (const FiniteElement<dim,spacedim> &fe_other) const
{
				   // since dofs on each subobject (vertex,
				   // line, ...) are ordered such that first
				   // come all from the first base element all
				   // multiplicities, then second base element
				   // all multiplicities, etc., we simply have
				   // to stack all the identities after each
				   // other
				   //
				   // the problem is that we have to work with
				   // two FEs (this and fe_other). only deal
				   // with the case that both are FESystems
				   // and that they both have the same number
				   // of bases (counting multiplicity) each of
				   // which match in their number of
				   // components. this covers
				   // FESystem(FE_Q(p),1,FE_Q(q),2) vs
				   // FESystem(FE_Q(r),2,FE_Q(s),1), but not
				   // FESystem(FE_Q(p),1,FE_Q(q),2) vs
				   // FESystem(FESystem(FE_Q(r),2),1,FE_Q(s),1)
  if (const FESystem<dim,spacedim> *fe_other_system
      = dynamic_cast<const FESystem<dim,spacedim>*>(&fe_other))
    {
				       // loop over all the base elements of
				       // this and the other element, counting
				       // their multiplicities
      unsigned int base_index       = 0,
		   base_index_other = 0;
      unsigned int multiplicity       = 0,
		   multiplicity_other = 0;

				       // we also need to keep track of the
				       // number of dofs already treated for
				       // each of the elements
      unsigned int dof_offset       = 0,
		   dof_offset_other = 0;

      std::vector<std::pair<unsigned int, unsigned int> > identities;

      while (true)
	{
	  const FiniteElement<dim,spacedim>
	    &base       = base_element(base_index),
	    &base_other = fe_other_system->base_element(base_index_other);

	  Assert (base.n_components() == base_other.n_components(),
		  ExcNotImplemented());

					   // now translate the identities
					   // returned by the base elements to
					   // the indices of this system
					   // element
	  std::vector<std::pair<unsigned int, unsigned int> > base_identities;
	  switch (structdim)
	    {
	      case 0:
		    base_identities = base.hp_vertex_dof_identities (base_other);
		    break;
	      case 1:
		    base_identities = base.hp_line_dof_identities (base_other);
		    break;
	      case 2:
		    base_identities = base.hp_quad_dof_identities (base_other);
		    break;
	      default:
		    Assert (false, ExcNotImplemented());
	    }

	  for (unsigned int i=0; i<base_identities.size(); ++i)
	    identities.push_back
	      (std::make_pair (base_identities[i].first + dof_offset,
			       base_identities[i].second + dof_offset_other));

					   // record the dofs treated above as
					   // already taken care of
	  dof_offset       += base.template n_dofs_per_object<structdim>();
	  dof_offset_other += base_other.template n_dofs_per_object<structdim>();

					   // advance to the next base element
					   // for this and the other
					   // fe_system; see if we can simply
					   // advance the multiplicity by one,
					   // or if have to move on to the
					   // next base element
	  ++multiplicity;
	  if (multiplicity == this->element_multiplicity(base_index))
	    {
	      multiplicity = 0;
	      ++base_index;
	    }
	  ++multiplicity_other;
	  if (multiplicity_other ==
	      fe_other_system->element_multiplicity(base_index_other))
	    {
	      multiplicity_other = 0;
	      ++base_index_other;
	    }

					   // see if we have reached the end
					   // of the present element. if so,
					   // we should have reached the end
					   // of the other one as well
	  if (base_index == this->n_base_elements())
	    {
	      Assert (base_index_other == fe_other_system->n_base_elements(),
		      ExcInternalError());
	      break;
	    }

					   // if we haven't reached the end of
					   // this element, we shouldn't have
					   // reached the end of the other one
					   // either
	  Assert (base_index_other != fe_other_system->n_base_elements(),
		  ExcInternalError());
	}

      return identities;
    }
  else
    {
      Assert (false, ExcNotImplemented());
      return std::vector<std::pair<unsigned int, unsigned int> >();
    }
}



template <int dim, int spacedim>
std::vector<std::pair<unsigned int, unsigned int> >
FESystem<dim,spacedim>::hp_vertex_dof_identities (const FiniteElement<dim,spacedim> &fe_other) const
{
  return hp_object_dof_identities<0> (fe_other);
}

template <int dim, int spacedim>
std::vector<std::pair<unsigned int, unsigned int> >
FESystem<dim,spacedim>::hp_line_dof_identities (const FiniteElement<dim,spacedim> &fe_other) const
{
  return hp_object_dof_identities<1> (fe_other);
}



template <int dim, int spacedim>
std::vector<std::pair<unsigned int, unsigned int> >
FESystem<dim,spacedim>::hp_quad_dof_identities (const FiniteElement<dim,spacedim> &fe_other) const
{
  return hp_object_dof_identities<2> (fe_other);
}



template <int dim, int spacedim>
FiniteElementDomination::Domination
FESystem<dim,spacedim>::
compare_for_face_domination (const FiniteElement<dim,spacedim> &fe_other) const
{
				   // at present all we can do is to compare
				   // with other FESystems that have the same
				   // number of components and bases
  if (const FESystem<dim,spacedim> *fe_sys_other
      = dynamic_cast<const FESystem<dim,spacedim>*>(&fe_other))
    {
      Assert (this->n_components() == fe_sys_other->n_components(),
	      ExcNotImplemented());
      Assert (this->n_base_elements() == fe_sys_other->n_base_elements(),
	      ExcNotImplemented());

      FiniteElementDomination::Domination
	domination = FiniteElementDomination::no_requirements;

				       // loop over all base elements and do
				       // some sanity checks
      for (unsigned int b=0; b<this->n_base_elements(); ++b)
	{
	  Assert (this->base_element(b).n_components() ==
		  fe_sys_other->base_element(b).n_components(),
		  ExcNotImplemented());
	  Assert (this->element_multiplicity(b) ==
		  fe_sys_other->element_multiplicity(b),
		  ExcNotImplemented());

					   // for this pair of base elements,
					   // check who dominates and combine
					   // with previous result
	  const FiniteElementDomination::Domination
	    base_domination = (this->base_element(b)
			       .compare_for_face_domination (fe_sys_other->base_element(b)));
	  domination = domination & base_domination;
	}

				       // if we've gotten here, then we've
				       // either found a winner or either
				       // element is fine being dominated
      Assert (domination !=
	      FiniteElementDomination::neither_element_dominates,
	      ExcInternalError());

      return domination;
    }

  Assert (false, ExcNotImplemented());
  return FiniteElementDomination::neither_element_dominates;
}



template <int dim, int spacedim>
FiniteElementData<dim>
FESystem<dim,spacedim>::multiply_dof_numbers (const FiniteElementData<dim> &fe1,
				     const unsigned int            N1,
				     const FiniteElementData<dim> &fe2,
				     const unsigned int            N2)
{
  std::vector<unsigned int> dpo;
  dpo.push_back(fe1.dofs_per_vertex * N1 + fe2.dofs_per_vertex * N2);
  dpo.push_back(fe1.dofs_per_line * N1 + fe2.dofs_per_line * N2);
  if (dim>1) dpo.push_back(fe1.dofs_per_quad * N1 + fe2.dofs_per_quad * N2);
  if (dim>2) dpo.push_back(fe1.dofs_per_hex * N1 + fe2.dofs_per_hex * N2);

  				   // degree is the maximal degree of
				   // the components.  max also makes
				   // sure that one unknown degree
				   // makes the degree of the system
				   // unknown.
  unsigned int degree = std::max(fe1.tensor_degree(), fe2.tensor_degree());
  return FiniteElementData<dim> (
    dpo,
    fe1.n_components() * N1 + fe2.n_components() * N2,
    degree,
    typename FiniteElementData<dim>::Conformity(fe1.conforming_space
						& fe2.conforming_space),
    N1 + N2);
}



template <int dim, int spacedim>
FiniteElementData<dim>
FESystem<dim,spacedim>::multiply_dof_numbers (const FiniteElementData<dim> &fe1,
				     const unsigned int            N1,
				     const FiniteElementData<dim> &fe2,
				     const unsigned int            N2,
				     const FiniteElementData<dim> &fe3,
				     const unsigned int            N3)
{
  std::vector<unsigned int> dpo;
  dpo.push_back(fe1.dofs_per_vertex * N1 +
		fe2.dofs_per_vertex * N2 +
		fe3.dofs_per_vertex * N3);
  dpo.push_back(fe1.dofs_per_line * N1 +
		fe2.dofs_per_line * N2 +
		fe3.dofs_per_line * N3);
  if (dim>1) dpo.push_back(fe1.dofs_per_quad * N1 +
			   fe2.dofs_per_quad * N2 +
			   fe3.dofs_per_quad * N3);
  if (dim>2) dpo.push_back(fe1.dofs_per_hex * N1 +
			   fe2.dofs_per_hex * N2 +
			   fe3.dofs_per_hex * N3);
				   // degree is the maximal degree of the components
  const unsigned int
    degree = std::max (std::max(fe1.tensor_degree(),
				fe2.tensor_degree()),
		       fe3.tensor_degree());
  return FiniteElementData<dim> (
    dpo,
    fe1.n_components() * N1 + fe2.n_components() * N2 + fe3.n_components() * N3,
    degree,
    typename FiniteElementData<dim>::Conformity(fe1.conforming_space
						& fe2.conforming_space
						& fe3.conforming_space),
    N1 + N2 + N3);
}



template <int dim, int spacedim>
FiniteElementData<dim>
FESystem<dim,spacedim>::multiply_dof_numbers (const FiniteElementData<dim> &fe1,
					      const unsigned int            N1,
					      const FiniteElementData<dim> &fe2,
					      const unsigned int            N2,
					      const FiniteElementData<dim> &fe3,
					      const unsigned int            N3,
					      const FiniteElementData<dim> &fe4,
					      const unsigned int            N4)
{
  std::vector<unsigned int> dpo;
  dpo.push_back(fe1.dofs_per_vertex * N1 +
		fe2.dofs_per_vertex * N2 +
		fe3.dofs_per_vertex * N3 +
		fe4.dofs_per_vertex * N4);
  dpo.push_back(fe1.dofs_per_line * N1 +
		fe2.dofs_per_line * N2 +
		fe3.dofs_per_line * N3 +
		fe4.dofs_per_line * N4);
  if (dim>1) dpo.push_back(fe1.dofs_per_quad * N1 +
			   fe2.dofs_per_quad * N2 +
			   fe3.dofs_per_quad * N3 +
			   fe4.dofs_per_quad * N4);
  if (dim>2) dpo.push_back(fe1.dofs_per_hex * N1 +
			   fe2.dofs_per_hex * N2 +
			   fe3.dofs_per_hex * N3 +
			   fe4.dofs_per_hex * N4);
				   // degree is the maximal degree of the components
  const unsigned int
    degree = std::max(std::max (std::max(fe1.tensor_degree(),fe2.tensor_degree()),fe3.tensor_degree()), fe4.tensor_degree());

  return FiniteElementData<dim> (
    dpo,
    fe1.n_components() * N1 + fe2.n_components() * N2 + fe3.n_components() * N3 +
    fe4.n_components() * N4,
    degree,
    typename FiniteElementData<dim>::Conformity(fe1.conforming_space
						& fe2.conforming_space
						& fe3.conforming_space
						& fe4.conforming_space),
    N1 + N2 + N3 + N4);
}


template <int dim, int spacedim>
FiniteElementData<dim>
FESystem<dim,spacedim>::multiply_dof_numbers (const FiniteElementData<dim> &fe1,
					      const unsigned int            N1,
					      const FiniteElementData<dim> &fe2,
					      const unsigned int            N2,
					      const FiniteElementData<dim> &fe3,
					      const unsigned int            N3,
					      const FiniteElementData<dim> &fe4,
					      const unsigned int            N4,
					      const FiniteElementData<dim> &fe5,
					      const unsigned int            N5)
{
  std::vector<unsigned int> dpo;
  dpo.push_back(fe1.dofs_per_vertex * N1 +
		fe2.dofs_per_vertex * N2 +
		fe3.dofs_per_vertex * N3 +
		fe4.dofs_per_vertex * N4 +
		fe5.dofs_per_vertex * N5);
  dpo.push_back(fe1.dofs_per_line * N1 +
		fe2.dofs_per_line * N2 +
		fe3.dofs_per_line * N3 +
		fe4.dofs_per_line * N4 +
		fe5.dofs_per_line * N5);
  if (dim>1) dpo.push_back(fe1.dofs_per_quad * N1 +
			   fe2.dofs_per_quad * N2 +
			   fe3.dofs_per_quad * N3 +
			   fe4.dofs_per_quad * N4 +
			   fe5.dofs_per_quad * N5);
  if (dim>2) dpo.push_back(fe1.dofs_per_hex * N1 +
			   fe2.dofs_per_hex * N2 +
			   fe3.dofs_per_hex * N3 +
			   fe4.dofs_per_hex * N4 +
			   fe5.dofs_per_hex * N5);
				   // degree is the maximal degree of the components
  const unsigned int
    degree = std::max(std::max(std::max (std::max(fe1.tensor_degree(),fe2.tensor_degree()),fe3.tensor_degree()), fe4.tensor_degree()), fe5.tensor_degree());

  return FiniteElementData<dim> (
    dpo,
    fe1.n_components() * N1 + fe2.n_components() * N2 + fe3.n_components() * N3 +
    fe4.n_components() * N4 + fe5.n_components() * N5,
    degree,
    typename FiniteElementData<dim>::Conformity(fe1.conforming_space
						& fe2.conforming_space
						& fe3.conforming_space
						& fe4.conforming_space
						& fe5.conforming_space),
    N1 + N2 + N3 + N4 + N5);
}


template <int dim, int spacedim>
std::vector<bool>
FESystem<dim,spacedim>::compute_restriction_is_additive_flags (const FiniteElement<dim,spacedim> &fe,
						      const unsigned int n_elements)
{
  std::vector<const FiniteElement<dim,spacedim>*> fe_list;
  std::vector<unsigned int>              multiplicities;

  fe_list.push_back (&fe);
  multiplicities.push_back (n_elements);

  return compute_restriction_is_additive_flags (fe_list, multiplicities);
}



template <int dim, int spacedim>
std::vector<bool>
FESystem<dim,spacedim>::compute_restriction_is_additive_flags (const FiniteElement<dim,spacedim> &fe1,
						      const unsigned int        N1,
						      const FiniteElement<dim,spacedim> &fe2,
						      const unsigned int        N2)
{
  std::vector<const FiniteElement<dim,spacedim>*> fe_list;
  std::vector<unsigned int>              multiplicities;

  fe_list.push_back (&fe1);
  multiplicities.push_back (N1);

  fe_list.push_back (&fe2);
  multiplicities.push_back (N2);

  return compute_restriction_is_additive_flags (fe_list, multiplicities);
}



template <int dim, int spacedim>
std::vector<bool>
FESystem<dim,spacedim>::compute_restriction_is_additive_flags (const FiniteElement<dim,spacedim> &fe1,
						      const unsigned int        N1,
						      const FiniteElement<dim,spacedim> &fe2,
						      const unsigned int        N2,
						      const FiniteElement<dim,spacedim> &fe3,
						      const unsigned int        N3)
{
  std::vector<const FiniteElement<dim,spacedim>*> fe_list;
  std::vector<unsigned int>              multiplicities;

  fe_list.push_back (&fe1);
  multiplicities.push_back (N1);

  fe_list.push_back (&fe2);
  multiplicities.push_back (N2);

  fe_list.push_back (&fe3);
  multiplicities.push_back (N3);

  return compute_restriction_is_additive_flags (fe_list, multiplicities);
}


template <int dim, int spacedim>
std::vector<bool>
FESystem<dim,spacedim>::compute_restriction_is_additive_flags (const FiniteElement<dim,spacedim> &fe1,
							       const unsigned int        N1,
							       const FiniteElement<dim,spacedim> &fe2,
							       const unsigned int        N2,
							       const FiniteElement<dim,spacedim> &fe3,
							       const unsigned int        N3,
							       const FiniteElement<dim,spacedim> &fe4,
							       const unsigned int        N4)
{
  std::vector<const FiniteElement<dim,spacedim>*> fe_list;
  std::vector<unsigned int>              multiplicities;

  fe_list.push_back (&fe1);
  multiplicities.push_back (N1);

  fe_list.push_back (&fe2);
  multiplicities.push_back (N2);

  fe_list.push_back (&fe3);
  multiplicities.push_back (N3);

  fe_list.push_back (&fe4);
  multiplicities.push_back (N4);

  return compute_restriction_is_additive_flags (fe_list, multiplicities);
}


template <int dim, int spacedim>
std::vector<bool>
FESystem<dim,spacedim>::compute_restriction_is_additive_flags (const FiniteElement<dim,spacedim> &fe1,
							       const unsigned int        N1,
							       const FiniteElement<dim,spacedim> &fe2,
							       const unsigned int        N2,
							       const FiniteElement<dim,spacedim> &fe3,
							       const unsigned int        N3,
							       const FiniteElement<dim,spacedim> &fe4,
							       const unsigned int        N4,
							       const FiniteElement<dim,spacedim> &fe5,
							       const unsigned int        N5)
{
  std::vector<const FiniteElement<dim,spacedim>*> fe_list;
  std::vector<unsigned int>              multiplicities;

  fe_list.push_back (&fe1);
  multiplicities.push_back (N1);

  fe_list.push_back (&fe2);
  multiplicities.push_back (N2);

  fe_list.push_back (&fe3);
  multiplicities.push_back (N3);

  fe_list.push_back (&fe4);
  multiplicities.push_back (N4);

  fe_list.push_back (&fe5);
  multiplicities.push_back (N5);
  return compute_restriction_is_additive_flags (fe_list, multiplicities);
}


template <int dim, int spacedim>
std::vector<bool>
FESystem<dim,spacedim>::
compute_restriction_is_additive_flags (const std::vector<const FiniteElement<dim,spacedim>*> &fes,
                                       const std::vector<unsigned int>              &multiplicities)
{
  Assert (fes.size() == multiplicities.size(), ExcInternalError());

				   // first count the number of dofs
				   // and components that will emerge
				   // from the given FEs
  unsigned int n_shape_functions = 0;
  for (unsigned int i=0; i<fes.size(); ++i)
    n_shape_functions += fes[i]->dofs_per_cell * multiplicities[i];

				   // generate the array that will
				   // hold the output
  std::vector<bool> retval (n_shape_functions, false);

				   // finally go through all the shape
				   // functions of the base elements,
				   // and copy their flags. this
				   // somehow copies the code in
				   // build_cell_table, which is not
				   // nice as it uses too much
				   // implicit knowledge about the
				   // layout of the individual bases
				   // in the composed FE, but there
				   // seems no way around...
				   //
				   // for each shape function, copy
				   // the flags from the base element
				   // to this one, taking into account
				   // multiplicities, and other
				   // complications
  unsigned int total_index = 0;
  for (unsigned int vertex_number=0;
       vertex_number<GeometryInfo<dim>::vertices_per_cell;
       ++vertex_number)
    {
      for(unsigned int base=0; base<fes.size(); ++base)
	for (unsigned int m=0; m<multiplicities[base]; ++m)
	  for (unsigned int local_index = 0;
	       local_index < fes[base]->dofs_per_vertex;
	       ++local_index, ++total_index)
	    {
	      const unsigned int index_in_base
		= (fes[base]->dofs_per_vertex*vertex_number +
		   local_index);

              Assert (index_in_base < fes[base]->dofs_per_cell,
                      ExcInternalError());
              retval[total_index] = fes[base]->restriction_is_additive(index_in_base);
	    }
    }

				   // 2. Lines
  if (GeometryInfo<dim>::lines_per_cell > 0)
    for (unsigned int line_number= 0;
	 line_number != GeometryInfo<dim>::lines_per_cell;
	 ++line_number)
      {
	for (unsigned int base=0; base<fes.size(); ++base)
	  for (unsigned int m=0; m<multiplicities[base]; ++m)
	    for (unsigned int local_index = 0;
		 local_index < fes[base]->dofs_per_line;
		 ++local_index, ++total_index)
	      {
		const unsigned int index_in_base
		  = (fes[base]->dofs_per_line*line_number +
		     local_index +
		     fes[base]->first_line_index);

                Assert (index_in_base < fes[base]->dofs_per_cell,
                        ExcInternalError());
                retval[total_index] = fes[base]->restriction_is_additive(index_in_base);
	      }
      }

				   // 3. Quads
  if (GeometryInfo<dim>::quads_per_cell > 0)
    for (unsigned int quad_number= 0;
	 quad_number != GeometryInfo<dim>::quads_per_cell;
	 ++quad_number)
      {
	for (unsigned int base=0; base<fes.size(); ++base)
	  for (unsigned int m=0; m<multiplicities[base]; ++m)
	    for (unsigned int local_index = 0;
		 local_index < fes[base]->dofs_per_quad;
		 ++local_index, ++total_index)
	      {
		const unsigned int index_in_base
		  = (fes[base]->dofs_per_quad*quad_number +
		     local_index +
		     fes[base]->first_quad_index);

                Assert (index_in_base < fes[base]->dofs_per_cell,
                        ExcInternalError());
                retval[total_index] = fes[base]->restriction_is_additive(index_in_base);
	      }
      }

				   // 4. Hexes
  if (GeometryInfo<dim>::hexes_per_cell > 0)
    for (unsigned int hex_number= 0;
	 hex_number != GeometryInfo<dim>::hexes_per_cell;
	 ++hex_number)
      {
	for(unsigned int base=0; base<fes.size(); ++base)
	  for (unsigned int m=0; m<multiplicities[base]; ++m)
	    for (unsigned int local_index = 0;
		 local_index < fes[base]->dofs_per_hex;
		 ++local_index, ++total_index)
	      {
		const unsigned int index_in_base
		  = (fes[base]->dofs_per_hex*hex_number +
		     local_index +
		     fes[base]->first_hex_index);

                Assert (index_in_base < fes[base]->dofs_per_cell,
                        ExcInternalError());
                retval[total_index] = fes[base]->restriction_is_additive(index_in_base);
	      }
      }

  Assert (total_index == n_shape_functions, ExcInternalError());

  return retval;
}



template <int dim, int spacedim>
std::vector<std::vector<bool> >
FESystem<dim,spacedim>::compute_nonzero_components (const FiniteElement<dim,spacedim> &fe1,
					   const unsigned int        N1)
{
  std::vector<const FiniteElement<dim,spacedim>*> fe_list;
  std::vector<unsigned int>              multiplicities;

  fe_list.push_back (&fe1);
  multiplicities.push_back (N1);

  return compute_nonzero_components (fe_list, multiplicities);
}



template <int dim, int spacedim>
std::vector<std::vector<bool> >
FESystem<dim,spacedim>::compute_nonzero_components (const FiniteElement<dim,spacedim> &fe1,
					   const unsigned int        N1,
					   const FiniteElement<dim,spacedim> &fe2,
					   const unsigned int        N2)
{
  std::vector<const FiniteElement<dim,spacedim>*> fe_list;
  std::vector<unsigned int>              multiplicities;

  fe_list.push_back (&fe1);
  multiplicities.push_back (N1);

  fe_list.push_back (&fe2);
  multiplicities.push_back (N2);

  return compute_nonzero_components (fe_list, multiplicities);
}



template <int dim, int spacedim>
std::vector<std::vector<bool> >
FESystem<dim,spacedim>::compute_nonzero_components (const FiniteElement<dim,spacedim> &fe1,
					   const unsigned int        N1,
					   const FiniteElement<dim,spacedim> &fe2,
					   const unsigned int        N2,
					   const FiniteElement<dim,spacedim> &fe3,
					   const unsigned int        N3)
{
  std::vector<const FiniteElement<dim,spacedim>*> fe_list;
  std::vector<unsigned int>              multiplicities;

  fe_list.push_back (&fe1);
  multiplicities.push_back (N1);

  fe_list.push_back (&fe2);
  multiplicities.push_back (N2);

  fe_list.push_back (&fe3);
  multiplicities.push_back (N3);

  return compute_nonzero_components (fe_list, multiplicities);
}



template <int dim, int spacedim>
std::vector<std::vector<bool> >
FESystem<dim,spacedim>::compute_nonzero_components (const FiniteElement<dim,spacedim> &fe1,
						    const unsigned int        N1,
						    const FiniteElement<dim,spacedim> &fe2,
						    const unsigned int        N2,
						    const FiniteElement<dim,spacedim> &fe3,
						    const unsigned int        N3,
						    const FiniteElement<dim,spacedim> &fe4,
						    const unsigned int        N4)
{
  std::vector<const FiniteElement<dim,spacedim>*> fe_list;
  std::vector<unsigned int>              multiplicities;

  fe_list.push_back (&fe1);
  multiplicities.push_back (N1);

  fe_list.push_back (&fe2);
  multiplicities.push_back (N2);

  fe_list.push_back (&fe3);
  multiplicities.push_back (N3);

  fe_list.push_back (&fe4);
  multiplicities.push_back (N4);

  return compute_nonzero_components (fe_list, multiplicities);
}



template <int dim, int spacedim>
std::vector<std::vector<bool> >
FESystem<dim,spacedim>::compute_nonzero_components (const FiniteElement<dim,spacedim> &fe1,
						    const unsigned int        N1,
						    const FiniteElement<dim,spacedim> &fe2,
						    const unsigned int        N2,
						    const FiniteElement<dim,spacedim> &fe3,
						    const unsigned int        N3,
						    const FiniteElement<dim,spacedim> &fe4,
						    const unsigned int        N4,
						    const FiniteElement<dim,spacedim> &fe5,
						    const unsigned int        N5)
{
  std::vector<const FiniteElement<dim,spacedim>*> fe_list;
  std::vector<unsigned int>              multiplicities;

  fe_list.push_back (&fe1);
  multiplicities.push_back (N1);

  fe_list.push_back (&fe2);
  multiplicities.push_back (N2);

  fe_list.push_back (&fe3);
  multiplicities.push_back (N3);

  fe_list.push_back (&fe4);
  multiplicities.push_back (N4);

  fe_list.push_back (&fe5);
  multiplicities.push_back (N5);

  return compute_nonzero_components (fe_list, multiplicities);
}



template <int dim, int spacedim>
std::vector<std::vector<bool> >
FESystem<dim,spacedim>::
compute_nonzero_components (const std::vector<const FiniteElement<dim,spacedim>*> &fes,
			    const std::vector<unsigned int>              &multiplicities)
{
  Assert (fes.size() == multiplicities.size(), ExcInternalError());

				   // first count the number of dofs
				   // and components that will emerge
				   // from the given FEs
  unsigned int n_shape_functions = 0;
  for (unsigned int i=0; i<fes.size(); ++i)
    n_shape_functions += fes[i]->dofs_per_cell * multiplicities[i];

  unsigned int n_components = 0;
  for (unsigned int i=0; i<fes.size(); ++i)
    n_components += fes[i]->n_components() * multiplicities[i];

				   // generate the array that will
				   // hold the output
  std::vector<std::vector<bool> >
    retval (n_shape_functions, std::vector<bool> (n_components, false));

				   // finally go through all the shape
				   // functions of the base elements,
				   // and copy their flags. this
				   // somehow copies the code in
				   // build_cell_table, which is not
				   // nice as it uses too much
				   // implicit knowledge about the
				   // layout of the individual bases
				   // in the composed FE, but there
				   // seems no way around...
				   //
				   // for each shape function, copy
				   // the non-zero flags from the base
				   // element to this one, taking into
				   // account multiplicities, multiple
				   // components in base elements, and
				   // other complications
  unsigned int total_index = 0;
  for (unsigned int vertex_number=0;
       vertex_number<GeometryInfo<dim>::vertices_per_cell;
       ++vertex_number)
    {
      unsigned comp_start = 0;
      for(unsigned int base=0; base<fes.size(); ++base)
	for (unsigned int m=0; m<multiplicities[base];
	     ++m, comp_start+=fes[base]->n_components())
	  for (unsigned int local_index = 0;
	       local_index < fes[base]->dofs_per_vertex;
	       ++local_index, ++total_index)
	    {
	      const unsigned int index_in_base
		= (fes[base]->dofs_per_vertex*vertex_number +
		   local_index);

	      Assert (comp_start+fes[base]->n_components() <=
		      retval[total_index].size(),
		      ExcInternalError());
	      for (unsigned int c=0; c<fes[base]->n_components(); ++c)
		{
		  Assert (index_in_base < fes[base]->nonzero_components.size(),
			  ExcInternalError());
		  Assert (c < fes[base]->nonzero_components[index_in_base].size(),
			  ExcInternalError());
		  retval[total_index][comp_start+c]
		    = fes[base]->nonzero_components[index_in_base][c];
		};
	    }
    }

				   // 2. Lines
  if (GeometryInfo<dim>::lines_per_cell > 0)
    for (unsigned int line_number= 0;
	 line_number != GeometryInfo<dim>::lines_per_cell;
	 ++line_number)
      {
	unsigned comp_start = 0;
	for (unsigned int base=0; base<fes.size(); ++base)
	  for (unsigned int m=0; m<multiplicities[base];
	       ++m, comp_start+=fes[base]->n_components())
	    for (unsigned int local_index = 0;
		 local_index < fes[base]->dofs_per_line;
		 ++local_index, ++total_index)
	      {
		const unsigned int index_in_base
		  = (fes[base]->dofs_per_line*line_number +
		     local_index +
		     fes[base]->first_line_index);

		Assert (comp_start+fes[base]->n_components() <=
			retval[total_index].size(),
			ExcInternalError());
		for (unsigned int c=0; c<fes[base]->n_components(); ++c)
		  {
		    Assert (index_in_base < fes[base]->nonzero_components.size(),
			    ExcInternalError());
		    Assert (c < fes[base]->nonzero_components[index_in_base].size(),
			    ExcInternalError());
		    retval[total_index][comp_start+c]
		      = fes[base]->nonzero_components[index_in_base][c];
		  };
	      }
      }

				   // 3. Quads
  if (GeometryInfo<dim>::quads_per_cell > 0)
    for (unsigned int quad_number= 0;
	 quad_number != GeometryInfo<dim>::quads_per_cell;
	 ++quad_number)
      {
	unsigned int comp_start = 0;
	for (unsigned int base=0; base<fes.size(); ++base)
	  for (unsigned int m=0; m<multiplicities[base];
	       ++m, comp_start+=fes[base]->n_components())
	    for (unsigned int local_index = 0;
		 local_index < fes[base]->dofs_per_quad;
		 ++local_index, ++total_index)
	      {
		const unsigned int index_in_base
		  = (fes[base]->dofs_per_quad*quad_number +
		     local_index +
		     fes[base]->first_quad_index);

		Assert (comp_start+fes[base]->n_components() <=
			retval[total_index].size(),
			ExcInternalError());
		for (unsigned int c=0; c<fes[base]->n_components(); ++c)
		  {
		    Assert (index_in_base < fes[base]->nonzero_components.size(),
			    ExcInternalError());
		    Assert (c < fes[base]->nonzero_components[index_in_base].size(),
			    ExcInternalError());
		    retval[total_index][comp_start+c]
		      = fes[base]->nonzero_components[index_in_base][c];
		  };
	      }
      }

				   // 4. Hexes
  if (GeometryInfo<dim>::hexes_per_cell > 0)
    for (unsigned int hex_number= 0;
	 hex_number != GeometryInfo<dim>::hexes_per_cell;
	 ++hex_number)
      {
	unsigned int comp_start = 0;
	for(unsigned int base=0; base<fes.size(); ++base)
	  for (unsigned int m=0; m<multiplicities[base];
	       ++m, comp_start+=fes[base]->n_components())
	    for (unsigned int local_index = 0;
		 local_index < fes[base]->dofs_per_hex;
		 ++local_index, ++total_index)
	      {
		const unsigned int index_in_base
		  = (fes[base]->dofs_per_hex*hex_number +
		     local_index +
		     fes[base]->first_hex_index);

		Assert (comp_start+fes[base]->n_components() <=
			retval[total_index].size(),
			ExcInternalError());
		for (unsigned int c=0; c<fes[base]->n_components(); ++c)
		  {
		    Assert (index_in_base < fes[base]->nonzero_components.size(),
			    ExcInternalError());
		    Assert (c < fes[base]->nonzero_components[index_in_base].size(),
			    ExcInternalError());
		    retval[total_index][comp_start+c]
		      = fes[base]->nonzero_components[index_in_base][c];
		  };
	      }
      }

  Assert (total_index == n_shape_functions, ExcInternalError());

  return retval;
}




template <int dim, int spacedim>
const FiniteElement<dim,spacedim> &
FESystem<dim,spacedim>::base_element (const unsigned int index) const
{
  Assert (index < base_elements.size(),
	  ExcIndexRange(index, 0, base_elements.size()));
  return *base_elements[index].first;
}



template <int dim, int spacedim>
bool
FESystem<dim,spacedim>::has_support_on_face (const unsigned int shape_index,
				    const unsigned int face_index) const
{
  return (base_element(this->system_to_base_index(shape_index).first.first)
          .has_support_on_face(this->system_to_base_index(shape_index).second,
                               face_index));
}



template <int dim, int spacedim>
Point<dim>
FESystem<dim,spacedim>::unit_support_point (const unsigned index) const
{
  Assert (index < this->dofs_per_cell,
          ExcIndexRange (index, 0, this->dofs_per_cell));
  typedef FiniteElement<dim,spacedim> FEL;
  Assert ((this->unit_support_points.size() == this->dofs_per_cell) ||
          (this->unit_support_points.size() == 0),
          typename FEL::ExcFEHasNoSupportPoints ());

                                   // let's see whether we have the
                                   // information pre-computed
  if (this->unit_support_points.size() != 0)
    return this->unit_support_points[index];
  else
                                     // no. ask the base element
                                     // whether it would like to
                                     // provide this information
    return (base_element(this->system_to_base_index(index).first.first)
            .unit_support_point(this->system_to_base_index(index).second));
}



template <int dim, int spacedim>
Point<dim-1>
FESystem<dim,spacedim>::unit_face_support_point (const unsigned index) const
{
  Assert (index < this->dofs_per_face,
          ExcIndexRange (index, 0, this->dofs_per_face));
  typedef  FiniteElement<dim,spacedim> FEL;
  Assert ((this->unit_face_support_points.size() == this->dofs_per_face) ||
          (this->unit_face_support_points.size() == 0),
          typename FEL::ExcFEHasNoSupportPoints ());

                                   // let's see whether we have the
                                   // information pre-computed
  if (this->unit_face_support_points.size() != 0)
    return this->unit_face_support_points[index];
  else
                                     // no. ask the base element
                                     // whether it would like to
                                     // provide this information
    return (base_element(this->face_system_to_base_index(index).first.first)
            .unit_face_support_point(this->face_system_to_base_index(index).second));
}



template <int dim, int spacedim>
std::size_t
FESystem<dim,spacedim>::memory_consumption () const
{
				   // neglect size of data stored in
				   // @p{base_elements} due to some
				   // problems with teh
				   // compiler. should be neglectable
				   // after all, considering the size
				   // of the data of the subelements
  std::size_t mem = (FiniteElement<dim,spacedim>::memory_consumption () +
		     sizeof (base_elements));
  for (unsigned int i=0; i<base_elements.size(); ++i)
    mem += MemoryConsumption::memory_consumption (*base_elements[i].first);
  return mem;
}




// explicit instantiations
#include "fe_system.inst"

DEAL_II_NAMESPACE_CLOSE
