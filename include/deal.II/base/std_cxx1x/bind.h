//---------------------------------------------------------------------------
//    $Id: bind.h 23752 2011-05-30 00:16:00Z bangerth $
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
#ifndef __deal2__std_cxx1x_bind_h
#define __deal2__std_cxx1x_bind_h


#include <deal.II/base/config.h>

#ifdef DEAL_II_CAN_USE_CXX1X

#  include <functional>

DEAL_II_NAMESPACE_OPEN
// in boost, the placeholders _1, _2, ... are in the global namespace. in
// C++0x, they are in namespace std::placeholders, which makes them awkward to
// use. import them into the deal.II::std_cxx1x namespace instead and do them
// same below if we use boost instead.
namespace std_cxx1x
{
  using namespace std::placeholders;
  using std::bind;
  using std::ref;
  using std::cref;
  using std::reference_wrapper;
}
DEAL_II_NAMESPACE_CLOSE

#else

#include <boost/bind.hpp>

DEAL_II_NAMESPACE_OPEN
namespace std_cxx1x
{
  using boost::bind;
  using boost::ref;
  using boost::cref;
  using boost::reference_wrapper;
  
  // now also import the _1, _2 placeholders from the global namespace
  // into the current one as suggested above
  using ::_1;
  using ::_2;
  using ::_3;
  using ::_4;
  using ::_5;
  using ::_6;
  using ::_7;
  using ::_8;
  using ::_9;
}
DEAL_II_NAMESPACE_CLOSE

#endif

#endif
