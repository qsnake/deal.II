//---------------------------------------------------------------------------
//    $Id: function.h 23752 2011-05-30 00:16:00Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 2009 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__std_cxx1x_function_h
#define __deal2__std_cxx1x_function_h


#include <deal.II/base/config.h>

#ifdef DEAL_II_CAN_USE_CXX1X

#  include <functional>
DEAL_II_NAMESPACE_OPEN
namespace std_cxx1x
{
  using std::function;
}
DEAL_II_NAMESPACE_CLOSE

#else

#include <boost/function.hpp>
DEAL_II_NAMESPACE_OPEN
namespace std_cxx1x
{
  using boost::function;
}
DEAL_II_NAMESPACE_CLOSE

#endif

#endif
