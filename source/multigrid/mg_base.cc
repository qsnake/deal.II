//---------------------------------------------------------------------------
//    $Id: mg_base.cc 23709 2011-05-17 04:34:08Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <deal.II/lac/vector.h>
#include <deal.II/lac/block_vector.h>
#include <deal.II/multigrid/mg_base.h>


DEAL_II_NAMESPACE_OPEN


template <class VECTOR>
MGTransferBase<VECTOR>::~MGTransferBase()
{}


template <class VECTOR>
MGMatrixBase<VECTOR>::~MGMatrixBase()
{}


template <class VECTOR>
MGSmootherBase<VECTOR>::~MGSmootherBase()
{}


template <class VECTOR>
MGCoarseGridBase<VECTOR>::~MGCoarseGridBase()
{}


// Explicit instantiations

//TODO: Use the template expander script for this
template class MGTransferBase<dealii::Vector<double> >;
template class MGTransferBase<dealii::Vector<float> >;
template class MGTransferBase<BlockVector<double> >;
template class MGTransferBase<BlockVector<float> >;

template class MGMatrixBase<dealii::Vector<double> >;
template class MGMatrixBase<dealii::Vector<float> >;
template class MGMatrixBase<BlockVector<double> >;
template class MGMatrixBase<BlockVector<float> >;

template class MGSmootherBase<dealii::Vector<float> >;
template class MGSmootherBase<dealii::Vector<double> >;
template class MGSmootherBase<BlockVector<float> >;
template class MGSmootherBase<BlockVector<double> >;

template class MGCoarseGridBase<dealii::Vector<double> >;
template class MGCoarseGridBase<dealii::Vector<float> >;
template class MGCoarseGridBase<BlockVector<double> >;
template class MGCoarseGridBase<BlockVector<float> >;

DEAL_II_NAMESPACE_CLOSE
