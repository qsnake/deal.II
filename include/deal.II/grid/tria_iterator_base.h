//---------------------------------------------------------------------------
//    $Id: tria_iterator_base.h 23876 2011-06-28 18:21:51Z kanschat $
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__tria_iterator_base_h
#define __deal2__tria_iterator_base_h


#include <deal.II/base/config.h>

DEAL_II_NAMESPACE_OPEN

/**
 * Namespace in which an enumeration is declared that denotes the
 * states in which an iterator can be in.
 *
 * @ingroup Iterators
 */
namespace IteratorState
{
  
/**
 *   The three states an iterator can be in: valid, past-the-end and
 *   invalid.
 */
  enum IteratorStates
  {
					 /// Iterator points to a valid object
	valid,
					 /// Iterator reached end of container
	past_the_end,
					 /// Iterator is invalid, probably due to an error
	invalid
  };
}



DEAL_II_NAMESPACE_CLOSE

#endif
