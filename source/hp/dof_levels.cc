//----------------------------  hp_dof_levels.cc  ---------------------------
//    $Id: dof_levels.cc 23709 2011-05-17 04:34:08Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 2003, 2006, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  hp_dof_levels.cc  ------------------------


#include <deal.II/base/memory_consumption.h>
#include <deal.II/hp/dof_levels.h>

DEAL_II_NAMESPACE_OPEN

namespace internal
{
  namespace hp
  {

    std::size_t
    DoFLevel<1>::memory_consumption () const
    {
      return (DoFLevel<0>::memory_consumption() +
              MemoryConsumption::memory_consumption (lines));
    }
    


    std::size_t
    DoFLevel<2>::memory_consumption () const
    {
      return (DoFLevel<0>::memory_consumption () +
              MemoryConsumption::memory_consumption (quads));
    }



    std::size_t
    DoFLevel<3>::memory_consumption () const
    {
      return (DoFLevel<0>::memory_consumption () +
              MemoryConsumption::memory_consumption (hexes));
    }



    std::size_t
    DoFLevel<0>::memory_consumption () const
    {
      return MemoryConsumption::memory_consumption (active_fe_indices);
    }
    
  }
}

DEAL_II_NAMESPACE_CLOSE
