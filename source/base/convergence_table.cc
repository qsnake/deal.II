//---------------------------------------------------------------------------
//    $Id: convergence_table.cc 23709 2011-05-17 04:34:08Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2007 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <deal.II/base/convergence_table.h>
#include <cmath>

DEAL_II_NAMESPACE_OPEN

ConvergenceTable::ConvergenceTable()
{}


void ConvergenceTable::evaluate_convergence_rates(const std::string &data_column_key,
						  const std::string &reference_column_key,
						  const RateMode     rate_mode)
{
  Assert(columns.count(data_column_key),
	 ExcColumnNotExistent(data_column_key));
  Assert(columns.count(reference_column_key),
	 ExcColumnNotExistent(reference_column_key));
 
  if (rate_mode==none)
    return;
 
  std::vector<TableEntryBase *> &entries=columns[data_column_key].entries;
  std::vector<TableEntryBase *> &ref_entries=columns[reference_column_key].entries;
  std::string rate_key=data_column_key;

  const unsigned int n=entries.size();
  const unsigned int n_ref=ref_entries.size();
  Assert(n == n_ref, ExcDimensionMismatch(n, n_ref));
  
  std::vector<double> values(n);
  std::vector<double> ref_values(n_ref);
  
  for (unsigned int i=0; i<n; ++i)
    {
      if (dynamic_cast<TableEntry<double>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<double>*>(entries[i])->value();
      else if (dynamic_cast<TableEntry<float>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<float>*>(entries[i])->value();
      else if (dynamic_cast<TableEntry<int>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<int>*>(entries[i])->value();
      else if (dynamic_cast<TableEntry<unsigned int>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<unsigned int>*>(entries[i])->value();
      else
	Assert(false, ExcWrongValueType());
      
      // And now the reference values.
      if (dynamic_cast<TableEntry<double>*>(ref_entries[i]) != 0)
	ref_values[i]=dynamic_cast<TableEntry<double>*>(ref_entries[i])->value();
      else if (dynamic_cast<TableEntry<float>*>(ref_entries[i]) != 0)
	ref_values[i]=dynamic_cast<TableEntry<float>*>(ref_entries[i])->value();
      else if (dynamic_cast<TableEntry<int>*>(ref_entries[i]) != 0)
	ref_values[i]=dynamic_cast<TableEntry<int>*>(ref_entries[i])->value();
      else if (dynamic_cast<TableEntry<unsigned int>*>(ref_entries[i]) != 0)
	ref_values[i]=dynamic_cast<TableEntry<unsigned int>*>(ref_entries[i])->value();
      else
	Assert(false, ExcWrongValueType());
    }
  
  switch (rate_mode)
    {
      case none:
	    break;
      case reduction_rate:
	    rate_key+="red.rate";
	    Assert(columns.count(rate_key)==0, ExcRateColumnAlreadyExists(rate_key));
					     // no value available for the
					     // first row
	    add_value(rate_key, std::string("-"));
	    for (unsigned int i=1; i<n; ++i)
		add_value(rate_key, values[i-1]/values[i] * 
			            ref_values[i]/ref_values[i-1]);
	    break;
      case reduction_rate_log2:
	    rate_key+="red.rate";
	    Assert(columns.count(rate_key)==0, ExcRateColumnAlreadyExists(rate_key));
					     // no value available for the
					     // first row
	    add_value(rate_key, std::string("-"));
	    for (unsigned int i=1; i<n; ++i)
		add_value(rate_key, 2*std::log(std::fabs(values[i-1]/values[i])) /
			  std::log(std::fabs(ref_values[i]/ref_values[i-1])));
	    break;
      default:
	    Assert(false, ExcNotImplemented());  
    }

  Assert(columns.count(rate_key), ExcInternalError());  
  columns[rate_key].flag=1;
  set_precision(rate_key, 2);

  std::string superkey=data_column_key;
  if (!supercolumns.count(superkey))
    {
      add_column_to_supercolumn(data_column_key, superkey);
      set_tex_supercaption(superkey, columns[data_column_key].tex_caption);
    }

  add_column_to_supercolumn(rate_key, superkey);
}



void
ConvergenceTable::evaluate_convergence_rates(const std::string &data_column_key,
                                             const RateMode     rate_mode)
{
  Assert(columns.count(data_column_key), ExcColumnNotExistent(data_column_key));
  
  std::vector<TableEntryBase *> &entries=columns[data_column_key].entries;
  std::string rate_key=data_column_key+"...";

  const unsigned int n=entries.size();
  
  std::vector<double> values(n);
  for (unsigned int i=0; i<n; ++i)
    {
      if (dynamic_cast<TableEntry<double>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<double>*>(entries[i])->value();
      else if (dynamic_cast<TableEntry<float>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<float>*>(entries[i])->value();
      else if (dynamic_cast<TableEntry<int>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<int>*>(entries[i])->value();
      else if (dynamic_cast<TableEntry<unsigned int>*>(entries[i]) != 0)
	values[i]=dynamic_cast<TableEntry<unsigned int>*>(entries[i])->value();
      else
	Assert(false, ExcWrongValueType());
    }
  
  switch (rate_mode)
    {
      case none:
	    break;

      case reduction_rate:
	    rate_key+="red.rate";
	    Assert(columns.count(rate_key)==0, ExcRateColumnAlreadyExists(rate_key));
					     // no value available for the
					     // first row
	    add_value(rate_key, std::string("-"));
	    for (unsigned int i=1; i<n; ++i)
	      add_value(rate_key, values[i-1]/values[i]);
	    break;

      case reduction_rate_log2:
	    rate_key+="red.rate.log2";
	    Assert(columns.count(rate_key)==0, ExcRateColumnAlreadyExists(rate_key));
					     // no value availble for the
					     // first row
	    add_value(rate_key, std::string("-"));
	    for (unsigned int i=1; i<n; ++i)
	      add_value(rate_key, std::log(std::fabs(values[i-1]/values[i]))/std::log(2.0));
	    break;

      default:
	    ExcNotImplemented();  
    }

  Assert(columns.count(rate_key), ExcInternalError());  
  columns[rate_key].flag=1;
  set_precision(rate_key, 2);
  
				   // set the superkey equal to the key
  std::string superkey=data_column_key;
				   // and set the tex caption of the supercolumn
				   // to the tex caption of the data_column.
  if (!supercolumns.count(superkey))
    {
      add_column_to_supercolumn(data_column_key, superkey);
      set_tex_supercaption(superkey, columns[data_column_key].tex_caption);
    }

  add_column_to_supercolumn(rate_key, superkey);
}



void
ConvergenceTable::omit_column_from_convergence_rate_evaluation(const std::string &key)
{
  Assert(columns.count(key), ExcColumnNotExistent(key));
  
  const std::map<std::string, Column>::iterator col_iter=columns.find(key);
  col_iter->second.flag=1;
}



void
ConvergenceTable::evaluate_all_convergence_rates(const std::string &reference_column_key,
                                                 const RateMode rate_mode)
{
  for (std::map<std::string, Column>::const_iterator col_iter=columns.begin();
       col_iter!=columns.end(); ++col_iter)
    if (!col_iter->second.flag)
      evaluate_convergence_rates(col_iter->first, reference_column_key, rate_mode);
}



void
ConvergenceTable::evaluate_all_convergence_rates(const RateMode rate_mode)
{
  for (std::map<std::string, Column>::const_iterator col_iter=columns.begin();
       col_iter!=columns.end(); ++col_iter)
    if (!col_iter->second.flag)
      evaluate_convergence_rates(col_iter->first, rate_mode);
}

DEAL_II_NAMESPACE_CLOSE
