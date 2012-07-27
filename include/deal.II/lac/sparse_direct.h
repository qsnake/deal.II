//---------------------------------------------------------------------------
//    $Id: sparse_direct.h 24299 2011-09-10 04:56:42Z bangerth $
//
//    Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__sparse_direct_h
#define __deal2__sparse_direct_h



#include <deal.II/base/config.h>
#include <deal.II/base/exceptions.h>
#include <deal.II/base/subscriptor.h>
#include <deal.II/base/thread_management.h>
#include <deal.II/lac/vector.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/sparse_matrix_ez.h>
#include <deal.II/lac/block_sparse_matrix.h>

#ifdef DEAL_II_USE_MUMPS
#  include <deal.II/base/utilities.h>
#  include <dmumps_c.h>
#endif

DEAL_II_NAMESPACE_OPEN

/**
 * This class provides an interface to the sparse direct solver MA27
 * from the Harwell Subroutine Library. MA27 is a direct solver
 * specialized for sparse symmetric indefinite systems of linear
 * equations and uses a modified form of Gauss elimination. It is
 * included in the <a
 * href="http://www.cse.clrc.ac.uk/Activity/HSL">Harwell Subroutine
 * Library</a> and is written in Fortran. The present class only
 * transforms the data stored in SparseMatrix objects into the
 * form which is required by the functions resembling MA27, calls
 * these Fortran functions, and interprets some of the returned values
 * indicating error codes, etc. It also manages allocation of the
 * right amount of temporary storage required by these functions.
 *
 * Note that this class only works if configuration of the deal.II library has
 * detected the presence of this solver. Please read the README file on what
 * the configure script is looking for and how to provide it.
 *
 *
 * <h3>Interface and Method</h3>
 *
 * For the meaning of the three functions initialize(), factorize(),
 * and solve(), as well as for the method used in MA27, please see the
 * <a href="http://www.cse.clrc.ac.uk/Activity/HSL">documentation</a>
 * of these functions. In practice, you will most often call the
 * second solve() function, which solves the linear system for a
 * given right hand side, but one can as well call the three functions
 * separately if, for example, one would like to solve the same matrix
 * for several right hand side vectors; the MA27 solver can do this
 * efficiently, as it computes a decomposition of the matrix, so that
 * subsequent solves only amount to a forward-backward substitution
 * which is significantly less costly than the decomposition process.
 *
 *
 * <h3>Parameters to the constructor</h3>
 *
 * The constructor of this class takes several arguments. The meaning
 * is the following: the MA27 functions require the user to allocate
 * and pass a certain amount of memory for temporary variables or for
 * data to be passed to subsequent functions. The sizes of these
 * arrays are denoted by the variables <tt>LIW1</tt>, <tt>LIW2</tt>, and <tt>LA</tt>,
 * where <tt>LIW1</tt> denotes the size of the <tt>IW</tt> array in the call to
 * <tt>MA27A</tt>, while <tt>LIW2</tt> is the array size in the call to
 * <tt>MA27B</tt>. The documentation of the MA27 functions gives ways to
 * obtain estimates for their values, e.g. by evaluating values
 * returned by functions called before. However, the documentation
 * only states that the values have to be <b>at least</b> as large as
 * the estimates, a hint that is not very useful oftentimes (in my
 * humble opinion, the lack of dynamic memory allocation mechanism is
 * a good reason not to program in Fortran 77 :-).
 *
 * In our experience, it is often necessary to go beyond the proposed
 * values (most often for <tt>LA</tt>, but also for <tt>LIW1</tt>). The first
 * three parameters of the constructor denote by which factor the
 * initial estimates shall be increased. The default values are 1.2
 * (the documentation recommends this value, 1, and 1.5, values which
 * have often worked for us. Note that the value of <tt>LIW</tt> is only
 * changed in the second call if the recommended value times
 * <tt>LIW_factor_2</tt> is larger than the array size already is from the
 * call to <tt>MA27A</tt>; otherwise, <tt>LIW_factor_2</tt> is ignored.
 *
 * If the values thus constructed fail to work, we try to restart the
 * called function with larger values until the calls succeed. The
 * second triple of values passed to the constructor denotes by which
 * factor we shall increase the array sizes. If the increment factors
 * are less than or equal to one, then we only try to call the
 * respective calls to the functions once and abort by throwing an
 * error. Note that the <tt>MA27C</tt> function writes out an error message
 * if the value of <tt>LA</tt> is too small and gives an indication to
 * which size it should be increased. However, most often the
 * indicated value is far too small and can not be relied upon.
 *
 *
 * <h3>Note on parallelization</h3>
 *
 * <h4>Synchronisation</h4>
 *
 * Due to the use of global variables through COMMON blocks, the calls
 * to the sparse direct solver routines are not multithreading-safe,
 * i.e. at each time there may only be one call to these functions
 * active. You have to synchronise your calls to the functions
 * provided by this class using mutexes (see the Threads
 * namespace for such classes) to avoid multiple active calls at the
 * same time if you use multithreading. Since you may use this class
 * in different parts of your program, and may not want to use a
 * global variable for locking, this class has a lock as static member
 * variable, which may be accessed using the
 * get_synchronisation_lock() function. Note however, that this class
 * does not perform the synchronisation for you within its member
 * functions. The reason is that you will usually want to synchronise
 * over the calls to initialize() and factorize(), since there should
 * probably not be a call to one of these function with another matrix
 * between the calls for one matrix. (The author does not really know
 * whether this is true, but it is probably safe to assume that.)
 * Since such cross-function synchronisation can only be performed
 * from outside, it is left to the user of this class to do so.
 *
 * <h4>Detached mode</h4>
 *
 * As an alternative, you can call the function set_detached_mode()
 * right after calling the constructor. This lets the program fork, so
 * that we now have two programs that communicate via pipes.  The
 * forked copy of the program then actually replaces itself by a
 * program called <tt>detached_ma27</tt>, that is started in its place
 * through the <tt>execv</tt> system call. Now everytime you call one of
 * the functions of this class, it relays the data to the other
 * program and lets it execute the respective function. The results
 * are then transfered back. Since the MA27 functions are only called
 * in the detached program, they will now no longer interfere with the
 * respective calls to other functions with different data, so no
 * synchronisation is necessary any more.
 *
 * The advantage of this approach is that as many instances of this
 * class may be active at any time as you want. This is handy, if your
 * programs spens a significant amount of time in them, and you are
 * using many threads, for example in a machine with 4 or more
 * processors. The disadvantage, of course, is that the data has to
 * copied to and from the detached program, which might make things
 * slower (though, as we use block writes, this should not be so much
 * of a factor).
 *
 * Since no more synchronisation is necessary, the
 * get_synchronisation_lock() returns a reference to a member
 * variable when the detached mode is set. Thus, you need not change
 * your program: you can still acquire and release the lock as before,
 * it will only have no effect now, since different objects of this
 * class no longer share the lock, i.e. you will get it always without
 * waiting. On the other hand, it will prevent that you call functions
 * of this object multiply in parallel at the same time, which is what
 * you probably wanted.
 *
 *
 * <h5>Internals of the detached mode</h5>
 *
 * The program that actually runs the detached solver is called
 * <tt>detached_ma27</tt>, and will show up under this name in the process
 * list. It communicates with the main program through a pipe.
 *
 * Since the solver and the main program are two separated processes,
 * the solver program will not be notified if the main program dies,
 * for example because it is aborted with Control-C, because an
 * exception is raised and not caught, or some other reason. It will
 * just not get any new jobs, but will happily wait until the end of
 * times. For this reason, the detached solver has a second thread
 * running in parallel that simply checks in regular intervals whether
 * the main program is still alive, using the <tt>ps</tt> program. If this
 * is no longer the case, the detached solver exits as well.
 *
 * Since the intervals between two such checks are a couple of second,
 * it may happen that the detached solver survives the main program by
 * some time. Presently, the check interval is once every 20
 * seconds. After that time, the detached solver should have noticed
 * the main programs demise.
 *
 * @ingroup Solvers Preconditioners
 *
 * @author Wolfgang Bangerth, 2000, 2001, 2002
 */
class SparseDirectMA27 : public Subscriptor
{
  public:
				     /**
				      * Constructor. See the
				      * documentation of this class
				      * for the meaning of the
				      * parameters to this function.
				      */
    SparseDirectMA27 (const double LIW_factor_1          = 1.2,
		      const double LIW_factor_2          = 1,
		      const double LA_factor             = 1.5,
		      const double LIW_increase_factor_1 = 1.2,
		      const double LIW_increase_factor_2 = 1.2,
		      const double LA_increase_factor    = 1.2,
		      const bool   suppress_output       = true);

                                     /**
                                      * Destructor.
                                      */
    ~SparseDirectMA27 ();

                                     /**
                                      * Set the detached mode (see the
                                      * general class documentation
                                      * for a description of what this
                                      * is).
                                      *
                                      * This function must not be
                                      * called after initialize()
                                      * (or the two-argument solve()
                                      * function has been called. If
                                      * it is to be called, then only
                                      * right after construction of
                                      * the object, and before first
                                      * use.
                                      */
    void set_detached_mode ();

                                     /**
                                      * Return whether the detached
                                      * mode is set.
                                      */
    bool detached_mode_set () const;

				     /**
				      * Initialize some data
				      * structures. This function
				      * computes symbolically some
				      * information based on the
				      * sparsity pattern, but does not
				      * actually use the values of the
				      * matrix, so only the sparsity
				      * pattern has to be passed as
				      * argument.
				      */
    void initialize (const SparsityPattern &sparsity_pattern);

				     /**
				      * Actually factorize the
				      * matrix. This function may be
				      * called multiple times for
				      * different matrices, after the
				      * object of this class has been
				      * initialized for a certain
				      * sparsity pattern. You may
				      * therefore save some computing
				      * time if you want to invert
				      * several matrices with the same
				      * sparsity pattern. However,
				      * note that the bulk of the
				      * computing time is actually
				      * spent in the factorization, so
				      * this functionality may not
				      * always be of large benefit.
				      *
				      * If the initialization step has
				      * not been performed yet, then
				      * the initialize() function is
				      * called at the beginning of
				      * this function.
				      */
    template <typename number>
    void factorize (const SparseMatrix<number> &matrix);

				     /**
				      * Solve for a certain right hand
				      * side vector. This function may
				      * be called multiple times for
				      * different right hand side
				      * vectors after the matrix has
				      * been factorized. This yields a
				      * big saving in computing time,
				      * since the actual solution is
				      * fast, compared to the
				      * factorization of the matrix.
				      *
				      * The solution will be returned
				      * in place of the right hand
				      * side vector.
				      *
				      * If the factorization has not
				      * happened before, strange
				      * things will happen. Note that
				      * we can't actually call the
				      * factorize() function from
				      * here if it has not yet been
				      * called, since we have no
				      * access to the actual matrix.
				      */
    template <typename number>
    void solve (Vector<number> &rhs_and_solution) const;

				     /**
				      * Call the three functions
				      * initialize, factorize and solve
				      * in that order, i.e. perform
				      * the whole solution process for
				      * the given right hand side
				      * vector.
				      *
				      * The solution will be returned
				      * in place of the right hand
				      * side vector.
				      */
    template <typename number>
    void solve (const SparseMatrix<number> &matrix,
		Vector<double>             &rhs_and_solution);

				     /**
				      * Return an estimate of the
				      * memory used by this class.
				      */
    std::size_t memory_consumption () const;

				     /**
				      * Get a reference to the
				      * synchronisation lock which can
				      * be used for this class. See
				      * the general description of
				      * this class for more
				      * information.
				      */
    Threads::ThreadMutex & get_synchronisation_lock () const;

    				     /** @addtogroup Exceptions
				      * @{ */

				     /**
				      * Exception.
				      */
    DeclException1 (ExcMA27AFailed,
		    int,
		    << "The function MA27A failed with an exit code of " << arg1);
				     /**
				      * Exception.
				      */
    DeclException1 (ExcMA27BFailed,
		    int,
		    << "The function MA27B failed with an exit code of " << arg1);
				     /**
				      * Exception.
				      */
    DeclException1 (ExcMA27CFailed,
		    int,
		    << "The function MA27C failed with an exit code of " << arg1);
				     /**
				      * Exception
				      */
    DeclException0 (ExcInitializeAlreadyCalled);

				     /**
				      * Exception
				      */
    DeclException0 (ExcFactorizeNotCalled);

				     /**
				      * Exception
				      */
    DeclException0 (ExcDifferentSparsityPatterns);

				     /**
				      * Exception
				      */
    DeclException2 (ExcReadError,
                    int, int,
                    << "Error while reading in detached mode. Return value "
                    << "for 'read' was " << arg1
                    << ", errno has value " << arg2);
                                     /**
                                      * Exception
                                      */
    DeclException0 (ExcMatrixNotSymmetric);
				     //@}
  private:
                                     /**
                                      * Declare a local type which
                                      * will store the data necessary
                                      * to communicate with a detached
                                      * solver. To avoid adding
                                      * various system include files,
                                      * the actual declaration of this
                                      * class is in the implementation
                                      * file.
                                      */
    struct DetachedModeData;

                                     /**
                                      * Store in the constructor
                                      * whether the MA27 routines
                                      * shall deliver output to stdout
                                      * or not.
                                      */
    const bool suppress_output;

                                     /**
                                      * Store whether
                                      * set_detached_mode() has been
                                      * called.
                                      */
    bool detached_mode;

                                     /**
                                      * Pointer to a structure that
                                      * will hold the data necessary
                                      * to uphold communication with a
                                      * detached solver.
                                      */
    DetachedModeData *detached_mode_data;

				     /**
				      * Store the three values passed
				      * to the cinstructor. See the
				      * documentation of this class
				      * for the meaning of these
				      * variables.
				      */
    const double LIW_factor_1;
    const double LIW_factor_2;
    const double LA_factor;

				     /**
				      * Increase factors in case a
				      * call to a function fails.
				      */
    const double LIW_increase_factor_1;
    const double LIW_increase_factor_2;
    const double LA_increase_factor;

				     /**
				      * Flags storing whether the
				      * first two functions have
				      * already been called.
				      */
    bool initialize_called;
    bool factorize_called;

				     /**
				      * Store a pointer to the
				      * sparsity pattern, to make sure
				      * that we use the same thing for
				      * all calls.
				      */
    SmartPointer<const SparsityPattern,SparseDirectMA27> sparsity_pattern;

				     /**
				      * Number of nonzero elements in
				      * the sparsity pattern on and
				      * above the diagonal.
				      */
    unsigned int n_nonzero_elements;

				     /**
				      * Arrays holding row and column
				      * indices.
				      */
    std::vector<unsigned int> row_numbers;
    std::vector<unsigned int> column_numbers;

				     /**
				      * Array to hold the matrix
				      * elements, and later the
				      * elements of the factors.
				      */
    std::vector<double>       A;

				     /**
				      * Length of the <tt>A</tt> array.
				      */
    unsigned int         LA;

				     /**
				      * Scratch arrays and variables
				      * used by the MA27 functions. We
				      * keep to the names introduced
				      * in the documentation of these
				      * functions, in all uppercase
				      * letters as is usual in
				      * Fortran.
				      */
    unsigned int LIW;
    std::vector<unsigned int> IW;
    std::vector<unsigned int> IKEEP;
    std::vector<unsigned int> IW1;

    unsigned int NSTEPS;
    unsigned int MAXFRT;

				     /**
				      * Two values that live inside a
				      * COMMON block of the Fortran
				      * code and are mirrored at these
				      * locations. They are used to
				      * transport information about
				      * the required length of arrays
				      * from the Fortran functions to
				      * the outside world.
				      */
    unsigned int NRLNEC;
    unsigned int NIRNEC;

				     /**
				      * Flag indicating the level of
				      * output desired and returning
				      * error values if error occured.
				      */
    int IFLAG;

				     /**
				      * Mutexes for synchronising access
				      * to this class.
				      */
    static Threads::ThreadMutex static_synchronisation_lock;
    mutable Threads::ThreadMutex non_static_synchronisation_lock;

				     /**
				      * Fill the <tt>A</tt> array from the
				      * symmetric part of the given
				      * matrix.
				      */
    template <typename number>
    void fill_A (const SparseMatrix<number> &matrix);

				     /**
				      * Call the respective function
				      * with the given args, either
				      * locally or remote.
				      */
    void call_ma27ad (const unsigned int *N,
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
				      * Call the respective function
				      * with the given args, either
				      * locally or remote.
				      */
    void call_ma27bd (const unsigned int *N,
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
				      * Call the respective function
				      * with the given args, either
				      * locally or remote.
				      */
    void call_ma27cd (const unsigned int *N,
                      const double       *A,
                      const unsigned int *LA,
                      const unsigned int *IW,
                      const unsigned int *LIW,
                      const unsigned int *MAXFRT,
                      double             *RHS,
                      const unsigned int *IW1,
                      const unsigned int *NSTEPS) const;

				     /**
				      * Call the respective function
				      * with the given args, either
				      * locally or remote.
				      */
    void call_ma27x1 (unsigned int *NRLNEC);

				     /**
				      * Call the respective function
				      * with the given args, either
				      * locally or remote.
				      */
    void call_ma27x2 (unsigned int *NIRNEC);

				     /**
				      * Call the respective function
				      * with the given args, either
				      * locally or remote.
				      */
    void call_ma27x3 (const unsigned int *LP);
};






/**
 * This class provides an interface to the sparse direct solver MA47
 * from the Harwell Subroutine Library. MA47 is a direct solver
 * specialized for sparse symmetric indefinite systems of linear
 * equations and uses a frontal elimination method. It is included in
 * the <a href="http://www.cse.clrc.ac.uk/Activity/HSL">Harwell
 * Subroutine Library</a> and is written in Fortran. The present class
 * only transforms the data stored in SparseMatrix objects into
 * the form which is required by the functions resembling MA47, calls
 * these Fortran functions, and interprets some of the returned values
 * indicating error codes, etc. It also manages allocation of the
 * right amount of temporary storage required by these functions.
 *
 * Note that this class only works if configuration of the deal.II library has
 * detected the presence of this solver. Please read the README file on what
 * the configure script is looking for and how to provide it.
 *
 *
 * <h3>Interface and Method</h3>
 *
 * For the meaning of the three functions initialize(), factorize(),
 * and solve(), as well as for the method used in MA47, please see the
 * <a href="http://www.cse.clrc.ac.uk/Activity/HSL">documentation</a>
 * of these functions. In practice, one will most often call the
 * second solve() function, which solves the linear system for a given
 * right hand side, but one can as well call the three functions
 * separately if, for example, one would like to solve the same matrix
 * for several right hand side vectors; the MA47 solver can do this
 * efficiently, as it computes a decomposition of the matrix, so that
 * subsequent solves only amount to a forward-backward substitution
 * which is significantly less costly than the decomposition process.
 *
 *
 * <h3>Parameters to the constructor</h3>
 *
 * The constructor of this class takes several arguments. Their
 * meaning is equivalent to those of the constructor of the
 * SparseDirectMA27 class; see there for more information.
 *
 *
 * <h3>Note on parallelization</h3>
 *
 * Due to the use of global variables through COMMON blocks, the calls
 * to the sparse direct solver routines is not multithreading-capable,
 * i.e. at each time there may only be one call to these functions
 * active. You have to synchronise your calls to the functions
 * provided by this class using mutexes (see the Threads
 * namespace for such classes) to avoid multiple active calls at the
 * same time if you use multithreading. Since you may use this class
 * in different parts of your program, and may not want to use a
 * global variable for locking, this class has a lock as static member
 * variable, which may be accessed using the
 * get_synchronisation_lock() function. Note however, that this class
 * does not perform the synchronisation for you within its member
 * functions. The reason is that you will usually want to synchronise
 * over the calls to initialize() and factorize(), since there should
 * probably not be a call to one of these function with another matrix
 * between the calls for one matrix. (The author does not really know
 * whether this is true, but it is probably safe to assume that.)
 * Since such cross-function synchronisation can only be performed
 * from outside, it is left to the user of this class to do so.
 *
 * A detached mode as for MA27 has not yet been implemented for this
 * class.
 *
 *
 * @ingroup Solvers Preconditioners
 *
 * @author Wolfgang Bangerth, 2000, 2001
 */
class SparseDirectMA47 : public Subscriptor
{
  public:
				     /**
				      * Constructor. See the
				      * documentation of this class
				      * for the meaning of the
				      * parameters to this function.
				      *
				      * This function already calls
				      * the initialization function
				      * <tt>MA47ID</tt> to set up some
				      * values.
				      */
    SparseDirectMA47 (const double LIW_factor_1 = 1.4,
		      const double LIW_factor_2 = 1,
		      const double LA_factor    = 3,
		      const double LIW_increase_factor_1 = 1.2,
		      const double LIW_increase_factor_2 = 1.2,
		      const double LA_increase_factor    = 1.2,
		      const bool   suppress_output       = true);

				     /**
				      * Initialize some data
				      * structures. This function
				      * computes symbolically some
				      * information based on the
				      * sparsity pattern, but does not
				      * actually use the values of the
				      * matrix, so only the sparsity
				      * pattern has to be passed as
				      * argument.
				      *
				      * Since the MA47 solver requires
				      * us to omit zero-entries in the
				      * matrix (even if they are in
				      * the sparsity pattern), we have
				      * to actually use the matrix
				      * here, as opposed to the MA27
				      * solver that only required the
				      * sparsity pattern.
				      */
    void initialize (const SparseMatrix<double> &matrix);

				     /**
				      * Actually factorize the
				      * matrix. Unlike for the MA27
				      * solver, this function may not
				      * be called multiple times for
				      * different matrices, since we
				      * have eliminated entries from
				      * the sparsity pattern where
				      * matrix entries happen to be
				      * zero. Since this is likely to
				      * change between matrices
				      * although they have the same
				      * sparsity pattern.
				      *
				      * If the initialization step has
				      * not been performed yet, then
				      * the initialize() function is
				      * called at the beginning of
				      * this function.
				      */
    void factorize (const SparseMatrix<double> &matrix);

				     /**
				      * Solve for a certain right hand
				      * side vector. This function may
				      * be called multiple times for
				      * different right hand side
				      * vectors after the matrix has
				      * been factorized. This yields a
				      * big saving in computing time,
				      * since the actual solution is
				      * fast, compared to the
				      * factorization of the matrix.
				      *
				      * The solution will be returned
				      * in place of the right hand
				      * side vector.
				      *
				      * If the factorization has not
				      * happened before, strange
				      * things will happen. Note that
				      * we can't actually call the
				      * factorize() function from
				      * here if it has not yet been
				      * called, since we have no
				      * access to the actual matrix.
				      */
    void solve (Vector<double> &rhs_and_solution);

				     /**
				      * Call the three functions
				      * initialize, factorize and
				      * solve
				      * in that order, i.e. perform
				      * the whole solution process for
				      * the given right hand side
				      * vector.
				      *
				      * The solution will be returned
				      * in place of the right hand
				      * side vector.
				      */
    void solve (const SparseMatrix<double> &matrix,
		Vector<double>             &rhs_and_solution);

				     /**
				      * Return an estimate of the
				      * memory used by this class.
				      */
    std::size_t memory_consumption () const;

				     /**
				      * Get a reference to the
				      * synchronisation lock which can
				      * be used for this class. See
				      * the general description of
				      * this class for more
				      * information.
				      */
    Threads::ThreadMutex & get_synchronisation_lock () const;

    				     /** @addtogroup Exceptions
				      * @{ */

				     /**
				      * Exception.
				      */
    DeclException1 (ExcMA47AFailed,
		    int,
		    << "The function MA47A failed with an exit code of " << arg1);
				     /**
				      * Exception.
				      */
    DeclException1 (ExcMA47BFailed,
		    int,
		    << "The function MA47B failed with an exit code of " << arg1);
				     /**
				      * Exception.
				      */
    DeclException1 (ExcMA47CFailed,
		    int,
		    << "The function MA47C failed with an exit code of " << arg1);
				     /**
				      * Exception
				      */
    DeclException0 (ExcInitializeAlreadyCalled);

				     /**
				      * Exception
				      */
    DeclException0 (ExcFactorizeNotCalled);

				     /**
				      * Exception
				      */
    DeclException0 (ExcCantFactorizeAgain);

				     /**
				      * Exception
				      */
    DeclException0 (ExcDifferentMatrices);
                                     /**
                                      * Exception
                                      */
    DeclException0 (ExcMatrixNotSymmetric);
				     //@}
  private:
                                     /**
                                      * Store in the constructor
                                      * whether the MA47 routines
                                      * shall deliver output to stdout
                                      * or not.
                                      */
    const bool suppress_output;

    				     /**
				      * Store the three values passed
				      * to the cinstructor. See the
				      * documentation of this class
				      * for the meaning of these
				      * variables.
				      */
    const double LIW_factor_1;
    const double LIW_factor_2;
    const double LA_factor;

				     /**
				      * Increase factors in case a
				      * call to a function fails.
				      */
    const double LIW_increase_factor_1;
    const double LIW_increase_factor_2;
    const double LA_increase_factor;

				     /**
				      * Flags storing whether the
				      * first two functions have
				      * already been called.
				      */
    bool initialize_called;
    bool factorize_called;

				     /**
				      * Store a pointer to the matrix,
				      * to make sure that we use the
				      * same thing for all calls.
				      */
    SmartPointer<const SparseMatrix<double>,SparseDirectMA47> matrix;

				     /**
				      * Number of nonzero elements in
				      * the sparsity pattern on and
				      * above the diagonal.
				      */
    unsigned int n_nonzero_elements;

				     /**
				      * Control values set by <tt>MA47ID</tt>.
				      */
    double       CNTL[2];
    unsigned int ICNTL[7];

				     /**
				      * Info field filled by the MA47
				      * functions and (partially) used
				      * for subsequent MA47 calls.
				      */
    int          INFO[24];

				     /**
				      * Arrays holding row and column
				      * indices.
				      */
    std::vector<unsigned int> row_numbers;
    std::vector<unsigned int> column_numbers;

				     /**
				      * Array to hold the matrix
				      * elements, and later the
				      * elements of the factors.
				      */
    std::vector<double>       A;

				     /**
				      * Length of the <tt>A</tt> array.
				      */
    unsigned int         LA;

				     /**
				      * Scratch arrays and variables
				      * used by the MA47 functions. We
				      * keep to the names introduced
				      * in the documentation of these
				      * functions, in all uppercase
				      * letters as is usual in
				      * Fortran.
				      */
    unsigned int LIW;
    std::vector<unsigned int> IW;
    std::vector<unsigned int> KEEP;
    std::vector<unsigned int> IW1;

				     /**
				      * Mutex for synchronising access
				      * to this class.
				      */
    static Threads::ThreadMutex synchronisation_lock;

				     /**
				      * Fill the <tt>A</tt> array from the
				      * symmetric part of the given
				      * matrix.
				      */
    void fill_A (const SparseMatrix<double> &matrix);

                                     /**
                                      * Call the <tt>ma47id</tt> function
                                      * with the given args.
                                      */
    void call_ma47id (double       *CNTL,
                      unsigned int *ICNTL);

                                     /**
                                      * Call the <tt>ma47ad</tt> function
                                      * with the given args.
                                      */
    void call_ma47ad (const unsigned int *n_rows,
                      const unsigned int *n_nonzero_elements,
                      unsigned int       *row_numbers,
                      unsigned int       *column_numbers,
                      unsigned int       *IW,
                      const unsigned int *LIW,
                      unsigned int       *KEEP,
                      const unsigned int *ICNTL,
                      int                *INFO);

                                     /**
                                      * Call the <tt>ma47bd</tt> function
                                      * with the given args.
                                      */
    void call_ma47bd (const unsigned int *n_rows,
                      const unsigned int *n_nonzero_elements,
                      const unsigned int *column_numbers,
                      double             *A,
                      const unsigned int *LA,
                      unsigned int       *IW,
                      const unsigned int *LIW,
                      const unsigned int *KEEP,
                      const double       *CNTL,
                      const unsigned int *ICNTL,
                      unsigned int       *IW1,
                      int                *INFO);

                                     /**
                                      * Call the <tt>ma47bd</tt> function
                                      * with the given args.
                                      */
    void call_ma47cd (const unsigned int *n_rows,
                      const double       *A,
                      const unsigned int *LA,
                      const unsigned int *IW,
                      const unsigned int *LIW,
                      double             *rhs_and_solution,
                      unsigned int       *IW1,
                      const unsigned int *ICNTL);
};




/**
 * This class provides an interface to the sparse direct solver
 * UMFPACK (see <a
 * href="http://www.cise.ufl.edu/research/sparse/umfpack">this
 * link</a>). UMFPACK is a set of routines for solving non-symmetric
 * sparse linear systems, Ax=b, using the Unsymmetric-pattern
 * MultiFrontal method and direct sparse LU factorization. Matrices
 * may have symmetric or unsymmetrix sparsity patterns, and may have
 * unsymmetric entries. The use of this class is explained in the @ref
 * step_22 "step-22" and @ref
 * step_29 "step-29" tutorial programs.
 *
 * This matrix class implements the usual interface of
 * preconditioners, that is a function initialize(const
 * SparseMatrix<double>&matrix,const AdditionalData) for initalizing
 * and the whole set of vmult() functions common to all
 * matrices. Implemented here are only vmult() and vmult_add(), which
 * perform multiplication with the inverse matrix. Furthermore, this
 * class provides an older interface, consisting of the functions
 * factorize() and solve(). Both interfaces are interchangeable.
 *
 * @note This class only exists if support for <a
 * href="http://www.cise.ufl.edu/research/sparse/umfpack">UMFPACK</a> was
 * enabled during configure and if the <a
 * href="http://www.cise.ufl.edu/research/sparse/umfpack">UMFPACK</a> library
 * was configured. The steps to do this are explained in the deal.II ReadMe
 * file. If you do nothing at the time you configure deal.II, then this class
 * will simply not work.
 *
 * @note UMFPACK has its own license, independent of that of deal.II. If you
 * want to use the UMFPACK you have to accept that license. It is linked to
 * from the deal.II ReadMe file. UMFPACK is included courtesy of its author,
 * <a href="http://www.cise.ufl.edu/~davis/">Timothy A. Davis</a>.
 *
 *
 * <h4>Instantiations</h4>
 *
 * There are instantiations of this class for SparseMatrix<double>,
 * SparseMatrix<float>, BlockSparseMatrix<double>, and
 * BlockSparseMatrix<float>.
 *
 * @ingroup Solvers Preconditioners
 *
 * @author Wolfgang Bangerth, 2004
 */
class SparseDirectUMFPACK : public Subscriptor
{
  public:
				     /**
				      * Dummy class needed for the
				      * usual initalization interface
				      * of preconditioners.
				      */
    class AdditionalData
    {};


				     /**
				      * Constructor. See the
				      * documentation of this class
				      * for the meaning of the
				      * parameters to this function.
				      */
    SparseDirectUMFPACK ();

                                     /**
                                      * Destructor.
                                      */
    ~SparseDirectUMFPACK ();

				     /**
				      * This function does nothing. It is only
				      * here to provide an interface that is
				      * consistent with that of the HSL MA27
				      * and MA47 solver classes.
				      */
    void initialize (const SparsityPattern &sparsity_pattern);

				     /**
				      * Factorize the matrix. This function
				      * may be called multiple times for
				      * different matrices, after the object
				      * of this class has been initialized for
				      * a certain sparsity pattern. You may
				      * therefore save some computing time if
				      * you want to invert several matrices
				      * with the same sparsity
				      * pattern. However, note that the bulk
				      * of the computing time is actually
				      * spent in the factorization, so this
				      * functionality may not always be of
				      * large benefit.
				      *
				      * In contrast to the other direct solver
				      * classes, the initialisation method
				      * does nothing. Therefore initialise
				      * is not automatically called by this
				      * method, when
				      * the initialization step has
				      * not been performed yet.
				      *
				      * This function copies the contents of
				      * the matrix into its own storage; the
				      * matrix can therefore be deleted after
				      * this operation, even if subsequent
				      * solves are required.
				      */
    template <class Matrix>
    void factorize (const Matrix &matrix);

				     /**
				      * Initialize memory and call
				      * SparseDirectUMFPACK::factorize.
				      */
    template <class Matrix>
    void initialize(const Matrix &matrix,
		    const AdditionalData additional_data = AdditionalData());

				     /**
				      * Preconditioner interface
				      * function. Usually, given the source
				      * vector, this method returns an
				      * approximated solution of <i>Ax
				      * = b</i>. As this class provides a
				      * wrapper to a direct solver, here
				      * it is actually the exact solution
				      * (exact within the range of numerical
				      * accuracy of course).
				      */
    void vmult (Vector<double>&, const Vector<double>&) const;

				     /**
				      * Not implemented but necessary
				      * for compiling.
				      */
    void Tvmult (Vector<double>&, const Vector<double>&) const;

				     /**
				      * Same as vmult(), but adding to
				      * the previous solution. Not
				      * implemented yet.
				      */
    void vmult_add (Vector<double>&, const Vector<double>&) const;

				     /**
				      * Not implemented but necessary
				      * for compiling.
				      */
    void Tvmult_add (Vector<double>&, const Vector<double>&) const;

				     /**
				      * Solve for a certain right hand
				      * side vector. This function may
				      * be called multiple times for
				      * different right hand side
				      * vectors after the matrix has
				      * been factorized. This yields a
				      * big saving in computing time,
				      * since the actual solution is
				      * fast, compared to the
				      * factorization of the matrix.
				      *
				      * The solution will be returned
				      * in place of the right hand
				      * side vector.
				      *
				      * If the factorization has not
				      * happened before, strange
				      * things will happen. Note that
				      * we can't actually call the
				      * factorize() function from
				      * here if it has not yet been
				      * called, since we have no
				      * access to the actual matrix.
				      */
    void solve (Vector<double> &rhs_and_solution) const;

				     /**
				      * Call the two functions
				      * factorize and solve
				      * in that order, i.e. perform
				      * the whole solution process for
				      * the given right hand side
				      * vector.
				      *
				      * The solution will be returned
				      * in place of the right hand
				      * side vector.
				      */
    template <class Matrix>
    void solve (const Matrix   &matrix,
		Vector<double> &rhs_and_solution);

                                     /**
                                      * One of the UMFPack routines
                                      * threw an error. The error code
                                      * is included in the output and
                                      * can be looked up in the
                                      * UMFPack user manual. The name
                                      * of the routine is included for
                                      * reference.
                                      */
    DeclException2 (ExcUMFPACKError, char*, int,
		    << "UMFPACK routine " << arg1
		    << " returned error status " << arg2
		    << ". See the file <contrib/umfpack/UMFPACK/Include/umfpack.h>"
		    << " for a description of error codes.");

  private:
                                     /**
                                      * The UMFPACK routines allocate objects
                                      * in which they store information about
                                      * symbolic and numeric values of the
                                      * decomposition. The actual data type of
                                      * these objects is opaque, and only
                                      * passed around as void pointers.
                                      */
    void *symbolic_decomposition;
    void *numeric_decomposition;

                                     /**
                                      * Free all memory that hasn't been freed
                                      * yet.
                                      */
    void clear ();


                                     /**
                                      * Make sure that the arrays Ai
                                      * and Ap are sorted in each
                                      * row. UMFPACK wants it this
                                      * way. We need to have three
                                      * versions of this function, one
                                      * for the usual SparseMatrix, one
                                      * for the SparseMatrixEZ, and
                                      * one for the BlockSparseMatrix
                                      * classes
                                      */
    template <typename number>
    void sort_arrays (const SparseMatrixEZ<number> &);

    template <typename number>
    void sort_arrays (const SparseMatrix<number> &);

    template <typename number>
    void sort_arrays (const BlockSparseMatrix<number> &);

                                     /**
                                      * The arrays in which we store the data
                                      * for the solver.
                                      */
    std::vector<long int> Ap;
    std::vector<long int> Ai;
    std::vector<double> Ax;

                                     /**
                                      * Control and info arrays for the solver
                                      * routines.
                                      */
    std::vector<double> control;
};


/**
 * This class provides an interface to the parallel sparse direct
 * solver <a href="http://mumps.enseeiht.fr">MUMPS</a>. MUMPS is
 * direct method based on a multifrontal approach, which performs a
 * direct LU factorization. The matrix coming in may have either
 * symmetric or nonsymmetric sparsity pattern.
 *
 * @note This class is useable if and only if a working installation
 * of <a href="http://mumps.enseeiht.fr">MUMPS</a> exists on your
 * system and was detected during configuration of
 * <code>deal.II</code>.
 *
 * <h4>Instantiations</h4>
 *
 * There are instantiations of this class for SparseMatrix<double>,
 * SparseMatrix<float>, BlockSparseMatrix<double>, and
 * BlockSparseMatrix<float>.
 *
 * @author Markus Buerg, 2010
 */
class SparseDirectMUMPS
{
 private:

#ifdef DEAL_II_USE_MUMPS
  DMUMPS_STRUC_C id;
#endif // DEAL_II_USE_MUMPS

  double       *a;
  double       *rhs;
  unsigned int *irn;
  unsigned int *jcn;
  unsigned int  n;
  unsigned int  nz;

                        /**
			 * Flags storing whether the function
			 * <tt>initialize ()</tt> has already been
			 * called.
			 */
  bool initialize_called;

 public:

                        /**
			 * Constructor
			 */
  SparseDirectMUMPS ();

                        /**
			 * Destructor
			 */
  ~SparseDirectMUMPS ();

                        /**
                         * Exception
                         */
  DeclException0 (ExcInitializeAlreadyCalled);

                        /**
                         * This function initializes a MUMPS instance
			 * and hands over the system's matrix
			 * <tt>matrix</tt> and right-hand side
			 * <tt>vector</tt> to the solver.
			 */
  template <class Matrix>
    void initialize (const Matrix& matrix,
		     const Vector<double>      & vector);

                        /**
                         * A function in which the linear system is
			 * solved and the solution vector is copied
			 * into the given <tt>vector</tt>.
			 */
  void solve (Vector<double>& vector);
};

DEAL_II_NAMESPACE_CLOSE

#endif // __deal2__sparse_direct_h
