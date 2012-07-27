//---------------------------------------------------------------------------
//    $Id: boost_threads.cc 23709 2011-05-17 04:34:08Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 2009, 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------

#include <deal.II/base/config.h>

#if (DEAL_II_USE_MT == 1) && !defined(DEAL_II_CAN_USE_CXX1X)

// if the C++ compiler doesn't completely support the C++1x standard (and
// consequently we can't use std::thread, std::mutex, etc), then include all
// the files that form BOOST's thread implementation so that we don't have to
// build BOOST itself only to get at this small part of it. it also ensures
// that we use the correct compiler and flags
#  define BOOST_THREAD_BUILD_LIB 1
#  define DBOOST_ALL_NO_LIB 1

#  ifdef DEAL_II_USE_MT_POSIX
#    define BOOST_THREAD_POSIX
#    include <boost/cstdint.hpp>

#    ifndef UINTMAX_C
#      define UINTMAX_C(x) x ## ULL
#    endif

#    include "../contrib/boost-1.46.1/libs/thread/src/pthread/once.cpp"
#    include "../contrib/boost-1.46.1/libs/thread/src/pthread/thread.cpp"
#  else
#    include "../contrib/boost-1.46.1/libs/thread/src/win32/once.cpp"
#    include "../contrib/boost-1.46.1/libs/thread/src/win32/thread.cpp"
#  endif

#endif
