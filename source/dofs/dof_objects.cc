//---------------------------------------------------------------------------
//    $Id: dof_objects.cc 23709 2011-05-17 04:34:08Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 2006, 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <deal.II/base/exceptions.h>
#include <deal.II/base/memory_consumption.h>
#include <deal.II/dofs/dof_objects.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/fe/fe.h>

DEAL_II_NAMESPACE_OPEN

namespace internal
{
  namespace DoFHandler
  {
    template <int dim>
    std::size_t
    DoFObjects<dim>::memory_consumption () const
    {
      return (MemoryConsumption::memory_consumption (dofs));
    }



    template <int dim>
    template <int dh_dim, int spacedim>
    void
    DoFObjects<dim>::
    set_dof_index (const dealii::DoFHandler<dh_dim, spacedim> &dof_handler,
		   const unsigned int       obj_index,
		   const unsigned int       fe_index,
		   const unsigned int       local_index,
		   const unsigned int       global_index)
    {
      Assert ((fe_index == dealii::DoFHandler<dh_dim, spacedim>::default_fe_index),
	      ExcMessage ("Only the default FE index is allowed for non-hp DoFHandler objects"));
      Assert (local_index<dof_handler.get_fe().template n_dofs_per_object<dim>(),
	      ExcIndexRange (local_index, 0, dof_handler.get_fe().template n_dofs_per_object<dim>()));
      Assert (obj_index * dof_handler.get_fe().template n_dofs_per_object<dim>()+local_index
	      <
	      dofs.size(),
	      ExcInternalError());

      dofs[obj_index * dof_handler.get_fe()
           .template n_dofs_per_object<dim>() + local_index] = global_index;
    }
  }
}


// explicit instantiations
namespace internal
{
  namespace DoFHandler
  {
#include "dof_objects.inst"
  }
}

DEAL_II_NAMESPACE_CLOSE
