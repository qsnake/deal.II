//---------------------------------------------------------------------------
//    $Id: solution_transfer.cc 23709 2011-05-17 04:34:08Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <deal.II/base/memory_consumption.h>
#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/dofs/dof_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/fe/fe.h>
#include <deal.II/lac/vector.h>
#include <deal.II/lac/petsc_vector.h>
#include <deal.II/lac/trilinos_vector.h>
#include <deal.II/lac/block_vector.h>
#include <deal.II/lac/petsc_block_vector.h>
#include <deal.II/lac/trilinos_block_vector.h>
#include <deal.II/numerics/solution_transfer.h>

DEAL_II_NAMESPACE_OPEN



template<int dim, typename VECTOR, class DH>
SolutionTransfer<dim, VECTOR, DH>::SolutionTransfer(const DH &dof)
		:
		dof_handler(&dof, typeid(*this).name()),
		n_dofs_old(0),
		prepared_for(none)
{}


template<int dim, typename VECTOR, class DH>
SolutionTransfer<dim, VECTOR, DH>::~SolutionTransfer()
{
  clear ();
}


template<int dim, typename VECTOR, class DH>
SolutionTransfer<dim, VECTOR, DH>::Pointerstruct::Pointerstruct():
		indices_ptr(0),
		dof_values_ptr(0)
{}



template<int dim, typename VECTOR, class DH>
void SolutionTransfer<dim, VECTOR, DH>::clear ()
{
  indices_on_cell.clear();
  dof_values_on_cell.clear();
  cell_map.clear();

  prepared_for=none;
}


template<int dim, typename VECTOR, class DH>
void SolutionTransfer<dim, VECTOR, DH>::prepare_for_pure_refinement()
{
  Assert(prepared_for!=pure_refinement, ExcAlreadyPrepForRef());
  Assert(prepared_for!=coarsening_and_refinement,
	 ExcAlreadyPrepForCoarseAndRef());

  clear();

  const unsigned int n_active_cells = dof_handler->get_tria().n_active_cells();
  n_dofs_old=dof_handler->n_dofs();

				   // efficient reallocation of indices_on_cell
  std::vector<std::vector<unsigned int> > (n_active_cells)
    .swap(indices_on_cell);

  typename DH::active_cell_iterator cell = dof_handler->begin_active(),
				    endc = dof_handler->end();

  for (unsigned int i=0; cell!=endc; ++cell, ++i)
    {
      indices_on_cell[i].resize(cell->get_fe().dofs_per_cell);
				       // on each cell store the indices of the
				       // dofs. after refining we get the values
				       // on the children by taking these
				       // indices, getting the respective values
				       // out of the data vectors and prolonging
				       // them to the children
      cell->get_dof_indices(indices_on_cell[i]);
      cell_map[std::make_pair(cell->level(),cell->index())].indices_ptr=&indices_on_cell[i];
    }
  prepared_for=pure_refinement;
}


template<int dim, typename VECTOR, class DH>
void
SolutionTransfer<dim, VECTOR, DH>::refine_interpolate(const VECTOR &in,
						  VECTOR       &out) const
{
  Assert(prepared_for==pure_refinement, ExcNotPrepared());
  Assert(in.size()==n_dofs_old, ExcDimensionMismatch(in.size(),n_dofs_old));
  Assert(out.size()==dof_handler->n_dofs(),
	 ExcDimensionMismatch(out.size(),dof_handler->n_dofs()));
  Assert(&in != &out,
         ExcMessage ("Vectors cannot be used as input and output"
                     " at the same time!"));

  Vector<typename VECTOR::value_type> local_values(0);

  typename DH::cell_iterator cell = dof_handler->begin(),
			     endc = dof_handler->end();

  typename std::map<std::pair<unsigned int, unsigned int>, Pointerstruct>::const_iterator
    pointerstruct,
    cell_map_end=cell_map.end();

  for (; cell!=endc; ++cell)
    {
      pointerstruct=cell_map.find(std::make_pair(cell->level(),cell->index()));

      if (pointerstruct!=cell_map_end)
					 // this cell was refined or not
					 // touched at all, so we can get
					 // the new values by just setting
					 // or interpolating to the children,
					 // which is both done by one
					 // function
	{
	  const unsigned int dofs_per_cell=cell->get_fe().dofs_per_cell;
	  local_values.reinit(dofs_per_cell, true); // fast reinit, i.e. the
						    // entries are not set to 0.
					   // make sure that the size of the
					   // stored indices is the same as
					   // dofs_per_cell. this is kind of a
					   // test if we use the same fe in the
					   // hp case. to really do that test we
					   // would have to store the fe_index
					   // of all cells
	  Assert(dofs_per_cell==(*pointerstruct->second.indices_ptr).size(),
		 ExcNumberOfDoFsPerCellHasChanged());
	  for (unsigned int i=0; i<dofs_per_cell; ++i)
	    local_values(i)=in((*pointerstruct->second.indices_ptr)[i]);
	  cell->set_dof_values_by_interpolation(local_values, out);
	}
    }
}


template<int dim, typename VECTOR, class DH>
void
SolutionTransfer<dim, VECTOR, DH>::
prepare_for_coarsening_and_refinement(const std::vector<VECTOR> &all_in)
{
  Assert(prepared_for!=pure_refinement, ExcAlreadyPrepForRef());
  Assert(!prepared_for!=coarsening_and_refinement,
	 ExcAlreadyPrepForCoarseAndRef());

  const unsigned int in_size=all_in.size();
  Assert(in_size!=0, ExcNoInVectorsGiven());

  clear();

  const unsigned int n_active_cells = dof_handler->get_tria().n_active_cells();
  n_dofs_old=dof_handler->n_dofs();

  for (unsigned int i=0; i<in_size; ++i)
    {
      Assert(all_in[i].size()==n_dofs_old,
	     ExcDimensionMismatch(all_in[i].size(),n_dofs_old));
    }

				   // first count the number
				   // of cells that'll be coarsened
				   // and that'll stay or be refined
  unsigned int n_cells_to_coarsen=0;
  unsigned int n_cells_to_stay_or_refine=0;
  typename DH::active_cell_iterator
    act_cell = dof_handler->begin_active(),
    endc = dof_handler->end();
  for (; act_cell!=endc; ++act_cell)
    {
      if (act_cell->coarsen_flag_set())
	++n_cells_to_coarsen;
      else
	++n_cells_to_stay_or_refine;
    }
  Assert((n_cells_to_coarsen+n_cells_to_stay_or_refine)==n_active_cells,
	 ExcInternalError());

  unsigned int n_coarsen_fathers=0;
  typename DH::cell_iterator
    cell=dof_handler->begin();
  for (; cell!=endc; ++cell)
    if (!cell->active() && cell->child(0)->coarsen_flag_set())
      ++n_coarsen_fathers;

  if (n_cells_to_coarsen)
    Assert(n_cells_to_coarsen>=2*n_coarsen_fathers, ExcInternalError());

				   // allocate the needed memory. initialize
                                   // the following arrays in an efficient
                                   // way, without copying much
  std::vector<std::vector<unsigned int> >
    (n_cells_to_stay_or_refine)
    .swap(indices_on_cell);

  std::vector<std::vector<Vector<typename VECTOR::value_type> > >
    (n_coarsen_fathers,
     std::vector<Vector<typename VECTOR::value_type> > (in_size))
    .swap(dof_values_on_cell);

				   // we need counters for
				   // the 'to_stay_or_refine' cells 'n_sr' and
				   // the 'coarsen_fathers' cells 'n_cf',
  unsigned int n_sr=0, n_cf=0;
  cell = dof_handler->begin();
  for (; cell!=endc; ++cell)
    {
      if (cell->active() && !cell->coarsen_flag_set())
	{
	  const unsigned int dofs_per_cell=cell->get_fe().dofs_per_cell;
	  indices_on_cell[n_sr].resize(dofs_per_cell);
					   // cell will not be coarsened,
					   // so we get away by storing the
					   // dof indices and later
					   // interpolating to the children
	  cell->get_dof_indices(indices_on_cell[n_sr]);
	  cell_map[std::make_pair(cell->level(), cell->index())].indices_ptr=&indices_on_cell[n_sr];
	  ++n_sr;
	}
      else if (cell->has_children() && cell->child(0)->coarsen_flag_set())
	{
					   // note that if one child has the
					   // coarsen flag, then all should
					   // have if Tria::prepare_* has
					   // worked correctly
	  for (unsigned int i=1; i<cell->n_children(); ++i)
	    Assert(cell->child(i)->coarsen_flag_set(),
		   ExcTriaPrepCoarseningNotCalledBefore());
	  const unsigned int dofs_per_cell=cell->get_fe().dofs_per_cell;

	  std::vector<Vector<typename VECTOR::value_type> >(in_size,
								Vector<typename VECTOR::value_type>(dofs_per_cell))
	    .swap(dof_values_on_cell[n_cf]);

	  for (unsigned int j=0; j<in_size; ++j)
	    {
					       // store the data of each of
					       // the input vectors
	      cell->get_interpolated_dof_values(all_in[j],
						dof_values_on_cell[n_cf][j]);
	    }
	  cell_map[std::make_pair(cell->level(), cell->index())].dof_values_ptr=&dof_values_on_cell[n_cf];
	  ++n_cf;
	}
    }
  Assert(n_sr==n_cells_to_stay_or_refine, ExcInternalError());
  Assert(n_cf==n_coarsen_fathers, ExcInternalError());

  prepared_for=coarsening_and_refinement;
}


template<int dim, typename VECTOR, class DH>
void
SolutionTransfer<dim, VECTOR, DH>::prepare_for_coarsening_and_refinement(const VECTOR &in)
{
  std::vector<VECTOR> all_in=std::vector<VECTOR>(1, in);
  prepare_for_coarsening_and_refinement(all_in);
}


template<int dim, typename VECTOR, class DH>
void SolutionTransfer<dim, VECTOR, DH>::
interpolate (const std::vector<VECTOR> &all_in,
	     std::vector<VECTOR>       &all_out) const
{
  Assert(prepared_for==coarsening_and_refinement, ExcNotPrepared());
  const unsigned int size=all_in.size();
  Assert(all_out.size()==size, ExcDimensionMismatch(all_out.size(), size));
  for (unsigned int i=0; i<size; ++i)
    Assert (all_in[i].size() == n_dofs_old,
	    ExcDimensionMismatch(all_in[i].size(), n_dofs_old));
  for (unsigned int i=0; i<all_out.size(); ++i)
    Assert (all_out[i].size() == dof_handler->n_dofs(),
	    ExcDimensionMismatch(all_out[i].size(), dof_handler->n_dofs()));
  for (unsigned int i=0; i<size; ++i)
    for (unsigned int j=0; j<size; ++j)
      Assert(&all_in[i] != &all_out[j],
             ExcMessage ("Vectors cannot be used as input and output"
                         " at the same time!"));

  Vector<typename VECTOR::value_type> local_values;
  std::vector<unsigned int> dofs;

  typename std::map<std::pair<unsigned int, unsigned int>, Pointerstruct>::const_iterator
    pointerstruct,
    cell_map_end=cell_map.end();

  typename DH::cell_iterator cell = dof_handler->begin(),
			     endc = dof_handler->end();
  for (; cell!=endc; ++cell)
    {
      pointerstruct=cell_map.find(std::make_pair(cell->level(),cell->index()));

      if (pointerstruct!=cell_map_end)
	{
	  const std::vector<unsigned int> * const indexptr
	    =pointerstruct->second.indices_ptr;

	  const std::vector<Vector<typename VECTOR::value_type> > * const valuesptr
	    =pointerstruct->second.dof_values_ptr;

	  const unsigned int dofs_per_cell=cell->get_fe().dofs_per_cell;

					   // cell stayed or is
					   // refined
	  if (indexptr)
	    {
	      Assert (valuesptr == 0,
		      ExcInternalError());
					       // make sure that the size of the
					       // stored indices is the same as
					       // dofs_per_cell. this is kind of
					       // a test if we use the same fe
					       // in the hp case. to really do
					       // that test we would have to
					       // store the fe_index of all
					       // cells
	      Assert(dofs_per_cell==(*indexptr).size(),
		     ExcNumberOfDoFsPerCellHasChanged());
					       // get the values of
					       // each of the input
					       // data vectors on this
					       // cell and prolong it
					       // to its children
	      local_values.reinit(dofs_per_cell, true);
	      for (unsigned int j=0; j<size; ++j)
		{
		  for (unsigned int i=0; i<dofs_per_cell; ++i)
		    local_values(i)=all_in[j]((*indexptr)[i]);
		  cell->set_dof_values_by_interpolation(local_values,
							all_out[j]);
		}
	    }
					   // children of cell were
					   // deleted
	  else if (valuesptr)
	    {
	      Assert (!cell->has_children(), ExcInternalError());
	      Assert (indexptr == 0,
		      ExcInternalError());
	      dofs.resize(dofs_per_cell);
					       // get the local
					       // indices
	      cell->get_dof_indices(dofs);

					       // distribute the
					       // stored data to the
					       // new vectors
	      for (unsigned int j=0; j<size; ++j)
		{
						   // make sure that the size of
						   // the stored indices is the
						   // same as
						   // dofs_per_cell. this is
						   // kind of a test if we use
						   // the same fe in the hp
						   // case. to really do that
						   // test we would have to
						   // store the fe_index of all
						   // cells
		  Assert(dofs_per_cell==(*valuesptr)[j].size(),
			 ExcNumberOfDoFsPerCellHasChanged());

		  for (unsigned int i=0; i<dofs_per_cell; ++i)
		    all_out[j](dofs[i])=(*valuesptr)[j](i);
		}
	    }
					   // undefined status
	  else
	    Assert(false, ExcInternalError());
	}
    }
}



template<int dim, typename VECTOR, class DH>
void SolutionTransfer<dim, VECTOR, DH>::interpolate(const VECTOR &in,
						VECTOR       &out) const
{
  Assert (in.size()==n_dofs_old,
	  ExcDimensionMismatch(in.size(), n_dofs_old));
  Assert (out.size()==dof_handler->n_dofs(),
	  ExcDimensionMismatch(out.size(), dof_handler->n_dofs()));

  std::vector<VECTOR> all_in(1);
  all_in[0] = in;
  std::vector<VECTOR> all_out(1);
  all_out[0] = out;
  interpolate(all_in,
	      all_out);
  out=all_out[0];
}



template<int dim, typename VECTOR, class DH>
std::size_t
SolutionTransfer<dim, VECTOR, DH>::memory_consumption () const
{
				   // at the moment we do not include the memory
				   // consumption of the cell_map as we have no
				   // real idea about memory consumption of a
				   // std::map
  return (MemoryConsumption::memory_consumption (dof_handler) +
	  MemoryConsumption::memory_consumption (n_dofs_old) +
	  sizeof (prepared_for) +
	  MemoryConsumption::memory_consumption (indices_on_cell) +
	  MemoryConsumption::memory_consumption (dof_values_on_cell));
}



template<int dim, typename VECTOR, class DH>
std::size_t
SolutionTransfer<dim, VECTOR, DH>::Pointerstruct::memory_consumption () const
{
  return sizeof(*this);
}



/*-------------- Explicit Instantiations -------------------------------*/
#include "solution_transfer.inst"

DEAL_II_NAMESPACE_CLOSE
