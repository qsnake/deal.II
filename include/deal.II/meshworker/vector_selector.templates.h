//---------------------------------------------------------------------------
//    $Id: vector_selector.templates.h 24247 2011-09-04 17:20:00Z young $
//
//    Copyright (C) 2009, 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------

#include <deal.II/meshworker/vector_selector.h>
#include <deal.II/base/vector_slice.h>
#include <deal.II/fe/fe_values.h>

DEAL_II_NAMESPACE_OPEN

namespace MeshWorker
{
  template <int dim, int spacedim>
  VectorDataBase<dim, spacedim>::~VectorDataBase()
  {}
  

  template <int dim, int spacedim>
  VectorDataBase<dim, spacedim>::VectorDataBase(const VectorSelector& v)
		  :
		  VectorSelector(v)
  {}  


  template <int dim, int spacedim>
  VectorDataBase<dim, spacedim>::VectorDataBase()
  {}  


  template <int dim, int spacedim>
  void
  VectorDataBase<dim, spacedim>::fill(
	std::vector<std::vector<std::vector<double> > >&,
	std::vector<std::vector<std::vector<Tensor<1,dim> > > >&,
	std::vector<std::vector<std::vector<Tensor<2,dim> > > >&,
	const FEValuesBase<dim,spacedim>&,
	const std::vector<unsigned int>&,
	unsigned int,
	unsigned int,
	unsigned int,
	unsigned int) const
  {
    Assert(false, ExcNotImplemented());
  }


  template <int dim, int spacedim>
  void
  VectorDataBase<dim, spacedim>::mg_fill(
	std::vector<std::vector<std::vector<double> > >&,
	std::vector<std::vector<std::vector<Tensor<1,dim> > > >&,
	std::vector<std::vector<std::vector<Tensor<2,dim> > > >&,
	const FEValuesBase<dim,spacedim>&,
	unsigned int,
	const std::vector<unsigned int>&,
	unsigned int,
	unsigned int,
	unsigned int,
	unsigned int) const
  {
    Assert(false, ExcNotImplemented());
  }
  

//----------------------------------------------------------------------//

  template <class VECTOR, int dim, int spacedim>
  VectorData<VECTOR, dim, spacedim>::VectorData()
  {}
  
  
  template <class VECTOR, int dim, int spacedim>
  VectorData<VECTOR, dim, spacedim>::VectorData(const VectorSelector& s)
		  :
		  VectorDataBase<dim, spacedim>(s)
  {}
  
  
  template <class VECTOR, int dim, int spacedim>
  void
  VectorData<VECTOR, dim, spacedim>::initialize(const NamedData<VECTOR*>& d)
  {
    data = d;
    VectorSelector::initialize(d);
  }
  
  
  template <class VECTOR, int dim, int spacedim>
  void
  VectorData<VECTOR, dim, spacedim>::initialize(const VECTOR* v, const std::string& name)
  {
    SmartPointer<const VECTOR,VectorData<VECTOR, dim, spacedim> > p = v;
    data.add(p, name);
    VectorSelector::initialize(data);
  }
  
  
  template <class VECTOR, int dim, int spacedim>
  void
  VectorData<VECTOR, dim, spacedim>::fill(
	std::vector<std::vector<std::vector<double> > >& values,
	std::vector<std::vector<std::vector<Tensor<1,dim> > > >& gradients,
	std::vector<std::vector<std::vector<Tensor<2,dim> > > >& hessians,
	const FEValuesBase<dim,spacedim>& fe,
	const std::vector<unsigned int>& index,
	unsigned int component,
	unsigned int n_comp,
	unsigned int start,
	unsigned int size) const
  {
    AssertDimension(values.size(), this->n_values());
    AssertDimension(gradients.size(), this->n_gradients());
    AssertDimension(hessians.size(), this->n_hessians());
    
    for (unsigned int i=0;i<this->n_values();++i)
      {
	const VECTOR& src = *data(this->value_index(i));
	VectorSlice<std::vector<std::vector<double> > > dst(values[i], component, n_comp);
	fe.get_function_values(src, make_slice(index, start, size), dst, true);
      }
    
    for (unsigned int i=0;i<this->n_gradients();++i)
      {
	const VECTOR& src = *data(this->gradient_index(i));
	VectorSlice<std::vector<std::vector<Tensor<1,dim> > > > dst(gradients[i], component, n_comp);
	fe.get_function_gradients(src, make_slice(index, start, size), dst, true);
      }
    
    for (unsigned int i=0;i<this->n_hessians();++i)
      {
	const VECTOR& src = *data(this->hessian_index(i));
	VectorSlice<std::vector<std::vector<Tensor<2,dim> > > > dst(hessians[i], component, n_comp);
	fe.get_function_hessians(src, make_slice(index, start, size), dst, true);
      }
  }


  template <class VECTOR, int dim, int spacedim>
  std::size_t
  VectorData<VECTOR, dim, spacedim>::memory_consumption () const
  {
    std::size_t mem = VectorSelector::memory_consumption();
    mem += sizeof (data);
    return mem;
  }

//----------------------------------------------------------------------//

  template <class VECTOR, int dim, int spacedim>
  MGVectorData<VECTOR, dim, spacedim>::MGVectorData()
  {}
  
  
  template <class VECTOR, int dim, int spacedim>
  MGVectorData<VECTOR, dim, spacedim>::MGVectorData(const VectorSelector& s)
		  :
		  VectorDataBase<dim, spacedim>(s)
  {}
  
  
  template <class VECTOR, int dim, int spacedim>
  void
  MGVectorData<VECTOR, dim, spacedim>::initialize(const NamedData<MGLevelObject<VECTOR>*>& d)
  {
    data = d;
    VectorSelector::initialize(d);
  }
  
  
  template <class VECTOR, int dim, int spacedim>
  void
  MGVectorData<VECTOR, dim, spacedim>::initialize(const MGLevelObject<VECTOR>* v, const std::string& name)
  {
    SmartPointer<const MGLevelObject<VECTOR>, MGVectorData<VECTOR, dim, spacedim> >
      p = v;
    data.add(p, name);
    VectorSelector::initialize(data);
  }
  
  
  template <class VECTOR, int dim, int spacedim>
  void
  MGVectorData<VECTOR, dim, spacedim>::mg_fill(
	std::vector<std::vector<std::vector<double> > >& values,
	std::vector<std::vector<std::vector<Tensor<1,dim> > > >& gradients,
	std::vector<std::vector<std::vector<Tensor<2,dim> > > >& hessians,
	const FEValuesBase<dim,spacedim>& fe,
	unsigned int level,
	const std::vector<unsigned int>& index,
	unsigned int component,
	unsigned int n_comp,
	unsigned int start,
	unsigned int size) const
  {
    AssertDimension(values.size(), this->n_values());
    AssertDimension(gradients.size(), this->n_gradients());
    AssertDimension(hessians.size(), this->n_hessians());
    
    for (unsigned int i=0;i<this->n_values();++i)
      {
	const VECTOR& src = (*data(this->value_index(i)))[level];
	VectorSlice<std::vector<std::vector<double> > > dst(values[i], component, n_comp);
	fe.get_function_values(src, make_slice(index, start, size), dst, true);
      }
    
    for (unsigned int i=0;i<this->n_gradients();++i)
      {
	const VECTOR& src = (*data(this->value_index(i)))[level];
	VectorSlice<std::vector<std::vector<Tensor<1,dim> > > > dst(gradients[i], component, n_comp);
	fe.get_function_gradients(src, make_slice(index, start, size), dst, true);
      }
    
    for (unsigned int i=0;i<this->n_hessians();++i)
      {
	const VECTOR& src = (*data(this->value_index(i)))[level];
	VectorSlice<std::vector<std::vector<Tensor<2,dim> > > > dst(hessians[i], component, n_comp);
	fe.get_function_hessians(src, make_slice(index, start, size), dst, true);
      }
  }


  template <class VECTOR, int dim, int spacedim>
  std::size_t
  MGVectorData<VECTOR, dim, spacedim>::memory_consumption () const
  {
    std::size_t mem = VectorSelector::memory_consumption();
    mem += sizeof (data);
    return mem;
  }
}

DEAL_II_NAMESPACE_CLOSE
