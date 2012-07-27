//----------------------------  hsl.h  ---------------------------
//    $Id: hsl.h 6899 2003-01-09 16:48:43Z wolf $
//    Version: $Name$
//
//    Copyright (C) 2000, 2001, 2002, 2003 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  hsl.h  ---------------------------
#ifndef __deal2__hsl_h
#define __deal2__hsl_h





/**
 * In this namespace we declare the Fortran functions that are used
 * from the Harwell Subroutine Library. The functions which we use
 * here provide services for the direct solution of sparse systems of
 * linear equations. For a thorough discussion of these functions, as
 * well as for a way to download them, please see
 * @url{http://www.cse.clrc.ac.uk/Activity/HSL}. For a description of
 * the steps necessary for the installation of HSL subroutines, read
 * the section on external libraries in the deal.II ReadMe file.
 *
 * @author Wolfgang Bangerth, 2000, 2001
 */
namespace HSL 
{
				   /**
				    * This namespace serves for the
				    * declaration of those functions
				    * inside the Harwell Subroutine
				    * Library that are used in
				    * connection with the MA27 direct
				    * sparse solver.
				    *
				    * MA27 is a direct solver for
				    * sparse symmetric linear systems
				    * of equations. The linear system
				    * needs not necessarily be
				    * definite.
				    *
				    * @author Wolfgang Bangerth, 2000
				    */
  namespace MA27
  {
				     /**
				      * Extern declaration of the MA27
				      * initialization function. See
				      * the MA27 documentation for
				      * more information about its
				      * purpose and the meaning of its
				      * arguments.
				      */
    extern "C"
    void ma27ad_ (const unsigned int *N,
		  const unsigned int *NZ,
		  const unsigned int *IRN,
		  const unsigned int *ICN,
		  unsigned int       *IW,
		  const unsigned int *LIW,
		  unsigned int       *IKEEP,
		  unsigned int       *IW1,
		  unsigned int       *NSTEPS,
		  int                *IFLAG);

				     /**
				      * Extern declaration of the MA27
				      * factorization function. See
				      * the MA27 documentation for
				      * more information about its
				      * purpose and the meaning of its
				      * arguments.
				      */
    extern "C"
    void ma27bd_ (const unsigned int *N,
		  const unsigned int *NZ,
		  const unsigned int *IRN,
		  const unsigned int *ICN,
		  double             *A,
		  const unsigned int *LA,
		  unsigned int       *IW,
		  const unsigned int *LIW,
		  const unsigned int *IKEEP,
		  const unsigned int *NSTEPS,
		  unsigned int       *MAXFRT,
		  unsigned int       *IW1,
		  int                *IFLAG);
    
				     /**
				      * Extern declaration of the MA27
				      * solver function. See
				      * the MA27 documentation for
				      * more information about its
				      * purpose and the meaning of its
				      * arguments.
				      */
    extern "C"
    void ma27cd_ (const unsigned int *N,
		  const double       *A,
		  const unsigned int *LA,
		  const unsigned int *IW,
		  const unsigned int *LIW,
		  double             *W,
		  const unsigned int *MAXFRT,
		  double             *RHS,
		  const unsigned int *IW1,
		  const unsigned int *NSTEPS);

				     /**
				      * Declaration of a Fortran
				      * helper function that returns
				      * the value of @p{NRLNEC} from
				      * the @p{MA27ED} common block of
				      * the MA27 functions.
				      *
				      * This function does not belong
				      * to the Harwell Subroutine
				      * Library, but was written for
				      * the interface to that library.
				      */
    extern "C"
    void ma27x1_ (unsigned int *NRLNEC);

				     /**
				      * Declaration of a Fortran
				      * helper function that returns
				      * the value of @p{NIRNEC} from
				      * the @p{MA27ED} common block of
				      * the MA27 functions.
				      *
				      * This function does not belong
				      * to the Harwell Subroutine
				      * Library, but was written for
				      * the interface to that library.
				      */
    extern "C"
    void ma27x2_ (unsigned int *NIRNEC);

				     /**
				      * Set the value of the error
				      * stream @p{LP}. Setting it to
				      * zero suppresses error output
				      * from the @p{MA27*} functions.
				      */
    extern "C"
    void ma27x3_ (const unsigned int *LP);
  }


  
  				   /**
				    * This namespace serves for the
				    * declaration of those functions
				    * inside the Harwell Subroutine
				    * Library that are used in
				    * connection with the MA47 direct
				    * sparse solver.
				    *
				    * MA47 is a direct solver for
				    * sparse symmetric linear systems
				    * of equations. The linear system
				    * needs not necessarily be
				    * definite.
				    *
				    * @author Wolfgang Bangerth, 2000
				    */
  namespace MA47
  {
				     /**
				      * Extern declaration of the
				      * function from the Harwell
				      * Subroutine Library that
				      * returns some default values
				      * for parameters of the MA47
				      * direct solver. See the MA47
				      * documentation for more
				      * information about its purpose
				      * and the meaning of its
				      * arguments.
				      */
    extern "C"
    void ma47id_ (double       *CNTL,
		  unsigned int *ICNTL);

				     /**
				      * Extern declaration of the
				      * function from the Harwell
				      * Subroutine Library that does
				      * some symbolic manipulations
				      * for the MA47 direct
				      * solver. See the MA47
				      * documentation for more
				      * information about its purpose
				      * and the meaning of its
				      * arguments.
				      */
    extern "C"
    void ma47ad_ (const unsigned int *N,
		  const unsigned int *NE,
		  unsigned int       *IRN,
		  unsigned int       *JCN,
		  unsigned int       *IW,
		  const unsigned int *LIW,
		  unsigned int       *KEEP,
		  const unsigned int *ICNTL,
		  double             *RINFO,
		  int                *INFO);
    
				     /**
				      * Extern declaration of the
				      * function from the Harwell
				      * Subroutine Library that does
				      * the factorization for the MA47
				      * direct solver. See the MA47
				      * documentation for more
				      * information about its purpose
				      * and the meaning of its
				      * arguments.
				      */
    extern "C"
    void ma47bd_ (const unsigned int *N,
		  const unsigned int *NE,
		  const unsigned int *JCN,
		  double             *A,
		  const unsigned int *LA,
		  unsigned int       *IW,
		  const unsigned int *LIW,
		  const unsigned int *KEEP,
		  const double       *CNTL,
		  const unsigned int *ICNTL,
		  unsigned int       *IW1,
		  double             *RINFO,
		  int                *INFO);
    
    
				     /**
				      * Extern declaration of the
				      * function from the Harwell
				      * Subroutine Library that does
				      * the solution for a certain
				      * right hand side for the MA47
				      * direct solver. See the MA47
				      * documentation for more
				      * information about its purpose
				      * and the meaning of its
				      * arguments.
				      */
    extern "C"
    void ma47cd_ (const unsigned int *N,
		  const double       *A,
		  const unsigned int *LA,
		  const unsigned int *IW,
		  const unsigned int *LIW,
		  double             *W,
		  double             *RHS,
		  unsigned int       *IW1,
		  const unsigned int *ICNTL);
  }
}


//----------------------------   hsl.h     ---------------------------
// end of #ifndef __deal2__hsl_h
#endif
//----------------------------   hsl.h     ---------------------------
