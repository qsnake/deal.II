//---------------------------------------------------------------------------
//    $Id: trilinos_block_vector.cc 23709 2011-05-17 04:34:08Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 2008, 2009, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <deal.II/lac/trilinos_block_vector.h>

#ifdef DEAL_II_USE_TRILINOS

#  include <deal.II/lac/trilinos_block_sparse_matrix.h>


DEAL_II_NAMESPACE_OPEN

namespace TrilinosWrappers
{
  namespace MPI
  {
    BlockVector &
    BlockVector::operator = (const value_type s)
    {
      BaseClass::operator = (s);
      return *this;
    }



    BlockVector &
    BlockVector::operator = (const BlockVector &v)
    {
      if (this->n_blocks() != v.n_blocks())
	reinit(v.n_blocks());

      for (unsigned int i=0; i<this->n_blocks(); ++i)
	this->components[i] = v.block(i);

      collect_sizes();

      return *this;
    }



    BlockVector &
    BlockVector::operator = (const ::dealii::TrilinosWrappers::BlockVector &v)
    {
      Assert (n_blocks() == v.n_blocks(),
	      ExcDimensionMismatch(n_blocks(),v.n_blocks()));

      for (unsigned int i=0; i<this->n_blocks(); ++i)
	this->components[i] = v.block(i);

      return *this;
    }



    BlockVector::~BlockVector ()
    {}



    void
    BlockVector::reinit (const std::vector<Epetra_Map> &input_maps,
			 const bool                     fast)
    {
      const unsigned int no_blocks = input_maps.size();
      std::vector<unsigned int> block_sizes (no_blocks);

      for (unsigned int i=0; i<no_blocks; ++i)
	{
	  block_sizes[i] = input_maps[i].NumGlobalElements();
	}

      this->block_indices.reinit (block_sizes);
      if (components.size() != n_blocks())
        components.resize(n_blocks());

      for (unsigned int i=0; i<n_blocks(); ++i)
        components[i].reinit(input_maps[i], fast);

      collect_sizes();
    }



    void
    BlockVector::reinit (const std::vector<IndexSet> &parallel_partitioning,
			 const MPI_Comm              &communicator,
			 const bool                   fast)
    {
      const unsigned int no_blocks = parallel_partitioning.size();
      std::vector<unsigned int> block_sizes (no_blocks);

      for (unsigned int i=0; i<no_blocks; ++i)
	{
	  block_sizes[i] = parallel_partitioning[i].size();
	}

      this->block_indices.reinit (block_sizes);
      if (components.size() != n_blocks())
        components.resize(n_blocks());

      for (unsigned int i=0; i<n_blocks(); ++i)
        components[i].reinit(parallel_partitioning[i], communicator, fast);

      collect_sizes();
    }



    void
    BlockVector::reinit (const BlockVector& v,
			 const bool fast)
    {
      block_indices = v.get_block_indices();
      if (components.size() != n_blocks())
        components.resize(n_blocks());

      for (unsigned int i=0;i<n_blocks();++i)
        components[i].reinit(v.block(i), fast, false);

      collect_sizes();
    }



    void
    BlockVector::reinit (const unsigned int num_blocks)
    {
      std::vector<unsigned int> block_sizes (num_blocks, 0);
      this->block_indices.reinit (block_sizes);
      if (this->components.size() != this->n_blocks())
        this->components.resize(this->n_blocks());

      for (unsigned int i=0;i<this->n_blocks();++i)
        components[i].clear();

      collect_sizes();
    }



    void
    BlockVector::import_nonlocal_data_for_fe
      (const TrilinosWrappers::BlockSparseMatrix &m,
       const BlockVector                         &v)
    {
      Assert (m.n_block_rows() == v.n_blocks(),
	      ExcDimensionMismatch(m.n_block_rows(),v.n_blocks()));
      Assert (m.n_block_cols() == v.n_blocks(),
	      ExcDimensionMismatch(m.n_block_cols(),v.n_blocks()));

      if (v.n_blocks() != n_blocks())
	{
	  block_indices = v.get_block_indices();
	  components.resize(v.n_blocks());
	}

      for (unsigned int i=0; i<this->n_blocks(); ++i)
	components[i].import_nonlocal_data_for_fe(m.block(i,i), v.block(i));

      collect_sizes();
    }



    void
    BlockVector::compress (const Epetra_CombineMode last_action)
    {
      for (unsigned int i=0; i<n_blocks(); ++i)
	components[i].compress(last_action);
    }



    void BlockVector::print (std::ostream       &out,
			     const unsigned int  precision,
			     const bool          scientific,
			     const bool          across) const
    {
      for (unsigned int i=0;i<this->n_blocks();++i)
	{
	  if (across)
	    out << 'C' << i << ':';
	  else
	    out << "Component " << i << std::endl;
	  this->components[i].print(out, precision, scientific, across);
	}
    }

  } /* end of namespace MPI */






  BlockVector &
  BlockVector::operator = (const value_type s)
  {
    BaseClass::operator = (s);
    return *this;
  }



  void
  BlockVector::reinit (const std::vector<Epetra_Map> &input_maps,
		       const bool                     fast)
  {
    unsigned int no_blocks = input_maps.size();
    std::vector<unsigned int> block_sizes (no_blocks);

    for (unsigned int i=0; i<no_blocks; ++i)
      block_sizes[i] = input_maps[i].NumGlobalElements();


    this->block_indices.reinit (block_sizes);
    if (components.size() != n_blocks())
      components.resize(n_blocks());

    for (unsigned int i=0; i<n_blocks(); ++i)
      components[i].reinit(input_maps[i], fast);

    collect_sizes();
  }



  void
  BlockVector::reinit (const std::vector<IndexSet> &partitioning,
		       const MPI_Comm              &communicator,
		       const bool                   fast)
  {
    unsigned int no_blocks = partitioning.size();
    std::vector<unsigned int> block_sizes (no_blocks);

    for (unsigned int i=0; i<no_blocks; ++i)
      block_sizes[i] = partitioning[i].size();


    this->block_indices.reinit (block_sizes);
    if (components.size() != n_blocks())
      components.resize(n_blocks());

    for (unsigned int i=0; i<n_blocks(); ++i)
      components[i].reinit(partitioning[i], communicator, fast);

    collect_sizes();
  }



  void
  BlockVector::reinit (const std::vector<unsigned int> &block_sizes,
		       const bool                       fast)
  {
    this->block_indices.reinit (block_sizes);
    if (components.size() != n_blocks())
      components.resize(n_blocks());

    for (unsigned int i=0; i<n_blocks(); ++i)
      components[i].reinit(block_sizes[i], fast);

    collect_sizes();
  }



  void
  BlockVector::reinit (const MPI::BlockVector &v)
  {
    block_indices = v.get_block_indices();
    if (components.size() != n_blocks())
      components.resize(n_blocks());

    for (unsigned int i=0;i<n_blocks();++i)
      components[i] = v.block(i);
  }



  void
  BlockVector::reinit (const unsigned int num_blocks)
  {
    std::vector<unsigned int> block_sizes (num_blocks, 0);
    block_indices.reinit (block_sizes);
    if (components.size() != n_blocks())
      components.resize(n_blocks());

    for (unsigned int i=0;i<n_blocks();++i)
      block(i).clear();

    collect_sizes();
  }



  void
  BlockVector::reinit (const BlockVector &v,
		       const bool         fast)
  {
    block_indices = v.get_block_indices();
    if (components.size() != n_blocks())
      components.resize(n_blocks());

    for (unsigned int i=0;i<n_blocks();++i)
      components[i].reinit(v.block(i), fast);

    collect_sizes();
  }



  BlockVector &
  BlockVector::operator = (const MPI::BlockVector &v)
  {
    reinit (v);

    return *this;
  }



  BlockVector &
  BlockVector::operator = (const BlockVector &v)
  {
    if (n_blocks() != v.n_blocks())
      {
	std::vector<unsigned int> block_sizes (v.n_blocks(), 0);
	block_indices.reinit (block_sizes);
	if (components.size() != n_blocks())
	  components.resize(n_blocks());
      }

    for (unsigned int i=0; i<this->n_blocks(); ++i)
      this->components[i] = v.block(i);

    collect_sizes();

    return *this;
  }



  void BlockVector::print (std::ostream       &out,
			   const unsigned int  precision,
			   const bool          scientific,
			   const bool          across) const
  {
    for (unsigned int i=0;i<this->n_blocks();++i)
      {
	if (across)
	  out << 'C' << i << ':';
	else
	  out << "Component " << i << std::endl;
	this->components[i].print(out, precision, scientific, across);
      }
  }

}


DEAL_II_NAMESPACE_CLOSE

#endif
