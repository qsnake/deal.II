//---------------------------------------------------------------------------
//    $Id: sparse_vanka.cc 23709 2011-05-17 04:34:08Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2005, 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <deal.II/lac/sparse_vanka.templates.h>

DEAL_II_NAMESPACE_OPEN


// explicit instantiations
template class SparseVanka<float>;
template class SparseVanka<double>;

template void SparseVanka<double>::vmult<float> (Vector<float>       &dst,
						 const Vector<float> &src) const;
template void SparseVanka<double>::vmult<double> (Vector<double>       &dst,
						  const Vector<double> &src) const;


template class SparseBlockVanka<float>;
template class SparseBlockVanka<double>;

template void SparseBlockVanka<double>::vmult<float> (Vector<float>       &dst,
						      const Vector<float> &src) const;
template void SparseBlockVanka<double>::vmult<double> (Vector<double>       &dst,
						       const Vector<double> &src) const;

DEAL_II_NAMESPACE_CLOSE
