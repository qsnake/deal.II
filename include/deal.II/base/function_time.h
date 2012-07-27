//---------------------------------------------------------------------------
//    $Id: function_time.h 23876 2011-06-28 18:21:51Z kanschat $
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__function_time_h
#define __deal2__function_time_h


#include <deal.II/base/config.h>
#include <deal.II/base/exceptions.h>

DEAL_II_NAMESPACE_OPEN
/**
 *  Support for time dependent functions.
 *  The library was also designed for time dependent problems. For this
 *  purpose, the function objects also contain a field which stores the
 *  time, as well as functions manipulating them. Time independent problems
 *  should not access or even abuse them for other purposes, but since one
 *  normally does not create thousands of function objects, the gain in
 *  generality weighs out the fact that we need not store the time value
 *  for not time dependent problems. The second advantage is that the derived
 *  standard classes like <tt>ZeroFunction</tt>, <tt>ConstantFunction</tt> etc also work
 *  for time dependent problems.
 *
 *  Access to the time goes through the following functions:
 *  @verbatim
 *  <li> <tt>get_time</tt>: return the present value of the time variable.
 *  <li> <tt>set_time</tt>: set the time value to a specific value.
 *  <li> <tt>advance_time</tt>: increase the time by a certain time step.
 *  @endverbatim
 *  The latter two functions are virtual, so that derived classes can
 *  perform computations which need only be done once for every new time.
 *  For example, if a time dependent function had a factor <tt>sin(t)</tt>, then
 *  it may be a reasonable choice to calculate this factor in a derived
 *  version of <tt>set_time</tt>, store it in a member variable and use that one
 *  rather than computing it every time <tt>operator()</tt>, <tt>value_list</tt> or one
 *  of the other functions is called.
 *
 *  By default, the <tt>advance_time</tt> function calls the <tt>set_time</tt> function
 *  with the new time, so it is sufficient in most cases to overload only
 *  <tt>set_time</tt> for computations as sketched out above.
 *
 *  The constructor of this class takes an initial value for the time
 *  variable, which defaults to zero. Because a default value is given,
 *  none of the derived classes needs to take an initial value for the
 *  time variable if not needed.
 *
 *  Once again the warning: do not use the <tt>time</tt> variable for any other
 *  purpose than the intended one! This will inevitably lead to confusion.
 *
 *
 *  @ingroup functions
 *  @author Wolfgang Bangerth, Guido Kanschat, 1998, 1999
 */
class FunctionTime
{
  public:
				     /**
				      * Constructor. May take an initial vakue
				      * for the time variable, which defaults
				      * to zero.
				      */
    FunctionTime (const double initial_time = 0.0);

				     /**
				      * Virtual destructor.
				      */
    virtual ~FunctionTime();
  
				     /**
				      * Return the value of the time variable/
				      */
    double get_time () const;

				     /**
				      * Set the time to <tt>new_time</tt>, overwriting
				      * the old value.
				      */
    virtual void set_time (const double new_time);

				     /**
				      * Advance the time by the given
				      * time step <tt>delta_t</tt>.
				      */
    virtual void advance_time (const double delta_t);

  private:
				     /**
				      * Store the present time.
				      */
    double time;
};



/*------------------------------ Inline functions ------------------------------*/

#ifndef DOXYGEN

inline double
FunctionTime::get_time () const
{
  return time;
}

#endif
DEAL_II_NAMESPACE_CLOSE

#endif
