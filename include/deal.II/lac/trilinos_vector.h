//---------------------------------------------------------------------------
//    $Id: trilinos_vector.h 23876 2011-06-28 18:21:51Z kanschat $
//
//    Copyright (C) 2008, 2009, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__trilinos_vector_h
#define __deal2__trilinos_vector_h


#include <deal.II/base/config.h>

#ifdef DEAL_II_USE_TRILINOS

#  include <deal.II/base/std_cxx1x/shared_ptr.h>
#  include <deal.II/base/subscriptor.h>
#  include <deal.II/base/index_set.h>
#  include <deal.II/base/utilities.h>
#  include <deal.II/lac/exceptions.h>
#  include <deal.II/lac/vector.h>
#  include <deal.II/lac/trilinos_vector_base.h>

#  include "Epetra_Map.h"
#  include "Epetra_LocalMap.h"

DEAL_II_NAMESPACE_OPEN


// forward declaration
template <typename> class Vector;

/**
 * @addtogroup TrilinosWrappers
 *@{
 */
namespace TrilinosWrappers
{
  class SparseMatrix;
/**
 * Namespace for Trilinos vector classes that work in parallel over
 * MPI. This namespace is restricted to vectors only, whereas matrices
 * are always MPI based when run on more than one processor.
 *
 * @ingroup TrilinosWrappers
 * @author Martin Kronbichler, Wolfgang Bangerth, 2008
 */
  namespace MPI
  {
    class BlockVector;

/**
 * This class implements a wrapper to use the Trilinos distributed
 * vector class Epetra_FEVector. This class is derived from the
 * TrilinosWrappers::VectorBase class and provides all functionality
 * included there.
 *
 * Note that Trilinos only guarantees that operations do what you expect
 * if the function @p GlobalAssemble has been called after vector
 * assembly in order to distribute the data. This is necessary since
 * some processes might have accumulated data of elements that are not
 * owned by themselves, but must be sent to the owning process. In order
 * to avoid using the wrong data, you need to call Vector::compress()
 * before you actually use the vectors.
 *
 * <h3>Parallel communication model</h3>
 *
 * The parallel functionality of Trilinos is built on top of the Message
 * Passing Interface (MPI). MPI's communication model is built on
 * collective communications: if one process wants something from
 * another, that other process has to be willing to accept this
 * communication. A process cannot query data from another process by
 * calling a remote function, without that other process expecting such
 * a transaction. The consequence is that most of the operations in the
 * base class of this class have to be called collectively. For example,
 * if you want to compute the l2 norm of a parallel vector, @em all
 * processes across which this vector is shared have to call the @p
 * l2_norm function. If you don't do this, but instead only call the @p
 * l2_norm function on one process, then the following happens: This one
 * process will call one of the collective MPI functions and wait for
 * all the other processes to join in on this. Since the other processes
 * don't call this function, you will either get a time-out on the first
 * process, or, worse, by the time the next a callto a Trilinos function
 * generates an MPI message on the other processes , you will get a
 * cryptic message that only a subset of processes attempted a
 * communication. These bugs can be very hard to figure out, unless you
 * are well-acquainted with the communication model of MPI, and know
 * which functions may generate MPI messages.
 *
 * One particular case, where an MPI message may be generated
 * unexpectedly is discussed below.
 *
 * <h3>Accessing individual elements of a vector</h3>
 *
 * Trilinos does allow read access to individual elements of a vector,
 * but in the distributed case only to elements that are stored
 * locally. We implement this through calls like
 * <tt>d=vec(i)</tt>. However, if you access an element outside the
 * locally stored range, an exception is generated.
 *
 * In contrast to read access, Trilinos (and the respective deal.II
 * wrapper classes) allow to write (or add) to individual elements of
 * vectors, even if they are stored on a different process. You can do
 * this writing, for example, <tt>vec(i)=d</tt> or <tt>vec(i)+=d</tt>,
 * or similar operations. There is one catch, however, that may lead to
 * very confusing error messages: Trilinos requires application programs
 * to call the compress() function when they switch from adding, to
 * elements to writing to elements. The reasoning is that all processes
 * might accumulate addition operations to elements, even if multiple
 * processes write to the same elements. By the time we call compress()
 * the next time, all these additions are executed. However, if one
 * process adds to an element, and another overwrites to it, the order
 * of execution would yield non-deterministic behavior if we don't make
 * sure that a synchronisation with compress() happens in between.
 *
 * In order to make sure these calls to compress() happen at the
 * appropriate time, the deal.II wrappers keep a state variable that
 * store which is the presently allowed operation: additions or
 * writes. If it encounters an operation of the opposite kind, it calls
 * compress() and flips the state. This can sometimes lead to very
 * confusing behavior, in code that may for example look like this:
 *
 * @verbatim
 * TrilinosWrappers::Vector vector;
 * // do some write operations on the vector
 * for (unsigned int i=0; i<vector->size(); ++i)
 *   vector(i) = i;
 *
 *                   // do some additions to vector elements, but
 *                   // only for some elements
 *   for (unsigned int i=0; i<vector->size(); ++i)
 *     if (some_condition(i) == true)
 *       vector(i) += 1;
 *
 *                   // do another collective operation
 *   const double norm = vector->l2_norm();
 * @endverbatim
 *
 * This code can run into trouble: by the time we see the first addition
 * operation, we need to flush the overwrite buffers for the vector, and
 * the deal.II library will do so by calling compress(). However, it
 * will only do so for all processes that actually do an addition -- if
 * the condition is never true for one of the processes, then this one
 * will not get to the actual compress() call, whereas all the other
 * ones do. This gets us into trouble, since all the other processes
 * hang in the call to flush the write buffers, while the one other
 * process advances to the call to compute the l2 norm. At this time,
 * you will get an error that some operation was attempted by only a
 * subset of processes. This behavior may seem surprising, unless you
 * know that write/addition operations on single elements may trigger
 * this behavior.
 *
 * The problem described here may be avoided by placing additional calls
 * to compress(), or making sure that all processes do the same type of
 * operations at the same time, for example by placing zero additions if
 * necessary.
 *
 * @ingroup TrilinosWrappers
 * @ingroup Vectors
 * @author Martin Kronbichler, Wolfgang Bangerth, 2008, 2009
 */
    class Vector : public VectorBase
    {
      public:
/**
 * @name Basic constructors and initalization.
 */
//@{
                                       /**
                                        * Default constructor that
                                        * generates an empty (zero size)
                                        * vector. The function
                                        * <tt>reinit()</tt> will have to
                                        * give the vector the correct
                                        * size and distribution among
                                        * processes in case of an MPI
                                        * run.
                                        */
	Vector ();

	                               /**
					* Copy constructor using the
					* given vector.
					*/
	Vector (const Vector &V);

					 /**
					  * Destructor.
					  */
	~Vector ();

				       /**
				        * Reinit functionality. This
				        * function sets the calling vector
				        * to the dimension and the parallel
				        * distribution of the input vector,
				        * but does not copy the elements in
				        * <tt>v</tt>. If <tt>fast</tt> is
				        * not <tt>true</tt>, the elements in
				        * the vector are initialized with
				        * zero, otherwise the content will
				        * be left unchanged and the user has
				        * to set all elements.
					*
				        * This function has a third  argument,
					* <tt>allow_different_maps</tt>,
					* that allows for an exchange of
					* data between two equal-sized
					* vectors (but being distributed
					* differently among the
					* processors). A trivial application
					* of this function is to generate a
					* replication of a whole vector on
					* each machine, when the calling
					* vector is built according to the
					* localized vector class
					* TrilinosWrappers::Vector, and
					* <tt>v</tt> is a distributed
					* vector. In this case, the variable
					* <tt>fast</tt> needs to be set to
					* <tt>false</tt>, since it does not
					* make sense to exchange data
					* between differently parallelized
					* vectors without touching the
					* elements.
				        */
	void reinit (const VectorBase &v,
		     const bool        fast = false,
	             const bool        allow_different_maps = false);

	void reinit (const BlockVector &v,
	             const bool         import_data = false);

                                       /**
					* Set all components of the
					* vector to the given number @p
					* s. Simply pass this down to
					* the base class, but we still
					* need to declare this function
					* to make the example given in
					* the discussion about making
					* the constructor explicit work.
					*/
        Vector & operator = (const TrilinosScalar s);

                                       /**
                                        * Copy the given vector. Resize
                                        * the present vector if
                                        * necessary. In this case, also
                                        * the Epetra_Map that designs
                                        * the parallel partitioning is
                                        * taken from the input vector.
                                        */
	Vector &
	  operator = (const Vector &V);

                                       /**
					* Copy operator from a given
					* localized vector (present on
					* all processes) in
					* TrilinosWrappers format to the
					* current distributed
					* vector. This function assumes
					* that the calling vector (left
					* hand object) already is of the
					* same size as the right hand
					* side vector. Otherwise, an
					* exception will be thrown.
					*/
	Vector &
	operator = (const ::dealii::TrilinosWrappers::Vector &V);

                                       /**
					* Another copy function. This
					* one takes a deal.II vector and
					* copies it into a
					* TrilinosWrapper vector. Note
					* that since we do not provide
					* any Epetra_map that tells
					* about the partitioning of the
					* vector among the MPI
					* processes, the size of the
					* TrilinosWrapper vector has to
					* be the same as the size of the
					* input vector. In order to
					* change the map, use the
					* reinit(const Epetra_Map
					* &input_map) function.
					*/
	template <typename Number>
	Vector &
	operator = (const ::dealii::Vector<Number> &v);

				       /**
					* This reinit function is
					* meant to be used for
					* parallel calculations where
					* some non-local data has to
					* be used. The typical
					* situation where one needs
					* this function is the call of
					* the
					* FEValues<dim>::get_function_values
					* function (or of some
					* derivatives) in
					* parallel. Since it is
					* usually faster to retrieve
					* the data in advance, this
					* function can be called
					* before the assembly forks
					* out to the different
					* processors. What this
					* function does is the
					* following: It takes the
					* information in the columns
					* of the given matrix and
					* looks which data couples
					* between the different
					* processors. That data is
					* then queried from the input
					* vector. Note that you should
					* not write to the resulting
					* vector any more, since the
					* some data can be stored
					* several times on different
					* processors, leading to
					* unpredictable results. In
					* particular, such a vector
					* cannot be used for
					* matrix-vector products as
					* for example done during the
					* solution of linear systems.
					*/
	void import_nonlocal_data_for_fe
	  (const dealii::TrilinosWrappers::SparseMatrix &matrix,
	   const Vector                                 &vector);
//@}
/**
 * @name Initialization with an Epetra_Map
 */
//@{
                                       /**
				        * This constructor takes an
				        * Epetra_Map that already knows
				        * how to distribute the
				        * individual components among
				        * the MPI processors. Since it
				        * also includes information
				        * about the size of the vector,
				        * this is all we need to
				        * generate a parallel vector.
                                        */
	Vector (const Epetra_Map &parallel_partitioning);

				       /**
					* Copy constructor from the
					* TrilinosWrappers vector
					* class. Since a vector of this
					* class does not necessarily
					* need to be distributed among
					* processes, the user needs to
					* supply us with an Epetra_Map
					* that sets the partitioning
					* details.
					*/
	explicit Vector (const Epetra_Map &parallel_partitioning,
			 const VectorBase &v);

				       /**
					* Reinitialize from a deal.II
					* vector. The Epetra_Map specifies the
					* %parallel partitioning.
					*/
	template <typename number>
	void reinit (const Epetra_Map             &parallel_partitioner,
		     const dealii::Vector<number> &v);

				       /**
				        * Reinit functionality. This
				        * function destroys the old
				        * vector content and generates a
				        * new one based on the input
				        * map.
				        */
	void reinit (const Epetra_Map &parallel_partitioning,
		     const bool        fast = false);

                                         /**
                                          * Copy-constructor from deal.II
                                          * vectors. Sets the dimension to that
                                          * of the given vector, and copies all
                                          * elements.
                                          */
        template <typename Number>
        explicit Vector (const Epetra_Map             &parallel_partitioning,
                         const dealii::Vector<Number> &v);
//@}
/**
 * @name Initialization with an IndexSet
 */
//@{
                                       /**
				        * This constructor takes an IndexSet
				        * that defines how to distribute the
				        * individual components among the
				        * MPI processors. Since it also
				        * includes information about the
				        * size of the vector, this is all we
				        * need to generate a %parallel
				        * vector.
                                        */
	Vector (const IndexSet &parallel_partitioning,
		const MPI_Comm &communicator = MPI_COMM_WORLD);

				       /**
					* Copy constructor from the
					* TrilinosWrappers vector
					* class. Since a vector of this
					* class does not necessarily need to
					* be distributed among processes,
					* the user needs to supply us with
					* an IndexSet and an MPI
					* communicator that set the
					* partitioning details.
					*/
	explicit Vector (const IndexSet   &parallel_partitioning,
			 const VectorBase &v,
			 const MPI_Comm   &communicator = MPI_COMM_WORLD);

                                         /**
                                          * Copy-constructor from deal.II
                                          * vectors. Sets the dimension to
                                          * that of the given vector, and
                                          * copies all the elements.
                                          */
        template <typename Number>
        explicit Vector (const IndexSet               &parallel_partitioning,
			 const dealii::Vector<Number> &v,
                         const MPI_Comm               &communicator = MPI_COMM_WORLD);

				       /**
				        * Reinit functionality. This
				        * function destroys the old vector
				        * content and generates a new one
				        * based on the input partitioning.
				        */
	void reinit (const IndexSet &parallel_partitioning,
		     const MPI_Comm &communicator = MPI_COMM_WORLD,
		     const bool      fast = false);
//@}
    };




// ------------------- inline and template functions --------------


/**
 * Global function @p swap which overloads the default implementation
 * of the C++ standard library which uses a temporary object. The
 * function simply exchanges the data of the two vectors.
 *
 * @relates TrilinosWrappers::MPI::Vector
 * @author Martin Kronbichler, Wolfgang Bangerth, 2008
 */
    inline
    void swap (Vector &u, Vector &v)
    {
      u.swap (v);
    }


#ifndef DOXYGEN

    template <typename number>
    Vector::Vector (const Epetra_Map             &input_map,
                    const dealii::Vector<number> &v)
    {
      reinit (input_map, v);
    }



    template <typename number>
    Vector::Vector (const IndexSet               &parallel_partitioner,
                    const dealii::Vector<number> &v,
		    const MPI_Comm               &communicator)
    {
      *this = Vector(parallel_partitioner.make_trilinos_map (communicator, true),
		     v);
    }




    template <typename number>
    void Vector::reinit (const Epetra_Map             &parallel_partitioner,
			 const dealii::Vector<number> &v)
    {
      if (vector.get() != 0 && vector->Map().SameAs(parallel_partitioner))
	vector.reset (new Epetra_FEVector(parallel_partitioner));

      const int size = parallel_partitioner.NumMyElements();

				   // Need to copy out values, since the
				   // deal.II might not use doubles, so
				   // that a direct access is not possible.
      for (int i=0; i<size; ++i)
	(*vector)[0][i] = v(parallel_partitioner.GID(i));
    }


    inline
    Vector &
    Vector::operator = (const TrilinosScalar s)
    {
      VectorBase::operator = (s);

      return *this;
    }


    template <typename Number>
    Vector &
    Vector::operator = (const ::dealii::Vector<Number> &v)
    {
      if (size() != v.size())
	{
	  vector.reset (new Epetra_FEVector(Epetra_Map
					    (v.size(), 0,
#ifdef DEAL_II_COMPILER_SUPPORTS_MPI
					     Epetra_MpiComm(MPI_COMM_SELF)
#else
					     Epetra_SerialComm()
#endif
					     )));
	}

      reinit (vector_partitioner(), v);
      return *this;
    }


#endif

  } /* end of namespace MPI */



/**
 * This class is a specialization of a Trilinos vector to a localized
 * version. The purpose of this class is to provide a copy interface
 * from the possibly parallel Vector class to a local vector on each
 * processor, in order to be able to access all elements in the vector
 * or to apply certain deal.II functions.
 *
 * @ingroup TrilinosWrappers
 * @ingroup Vectors
 * @author Martin Kronbichler, 2008
 */
  class Vector : public VectorBase
  {
    public:
                                       /**
                                        * Default constructor that
                                        * generates an empty (zero size)
                                        * vector. The function
                                        * <tt>reinit()</tt> will have to
                                        * give the vector the correct
                                        * size.
                                        */
      Vector ();

                                       /**
				        * This constructor takes as
				        * input the number of elements
				        * in the vector.
                                        */
      Vector (const unsigned int n);

                                       /**
				        * This constructor takes as
				        * input the number of elements
				        * in the vector. If the map is
				        * not localized, i.e., if there
				        * are some elements that are not
				        * present on all processes, only
				        * the global size of the map
				        * will be taken and a localized
				        * map will be generated
				        * internally.
                                        */
      Vector (const Epetra_Map &partitioning);

                                       /**
				        * This constructor takes as input
				        * the number of elements in the
				        * vector. If the index set is not
				        * localized, i.e., if there are some
				        * elements that are not present on
				        * all processes, only the global
				        * size of the index set will be
				        * taken and a localized version will
				        * be generated internally.
                                        */
      Vector (const IndexSet &partitioning,
	      const MPI_Comm &communicator = MPI_COMM_WORLD);

                                       /**
				        * This constructor takes a
				        * (possibly parallel) Trilinos
				        * Vector and generates a
				        * localized version of the whole
				        * content on each processor.
                                        */
      explicit Vector (const VectorBase &V);

                                         /**
                                          * Copy-constructor from deal.II
                                          * vectors. Sets the dimension to that
                                          * of the given vector, and copies all
                                          * elements.
                                          */
      template <typename Number>
      explicit Vector (const dealii::Vector<Number> &v);

                                       /**
					* Reinit function that resizes
					* the vector to the size
					* specified by <tt>n</tt>.
					*/
      void reinit (const unsigned int n,
		   const bool         fast = false);

                                       /**
					* Initialization with an
					* Epetra_Map. Similar to the call in
					* the other class MPI::Vector, with
					* the difference that now a copy on
					* all processes is generated. This
					* initialization function is
					* appropriate when the data in the
					* localized vector should be
					* imported from a distributed vector
					* that has been initialized with the
					* same communicator. The variable
					* <tt>fast</tt> determines whether
					* the vector should be filled with
					* zero or left untouched.
					*/
      void reinit (const Epetra_Map &input_map,
		   const bool        fast = false);

                                       /**
					* Initialization with an
					* IndexSet. Similar to the call in
					* the other class MPI::Vector, with
					* the difference that now a copy on
					* all processes is generated. This
					* initialization function is
					* appropriate in case the data in
					* the localized vector should be
					* imported from a distributed vector
					* that has been initialized with the
					* same communicator. The variable
					* <tt>fast</tt> determines whether
					* the vector should be filled with
					* zero or left untouched.
					*/
      void reinit (const IndexSet   &input_map,
		   const MPI_Comm   &communicator = MPI_COMM_WORLD,
		   const bool        fast = false);

                                       /**
					* Reinit function. Takes the
					* information of a Vector and copies
					* everything to the calling vector,
					* now also allowing different maps.
					*/
      void reinit (const VectorBase &V,
		   const bool        fast = false,
		   const bool        allow_different_maps = false);

                                       /**
					* Set all components of the
					* vector to the given number @p
					* s. Simply pass this down to
					* the base class, but we still
					* need to declare this function
					* to make the example given in
					* the discussion about making
					* the constructor explicit work.
					*/
      Vector & operator = (const TrilinosScalar s);

                                       /**
					* Sets the left hand argument to
					* the (parallel) Trilinos
					* Vector. Equivalent to the @p
					* reinit function.
					*/
      Vector &
	operator = (const MPI::Vector &V);

                                       /**
					* Sets the left hand argument to
					* the deal.II vector.
					*/
      template <typename Number>
      Vector &
	operator = (const ::dealii::Vector<Number> &V);

                                       /**
					* Copy operator. Copies both the
					* dimension and the content in
					* the right hand argument.
					*/
      Vector &
	operator = (const Vector &V);

				       /**
					* This function does nothing but is
					* there for compatibility with the
					* @p PETScWrappers::Vector class.
					*
					* For the PETSc vector wrapper class,
					* this function updates the ghost
					* values of the PETSc vector. This
					* is necessary after any modification
					* before reading ghost values.
					*
					* However, for the implementation of
					* this class, it is immaterial and thus
					* an empty function.
					*/
      void update_ghost_values () const;
  };



// ------------------- inline and template functions --------------


/**
 * Global function @p swap which overloads the default implementation
 * of the C++ standard library which uses a temporary object. The
 * function simply exchanges the data of the two vectors.
 *
 * @relates TrilinosWrappers::Vector
 * @author Martin Kronbichler, Wolfgang Bangerth, 2008
 */
  inline
  void swap (Vector &u, Vector &v)
  {
    u.swap (v);
  }


#ifndef DOXYGEN

  template <typename number>
  Vector::Vector (const dealii::Vector<number> &v)
  {
    Epetra_LocalMap map ((int)v.size(), 0, Utilities::Trilinos::comm_self());
    vector.reset (new Epetra_FEVector(map));
    *this = v;
  }



  inline
  Vector &
  Vector::operator = (const TrilinosScalar s)
  {
    VectorBase::operator = (s);

    return *this;
  }



  template <typename Number>
  Vector &
  Vector::operator = (const ::dealii::Vector<Number> &v)
  {
    if (size() != v.size())
      {
	vector.reset();

	Epetra_LocalMap map ((int)v.size(), 0,
			     Utilities::Trilinos::comm_self());
	vector.reset (new Epetra_FEVector(map));
      }

    const Epetra_Map & map = vector_partitioner();
    const int size = map.NumMyElements();

    Assert (map.MaxLID() == size-1,
	    ExcDimensionMismatch(map.MaxLID(), size-1));

				   // Need to copy out values, since the
				   // deal.II might not use doubles, so
				   // that a direct access is not possible.
    for (int i=0; i<size; ++i)
      (*vector)[0][i] = v(i);

    return *this;
  }



  inline
  void
  Vector::update_ghost_values () const
  {}


#endif


}


/*@}*/

DEAL_II_NAMESPACE_CLOSE

#endif // DEAL_II_USE_TRILINOS

/*----------------------------   trilinos_vector.h     ---------------------------*/

#endif
/*----------------------------   trilinos_vector.h     ---------------------------*/
