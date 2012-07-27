//---------------------------------------------------------------------------
//    $Id: function_time.cc 23709 2011-05-17 04:34:08Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <deal.II/base/function_time.h>

DEAL_II_NAMESPACE_OPEN


FunctionTime::FunctionTime(const double initial_time)
		:
		time(initial_time)
{}



FunctionTime::~FunctionTime()
{}



void
FunctionTime::set_time (const double new_time)
{
  time = new_time;
}



void
FunctionTime::advance_time (const double delta_t)
{
  set_time (time+delta_t);
}

DEAL_II_NAMESPACE_CLOSE
