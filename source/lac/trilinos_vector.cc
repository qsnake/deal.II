//---------------------------------------------------------------------------
//    $Id: trilinos_vector.cc 23709 2011-05-17 04:34:08Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 2008, 2009 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <deal.II/lac/trilinos_vector.h>

#ifdef DEAL_II_USE_TRILINOS

#  include <deal.II/lac/trilinos_sparse_matrix.h>
#  include <deal.II/lac/trilinos_block_vector.h>
#  include <Epetra_Import.h>
#  include <Epetra_Vector.h>

#  include <cmath>


DEAL_II_NAMESPACE_OPEN

namespace TrilinosWrappers
{
  namespace MPI
  {


    Vector::Vector ()
    {
      last_action = Zero;
      vector.reset(new Epetra_FEVector(Epetra_Map(0,0,0,Utilities::Trilinos::comm_self())));
    }



    Vector::Vector (const Epetra_Map &parallel_partitioning)
    {
      reinit (parallel_partitioning);
    }



    Vector::Vector (const IndexSet &parallel_partitioning,
		    const MPI_Comm &communicator)
    {
      reinit (parallel_partitioning, communicator);
    }



    Vector::Vector (const Vector &v)
                    :
                    VectorBase()
    {
      last_action = Zero;
      vector.reset (new Epetra_FEVector(*v.vector));
    }



    Vector::Vector (const Epetra_Map &input_map,
		    const VectorBase &v)
                    :
                    VectorBase()
    {
      AssertThrow (input_map.NumGlobalElements() == v.vector->Map().NumGlobalElements(),
		   ExcDimensionMismatch (input_map.NumGlobalElements(),
					 v.vector->Map().NumGlobalElements()));

      last_action = Zero;

      if (input_map.SameAs(v.vector->Map()) == true)
	vector.reset (new Epetra_FEVector(*v.vector));
      else
	{
	  vector.reset (new Epetra_FEVector(input_map));
	  reinit (v, false, true);
	}
    }



    Vector::Vector (const IndexSet   &parallel_partitioner,
		    const VectorBase &v,
		    const MPI_Comm   &communicator)
                    :
                    VectorBase()
    {
      AssertThrow ((int)parallel_partitioner.size() == v.vector->Map().NumGlobalElements(),
		   ExcDimensionMismatch (parallel_partitioner.size(),
					 v.vector->Map().NumGlobalElements()));

      last_action = Zero;

      vector.reset (new Epetra_FEVector
		    (parallel_partitioner.make_trilinos_map(communicator,
							    true)));
      reinit (v, false, true);
    }



    Vector::~Vector ()
    {}



    void
    Vector::reinit (const Epetra_Map &input_map,
		    const bool        fast)
    {
      if (vector->Map().SameAs(input_map)==false)
	vector.reset (new Epetra_FEVector(input_map));
      else if (fast == false)
	{
	  const int ierr = vector->PutScalar(0.);
	  Assert (ierr == 0, ExcTrilinosError(ierr));
	}

      last_action = Zero;
    }



    void
    Vector::reinit (const IndexSet &parallel_partitioner,
		    const MPI_Comm &communicator,
		    const bool      fast)
    {
      Epetra_Map map = parallel_partitioner.make_trilinos_map (communicator,
							       true);
      reinit (map, fast);
    }



    void
    Vector::reinit (const VectorBase &v,
		    const bool        fast,
		    const bool        allow_different_maps)
    {
					// In case we do not allow to
					// have different maps, this
					// call means that we have to
					// reset the vector. So clear
					// the vector, initialize our
					// map with the map in v, and
					// generate the vector.
      if (allow_different_maps == false)
        {
	  if (vector->Map().SameAs(v.vector->Map()) == false)
	    {
	      vector.reset (new Epetra_FEVector(v.vector->Map()));
	      last_action = Zero;
	    }
	  else if (fast == false)
	    {
					       // old and new vectors
					       // have exactly the
					       // same map, i.e. size
					       // and parallel
					       // distribution
	      int ierr;
	      ierr = vector->GlobalAssemble (last_action);
	      Assert (ierr == 0, ExcTrilinosError(ierr));

	      ierr = vector->PutScalar(0.0);
	      Assert (ierr == 0, ExcTrilinosError(ierr));

	      last_action = Zero;
	    }
	}

					// Otherwise, we have to check
					// that the two vectors are
					// already of the same size,
					// create an object for the data
					// exchange and then insert all
					// the data. The first assertion
					// is only a check whether the
					// user knows what she is doing.
      else
        {
	  Assert (fast == false,
		  ExcMessage ("It is not possible to exchange data with the "
			      "option fast set, which would not write "
			      "elements."));

	  AssertThrow (size() == v.size(),
		       ExcDimensionMismatch (size(), v.size()));

	  Epetra_Import data_exchange (vector->Map(), v.vector->Map());

	  const int ierr = vector->Import(*v.vector, data_exchange, Insert);
	  AssertThrow (ierr == 0, ExcTrilinosError(ierr));

	  last_action = Insert;
	}

    }



    void
    Vector::reinit (const BlockVector &v,
		    const bool         import_data)
    {
					// In case we do not allow to
					// have different maps, this
					// call means that we have to
					// reset the vector. So clear
					// the vector, initialize our
					// map with the map in v, and
					// generate the vector.
      if (v.n_blocks() == 0)
	return;

				// create a vector that holds all the elements
				// contained in the block vector. need to
				// manually create an Epetra_Map.
      unsigned int n_elements = 0, added_elements = 0, block_offset = 0;
      for (unsigned int block=0; block<v.n_blocks();++block)
	n_elements += v.block(block).local_size();
      std::vector<int> global_ids (n_elements, -1);
      for (unsigned int block=0; block<v.n_blocks();++block)
	{
	  int * glob_elements = v.block(block).vector_partitioner().MyGlobalElements();
	  for (unsigned int i=0; i<v.block(block).local_size(); ++i)
	    global_ids[added_elements++] = glob_elements[i] + block_offset;
	  block_offset += v.block(block).size();
	}

      Assert (n_elements == added_elements, ExcInternalError());
      Epetra_Map new_map (v.size(), n_elements, &global_ids[0], 0,
			  v.block(0).vector_partitioner().Comm());

      std_cxx1x::shared_ptr<Epetra_FEVector> actual_vec;
      if ( import_data == true )
	actual_vec.reset (new Epetra_FEVector (new_map));
      else
	{
	  vector.reset (new Epetra_FEVector (new_map));
	  actual_vec = vector;
	}

      TrilinosScalar* entries = (*actual_vec)[0];
      block_offset = 0;
      for (unsigned int block=0; block<v.n_blocks();++block)
	{
	  v.block(block).trilinos_vector().ExtractCopy (entries, 0);
	  entries += v.block(block).local_size();
	}

      if (import_data == true)
        {
	  AssertThrow (static_cast<unsigned int>(actual_vec->GlobalLength())
		       == v.size(),
		       ExcDimensionMismatch (actual_vec->GlobalLength(),
					     v.size()));

	  Epetra_Import data_exchange (vector->Map(), actual_vec->Map());

	  const int ierr = vector->Import(*actual_vec, data_exchange, Insert);
	  AssertThrow (ierr == 0, ExcTrilinosError(ierr));

	  last_action = Insert;
	}

    }



    Vector &
    Vector::operator = (const Vector &v)
    {
				// distinguish three cases. First case: both
				// vectors have the same layout (just need to
				// copy the local data, not reset the memory
				// and the underlying Epetra_Map). The third
				// case means that we have to rebuild the
				// calling vector.
      if (vector->Map().SameAs(v.vector->Map()))
	{
	  *vector = *v.vector;
	  last_action = Zero;
	}
				// Second case: vectors have the same global
				// size, but different parallel layouts (and
				// one of them a one-to-one mapping). Then we
				// can call the import/export functionality.
      else if (size() == v.size() &&
	       (v.vector->Map().UniqueGIDs() || vector->Map().UniqueGIDs()))
	{
	  reinit (v, false, true);
	}
				// Third case: Vectors do not have the same
				// size.
      else
	{
	  vector.reset (new Epetra_FEVector(*v.vector));
	  last_action = Zero;
	}

      return *this;
    }



    Vector &
    Vector::operator = (const TrilinosWrappers::Vector &v)
    {
      Assert (size() == v.size(), ExcDimensionMismatch(size(), v.size()));

      Epetra_Import data_exchange (vector->Map(), v.vector->Map());
      const int ierr = vector->Import(*v.vector, data_exchange, Insert);

      AssertThrow (ierr == 0, ExcTrilinosError(ierr));

      last_action = Insert;

      return *this;
    }



    void
    Vector::import_nonlocal_data_for_fe (const TrilinosWrappers::SparseMatrix &m,
					 const Vector                         &v)
    {
      Assert (m.trilinos_matrix().Filled() == true,
	      ExcMessage ("Matrix is not compressed. "
			  "Cannot find exchange information!"));
      Assert (v.vector->Map().UniqueGIDs() == true,
	      ExcMessage ("The input vector has overlapping data, "
			  "which is not allowed."));

      if (vector->Map().SameAs(m.col_partitioner()) == false)
	{
	  Epetra_Map map = m.col_partitioner();
	  vector.reset (new Epetra_FEVector(map));
	}

      Epetra_Import data_exchange (vector->Map(), v.vector->Map());
      const int ierr = vector->Import(*v.vector, data_exchange, Insert);

      AssertThrow (ierr == 0, ExcTrilinosError(ierr));

      last_action = Insert;
    }

  } /* end of namespace MPI */




  Vector::Vector ()
  {
    last_action = Zero;
    Epetra_LocalMap map (0, 0, Utilities::Trilinos::comm_self());
    vector.reset (new Epetra_FEVector(map));
  }



  Vector::Vector (const unsigned int n)
  {
    last_action = Zero;
    Epetra_LocalMap map ((int)n, 0, Utilities::Trilinos::comm_self());
    vector.reset (new Epetra_FEVector (map));
  }



  Vector::Vector (const Epetra_Map &input_map)
  {
    last_action = Zero;
    Epetra_LocalMap map (input_map.NumGlobalElements(),
			 input_map.IndexBase(),
			 input_map.Comm());
    vector.reset (new Epetra_FEVector(map));
  }



  Vector::Vector (const IndexSet &partitioning,
		  const MPI_Comm &communicator)
  {
    last_action = Zero;
    Epetra_LocalMap map (partitioning.size(),
			 0,
#ifdef DEAL_II_COMPILER_SUPPORTS_MPI
			 Epetra_MpiComm(communicator));
#else
                         Epetra_SerialComm());
    (void)communicator;
#endif
    vector.reset (new Epetra_FEVector(map));
  }



  Vector::Vector (const VectorBase &v)
  {
    last_action = Zero;
    Epetra_LocalMap map (v.vector->Map().NumGlobalElements(),
			 v.vector->Map().IndexBase(),
			 v.vector->Map().Comm());
    vector.reset (new Epetra_FEVector(map));

    if (vector->Map().SameAs(v.vector->Map()) == true)
      {
	const int ierr = vector->Update(1.0, *v.vector, 0.0);
	AssertThrow (ierr == 0, ExcTrilinosError(ierr));
      }
    else
      reinit (v, false, true);

  }



  void
  Vector::reinit (const unsigned int n,
		  const bool         fast)
  {
    if (size() != n)
      {
	Epetra_LocalMap map ((int)n, 0,
			     Utilities::Trilinos::comm_self());
	vector.reset (new Epetra_FEVector (map));
      }
    else if (fast == false)
      {
	int ierr;
	ierr = vector->GlobalAssemble(last_action);
	Assert (ierr == 0, ExcTrilinosError(ierr));

	ierr = vector->PutScalar(0.0);
	Assert (ierr == 0, ExcTrilinosError(ierr));
      }

    last_action = Zero;
  }



  void
  Vector::reinit (const Epetra_Map &input_map,
                  const bool        fast)
  {
    if (vector->Map().NumGlobalElements() != input_map.NumGlobalElements())
      {
	Epetra_LocalMap map (input_map.NumGlobalElements(),
			     input_map.IndexBase(),
			     input_map.Comm());
	vector.reset (new Epetra_FEVector (map));
      }
    else if (fast == false)
      {
	int ierr;
	ierr = vector->GlobalAssemble(last_action);
	Assert (ierr == 0, ExcTrilinosError(ierr));

	ierr = vector->PutScalar(0.0);
	Assert (ierr == 0, ExcTrilinosError(ierr));
      }

    last_action = Zero;
  }



  void
  Vector::reinit (const IndexSet &partitioning,
		  const MPI_Comm &communicator,
                  const bool      fast)
  {
    if (vector->Map().NumGlobalElements() !=
	static_cast<int>(partitioning.size()))
      {
	Epetra_LocalMap map (partitioning.size(),
			     0,
#ifdef DEAL_II_COMPILER_SUPPORTS_MPI
			     Epetra_MpiComm(communicator));
#else
                             Epetra_SerialComm());
        (void)communicator;
#endif
	vector.reset (new Epetra_FEVector(map));
      }
    else if (fast == false)
      {
	int ierr;
	ierr = vector->GlobalAssemble(last_action);
	Assert (ierr == 0, ExcTrilinosError(ierr));

	ierr = vector->PutScalar(0.0);
	Assert (ierr == 0, ExcTrilinosError(ierr));
      }

    last_action = Zero;
  }



  void
  Vector::reinit (const VectorBase &v,
		  const bool        fast,
		  const bool        allow_different_maps)
  {
					// In case we do not allow to
					// have different maps, this
					// call means that we have to
					// reset the vector. So clear
					// the vector, initialize our
					// map with the map in v, and
					// generate the vector.
    if (allow_different_maps == false)
      {
	if (local_range() != v.local_range())
	  {
	    Epetra_LocalMap map (v.vector->GlobalLength(),
				 v.vector->Map().IndexBase(),
				 v.vector->Comm());
	    vector.reset (new Epetra_FEVector(map));
	  }
	else
	  {
	    int ierr;
	    Assert (vector->Map().SameAs(v.vector->Map()) == true,
		    ExcMessage ("The Epetra maps in the assignment operator ="
				" do not match, even though the local_range "
				" seems to be the same. Check vector setup!"));

	    ierr = vector->GlobalAssemble(last_action);
	    Assert (ierr == 0, ExcTrilinosError(ierr));

	    ierr = vector->PutScalar(0.0);
	    Assert (ierr == 0, ExcTrilinosError(ierr));
	  }
	last_action = Zero;
      }

					// Otherwise, we have to check
					// that the two vectors are
					// already of the same size,
					// create an object for the data
					// exchange and then insert all
					// the data.
    else
      {
	Assert (fast == false,
		ExcMessage ("It is not possible to exchange data with the "
			    "option fast set, which would not write "
			    "elements."));

	AssertThrow (size() == v.size(),
		     ExcDimensionMismatch (size(), v.size()));

	Epetra_Import data_exchange (vector->Map(), v.vector->Map());

	const int ierr = vector->Import(*v.vector, data_exchange, Insert);
	AssertThrow (ierr == 0, ExcTrilinosError(ierr));

	last_action = Insert;
      }

  }



  Vector &
  Vector::operator = (const MPI::Vector &v)
  {
    if (size() != v.size())
      {
	Epetra_LocalMap map (v.vector->Map().NumGlobalElements(),
			     v.vector->Map().IndexBase(),
			     v.vector->Comm());
	vector.reset (new Epetra_FEVector(map));
      }

    reinit (v, false, true);
    return *this;
  }



  Vector &
  Vector::operator = (const Vector &v)
  {
    if (size() != v.size())
      {
	Epetra_LocalMap map (v.vector->Map().NumGlobalElements(),
			     v.vector->Map().IndexBase(),
			     v.vector->Comm());
	vector.reset (new Epetra_FEVector(map));
      }

    const int ierr = vector->Update(1.0, *v.vector, 0.0);
    Assert (ierr == 0, ExcTrilinosError(ierr));

    return *this;
  }

}

DEAL_II_NAMESPACE_CLOSE

#endif // DEAL_II_USE_TRILINOS
