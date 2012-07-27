//---------------------------------------------------------------------------
//    $Id: exceptions.cc 23709 2011-05-17 04:34:08Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 1998, 2000, 2001, 2002, 2003, 2005, 2006, 2009 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <deal.II/base/exceptions.h>
#include <deal.II/base/logstream.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <sstream>

#ifdef HAVE_GLIBC_STACKTRACE
#  include <execinfo.h>
#endif

#ifdef HAVE_LIBSTDCXX_DEMANGLER
#  include <cxxabi.h>
#endif

DEAL_II_NAMESPACE_OPEN


namespace deal_II_exceptions
{

  std::string additional_assert_output;

  void set_additional_assert_output (const char * const p)
  {
    additional_assert_output = p;
  }



  bool show_stacktrace = true;

  void suppress_stacktrace_in_exceptions ()
  {
    show_stacktrace = false;
  }


  bool abort_on_exception = true;

  void disable_abort_on_exception ()
  {
    abort_on_exception = false;
  }
}



ExceptionBase::ExceptionBase ()
                :
		file(""), line(0), function(""), cond(""), exc(""),
		stacktrace (0),
		n_stacktrace_frames (0)
{}



ExceptionBase::ExceptionBase (const char* f, const int l, const char *func,
			      const char* c, const char *e)
                :
		file(f), line(l), function(func), cond(c), exc(e),
		stacktrace (0),
		n_stacktrace_frames (0)
{}



ExceptionBase::ExceptionBase (const ExceptionBase &exc)
                :
                std::exception (exc),
		file(exc.file), line(exc.line),
                function(exc.function), cond(exc.cond), exc(exc.exc),
                                                 // don't copy stacktrace to
                                                 // avoid double de-allocation
                                                 // problem
		stacktrace (0),
		n_stacktrace_frames (0)
{}



ExceptionBase::~ExceptionBase () throw ()
{
  if (stacktrace != 0)
    {
      free (stacktrace);
      stacktrace = 0;
    }
}



void ExceptionBase::set_fields (const char* f,
				const int l,
				const char *func,
				const char *c,
				const char *e)
{
  file = f;
  line = l;
  function = func;
  cond = c;
  exc  = e;

				   // if the system supports this, get
				   // a stacktrace how we got here
#ifdef HAVE_GLIBC_STACKTRACE
   void * array[25];
   n_stacktrace_frames = backtrace(array, 25);
   stacktrace = backtrace_symbols(array, n_stacktrace_frames);
#endif  
}



void ExceptionBase::print_stack_trace (std::ostream &out) const
{
  if (n_stacktrace_frames == 0)
    return;

  if (deal_II_exceptions::show_stacktrace == false)
    return;
  
  
				   // if there is a stackframe stored, print it
  out << std::endl;
  out << "Stacktrace:" << std::endl
      << "-----------" << std::endl;
  
				    // print the stacktrace. first
				    // omit all those frames that have
				    // ExceptionBase or
				    // deal_II_exceptions in their
				    // names, as these correspond to
				    // the exception raising mechanism
				    // themselves, rather than the
				    // place where the exception was
				    // triggered
   int frame = 0;
   while ((frame < n_stacktrace_frames)
	  &&
	  ((std::string(stacktrace[frame]).find ("ExceptionBase") != std::string::npos)
	   ||
	   (std::string(stacktrace[frame]).find ("deal_II_exceptions") != std::string::npos)))
     ++frame;

				    // output the rest
   const unsigned int first_significant_frame = frame;
   for (; frame < n_stacktrace_frames; ++frame)
     {
       out << '#' << frame - first_significant_frame
	   << "  ";

					// the stacktrace frame is
					// actually of the format
					// "filename(functionname+offset)
					// [address]". let's try to
					// get the mangled
					// functionname out:
       std::string stacktrace_entry (stacktrace[frame]);
       const unsigned int pos_start = stacktrace_entry.find('('),
			  pos_end   = stacktrace_entry.find('+');
       std::string functionname = stacktrace_entry.substr (pos_start+1,
							   pos_end-pos_start-1);

					// demangle, and if successful
					// replace old mangled string
					// by unmangled one (skipping
					// address and offset). treat
					// "main" differently, since
					// it is apparently demangled
					// as "unsigned int" for
					// unknown reasons :-)
					// if we can, demangle the
					// function name
#ifdef HAVE_LIBSTDCXX_DEMANGLER
       int         status;
       char *p = abi::__cxa_demangle(functionname.c_str(), 0, 0, &status);
       
       if ((status == 0) && (functionname != "main"))
	 {
	   std::string realname(p);
					    // in MT mode, one often
					    // gets backtraces
					    // spanning several lines
					    // because we have so many
					    // boost::tuple arguments
					    // in the MT calling
					    // functions. most of the
					    // trailing arguments of
					    // these tuples are
					    // actually unused
					    // boost::tuples::null_type,
					    // so we should split them
					    // off if they are
					    // trailing a template
					    // argument list
	   while (realname.find (", boost::tuples::null_type>")
		  != std::string::npos)
	     realname.erase (realname.find (", boost::tuples::null_type>"),
			     std::string (", boost::tuples::null_type").size());
	   
	   stacktrace_entry = stacktrace_entry.substr(0, pos_start)
			      +
			      ": "
			      +
			      realname;
	 }
       else
	 stacktrace_entry = stacktrace_entry.substr(0, pos_start)
			    +
			    ": "
			    +
			    functionname;

       free (p);
       
#else	 

       stacktrace_entry = stacktrace_entry.substr(0, pos_start)
			  +
			  ": "
			  +
			  functionname;
#endif
       
					// then output what we have
       out << stacktrace_entry
	   << std::endl;

					// stop if we're in main()
       if (functionname == "main")
	 break;
     }
}


void ExceptionBase::print_exc_data (std::ostream &out) const
{
  out << "An error occurred in line <" << line
      << "> of file <" << file
      << "> in function" << std::endl
      << "    " << function << std::endl
      << "The violated condition was: "<< std::endl
      << "    " << cond << std::endl
      << "The name and call sequence of the exception was:" << std::endl
      << "    " << exc  << std::endl
      << "Additional Information: " << std::endl;
				   // Additionally, leave a trace in
				   // deallog if we do not stop here
  if (deal_II_exceptions::abort_on_exception == false)
    deallog << exc << std::endl;
}


void ExceptionBase::print_info (std::ostream &out) const
{
  out << "(none)" << std::endl;
}


const char * ExceptionBase::what () const throw ()
{
                                   // if we say that this function
                                   // does not throw exceptions, we
                                   // better make sure it does not
  try 
    {
                                       // have a place where to store the
                                       // description of the exception as
                                       // a char *
                                       //
                                       // this thing obviously is not
                                       // multi-threading safe, but we
                                       // don't care about that for now
                                       //
                                       // we need to make this object
                                       // static, since we want to return
                                       // the data stored in it and
                                       // therefore need a lifetime which
                                       // is longer than the execution
                                       // time of this function
      static std::string description;
                                       // convert the messages printed by
                                       // the exceptions into a
                                       // std::string
      std::ostringstream converter;

      converter << "--------------------------------------------------------"
                << std::endl;
                                       // put general info into the std::string
      print_exc_data (converter);
                                       // put in exception specific data
      print_info (converter);

      print_stack_trace (converter);

      converter << "--------------------------------------------------------"
                << std::endl;

      description = converter.str();

      return description.c_str();
    }
  catch (std::exception &exc) 
    {
      std::cerr << "*** Exception encountered in exception handling routines ***"
                << std::endl
                << "*** Message is "   << std::endl
                << exc.what ()         << std::endl
                << "*** Aborting! ***" << std::endl;

      std::abort ();
    }
  catch (...)
    {
      std::cerr << "*** Exception encountered in exception handling routines ***"
                << std::endl
                << "*** Aborting! ***" << std::endl;

      std::abort ();
    }
  return 0;
}



namespace deal_II_exceptions
{
  namespace internals 
  {
    
				     /**
				      * Number of exceptions dealt
				      * with so far. Zero at program
				      * start. Messages are only
				      * displayed if the value is
				      * zero.
				      */
    unsigned int n_treated_exceptions;
     ExceptionBase *last_exception;
  

    void issue_error_assert (const char *file,
			     int         line,
			     const char *function,
			     const char *cond,
			     const char *exc_name,
			     ExceptionBase &e)
    {
                                       // fill the fields of the
				       // exception object
       e.set_fields (file, line, function, cond, exc_name);
      
				       // if no other exception has
				       // been displayed before, show
				       // this one
      if (n_treated_exceptions == 0)
	{
          std::cerr << "--------------------------------------------------------"
		    << std::endl;
					   // print out general data
	  e.print_exc_data (std::cerr);
					   // print out exception
					   // specific data
	  e.print_info (std::cerr);
	  e.print_stack_trace (std::cerr);
	  std::cerr << "--------------------------------------------------------"
		    << std::endl;

					   // if there is more to say,
					   // do so
	  if (!additional_assert_output.empty())
	    std::cerr << additional_assert_output << std::endl;
	}
      else
	{
					   // if this is the first
					   // follow-up message,
					   // display a message that
					   // further exceptions are
					   // suppressed
	  if (n_treated_exceptions == 1)
	    std::cerr << "******** More assertions fail but messages are suppressed! ********"
		      << std::endl;
	};
      
				       // increase number of treated
				       // exceptions by one
      n_treated_exceptions++;
      last_exception = &e;
    
    
				       // abort the program now since
				       // something has gone horribly
				       // wrong. however, there is one
				       // case where we do not want to
				       // do that, namely when another
				       // exception, possibly thrown
				       // by AssertThrow is active,
				       // since in that case we will
				       // not come to see the original
				       // exception. in that case
				       // indicate that the program is
				       // not aborted due to this
				       // reason.
      if (std::uncaught_exception() == true)
	{
					   // only display message once
	  if (n_treated_exceptions <= 1)
	    std::cerr << "******** Program is not aborted since another exception is active! ********"
		      << std::endl;
	}
      else if(deal_II_exceptions::abort_on_exception == true)
         std::abort ();
      else
	--n_treated_exceptions;
    }



    void abort ()
    {
      if (deal_II_exceptions::abort_on_exception == true)
         std::abort ();
    }
    
  }
  
}


DEAL_II_NAMESPACE_CLOSE


// from the aclocal file:
// Newer versions of gcc have a very nice feature: you can set
// a verbose terminate handler, that not only aborts a program
// when an exception is thrown and not caught somewhere, but
// before aborting it prints that an exception has been thrown,
// and possibly what the std::exception::what() function has to
// say. Since many people run into the trap of not having a
// catch clause in main(), they wonder where that abort may be
// coming from. The terminate handler then at least says what is
// missing in their program.
#ifdef HAVE_VERBOSE_TERMINATE
namespace __gnu_cxx
{
  extern void __verbose_terminate_handler ();
}

namespace 
{
  struct preload_terminate_dummy
  {
     preload_terminate_dummy()
       { std::set_terminate(__gnu_cxx::__verbose_terminate_handler); }
  };

  static preload_terminate_dummy dummy;
}

#endif
