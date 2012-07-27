//---------------------------------------------------------------------------
//    $Id: thread.h 23752 2011-05-30 00:16:00Z bangerth $
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
#ifndef __deal2__std_cxx1x_thread_h
#define __deal2__std_cxx1x_thread_h


#include <deal.II/base/config.h>

#ifdef DEAL_II_CAN_USE_CXX1X

#  include <thread>
DEAL_II_NAMESPACE_OPEN
namespace std_cxx1x
{
  using std::thread;
}
DEAL_II_NAMESPACE_CLOSE

#else

#  include <boost/thread.hpp>
DEAL_II_NAMESPACE_OPEN
namespace std_cxx1x
{
  using boost::thread;
}
DEAL_II_NAMESPACE_CLOSE

#endif

#endif
