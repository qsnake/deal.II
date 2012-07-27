//---------------------------------------------------------------------------
//    $Id: tria_faces.cc 23709 2011-05-17 04:34:08Z bangerth $
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

#include <deal.II/base/memory_consumption.h>
#include <deal.II/grid/tria_faces.h>

DEAL_II_NAMESPACE_OPEN


namespace internal
{
  namespace Triangulation
  {

    std::size_t
    TriaFaces<1>::memory_consumption () const
    {
      return 0;
    }


    std::size_t
    TriaFaces<2>::memory_consumption () const
    {
      return MemoryConsumption::memory_consumption (lines);
    }


    std::size_t
    TriaFaces<3>::memory_consumption () const
    {
      return (MemoryConsumption::memory_consumption (quads) +
	      MemoryConsumption::memory_consumption (lines) );
    }
  }
}

DEAL_II_NAMESPACE_CLOSE

