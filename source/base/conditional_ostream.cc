//---------------------------------------------------------------------------
//    $Id: conditional_ostream.cc 23709 2011-05-17 04:34:08Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 2004, 2005, 2006, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------

#include <deal.II/base/conditional_ostream.h>

DEAL_II_NAMESPACE_OPEN

ConditionalOStream::ConditionalOStream(std::ostream &stream,
                                       const bool    active)
                :
		output_stream (stream),
		active_flag(active)
{}


void ConditionalOStream::set_condition(bool flag)
{
  active_flag = flag;
}


bool ConditionalOStream::is_active() const
{
  return active_flag;
}

DEAL_II_NAMESPACE_CLOSE
