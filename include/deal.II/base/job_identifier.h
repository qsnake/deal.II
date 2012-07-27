//---------------------------------------------------------------------------
//    $Id: job_identifier.h 23876 2011-06-28 18:21:51Z kanschat $
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__job_identifier_h
#define __deal2__job_identifier_h


#include <deal.II/base/config.h>
#include <string>

DEAL_II_NAMESPACE_OPEN
/**
 * Identification of a program run. <tt>JobIdentifier</tt> determines the
 * start time of a program run and stores it as a program
 * identifier. There exists a library object <tt>dealjobid</tt> of this
 * class. This object can be accessed by all output functions to
 * provide an id for the current job.
 * 
 * @ingroup utilities
 */
class JobIdentifier
{
  public:
				     /**
				      * Constructor. Set program
				      * identifier to value of
				      * <tt>program_id</tt> concatenated
				      * with the present time.
				      */
    JobIdentifier();

				     /**
				      * This function returns an
				      * identifier for the running
				      * program. Currently, the
				      * library provides a function
				      * returning "JobID".
				      *
				      * The user may define a
				      * replacement of this function
				      * in his source code and avoid
				      * linking the library
				      * version. Unfortunately, this
				      * mechanism does not work with
				      * shared libraries.
				      */
    static const char* program_id();

				     /**
				      * Obtain the base name of the
				      * file currently being
				      * compiled. That is, if the file
				      * is <tt>mypath/file.cc</tt>
				      * return just
				      * <tt>file</tt>. Typically, this
				      * can be called from a program
				      * with the argument
				      * <tt>__FILE__</tt> and is used
				      * in the deal.II test suite.
				      */
    static std::string base_name(const char* filename);
    
				     /**
				      * Return the value of <tt>id</tt>.
				      */
    const std::string operator () () const;
    
  private:
				     /**
				      * String holding the identifier
				      * of the presently running
				      * program.
				      */
    std::string id;
};


/*------------------------------ Inline functions ------------------------------*/


/**
 * Global object to identify the presently running program.
 */
extern JobIdentifier dealjobid;
DEAL_II_NAMESPACE_CLOSE

#endif
