//---------------------------------------------------------------------------
//    $Id: function_parser.cc 23989 2011-08-02 09:47:19Z kronbichler $
//    Version: $Name$
//
//    Copyright (C) 2005, 2006, 2007, 2009 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------

#include <deal.II/base/function_parser.h>
#include <deal.II/base/utilities.h>
#include <deal.II/lac/vector.h>

#ifndef DEAL_II_DISABLE_PARSER
#  include <functionparser/fparser.h>
#else

namespace fparser
{
  class FunctionParser
  {};
}

#endif

DEAL_II_NAMESPACE_OPEN



template <int dim>
FunctionParser<dim>::FunctionParser(const unsigned int n_components,
				    const double       initial_time)
                :
                Function<dim>(n_components, initial_time),
                fp (0)
{
  fp = new fparser::FunctionParser[n_components];
}



template <int dim>
FunctionParser<dim>::~FunctionParser()
{
  delete[] fp;
}


#ifndef DEAL_II_DISABLE_PARSER
template <int dim>
void FunctionParser<dim>::initialize (const std::string                   &variables,
                      const std::vector<std::string>      &expressions,
                      const std::map<std::string, double> &constants,
                      const bool time_dependent,
                      const bool use_degrees)
{
    initialize (variables,
                expressions,
                constants,
                std::map< std::string, double >(),
                time_dependent,
                use_degrees);
}



template <int dim>
void FunctionParser<dim>::initialize (const std::string   &variables,
				      const std::vector<std::string>      &expressions,
				      const std::map<std::string, double> &constants,
				      const std::map<std::string, double> &units,
				      const bool time_dependent,
				      const bool use_degrees)
{
				   // We check that the number of
				   // components of this function
				   // matches the number of components
				   // passed in as a vector of
				   // strings.
  AssertThrow(this->n_components == expressions.size(),
	      ExcInvalidExpressionSize(this->n_components,
				       expressions.size()) );



  for (unsigned int i=0; i<this->n_components; ++i)
    {

                       // Add the various units to
                       // the parser.
        std::map< std::string, double >::const_iterator
            unit = units.begin(),
            endu = units.end();
        for (; unit != endu; ++unit)
        {
            const bool success = fp[i].AddUnit(unit->first, unit->second);
            AssertThrow (success,
                         ExcMessage("Invalid Unit Name [error adding a unit]"));
        }



				       // Add the various constants to
				       // the parser.
      std::map< std::string, double >::const_iterator
	constant = constants.begin(),
	endc  = constants.end();
      for(; constant != endc; ++constant)
	{
	  const bool success = fp[i].AddConstant(constant->first, constant->second);
	  AssertThrow (success, ExcMessage("Invalid Constant Name"));
	}

      const int ret_value = fp[i].Parse(expressions[i],
					variables,
					use_degrees);
      AssertThrow (ret_value == -1,
		   ExcParseError(ret_value, fp[i].ErrorMsg()));

				       // The fact that the parser did
				       // not throw an error does not
				       // mean that everything went
				       // ok... we can still have
				       // problems with the number of
				       // variables...
    }

				   // Now we define how many variables
				   // we expect to read in.  We
				   // distinguish between two cases:
				   // Time dependent problems, and not
				   // time dependent problems. In the
				   // first case the number of
				   // variables is given by the
				   // dimension plus one. In the other
				   // case, the number of variables is
				   // equal to the dimension. Once we
				   // parsed the variables string, if
				   // none of this is the case, then
				   // an exception is thrown.
  if (time_dependent)
    n_vars = dim+1;
  else
    n_vars = dim;

				   // Let's check if the number of
				   // variables is correct...
  AssertThrow (n_vars == fp[0].NVars(),
	       ExcDimensionMismatch(n_vars,fp[0].NVars()));

				   // Now set the initialization bit.
  initialized = true;
}



template <int dim>
void FunctionParser<dim>::initialize (const std::string &variables,
				      const std::string &expression,
				      const std::map<std::string, double> &constants,
				      const bool time_dependent,
				      const bool use_degrees)
{
				   // initialize with the things
				   // we got.
  initialize (variables,
              Utilities::split_string_list (expression, ';'),
              constants,
              time_dependent,
              use_degrees);
}



template <int dim>
void FunctionParser<dim>::initialize (const std::string &variables,
                      const std::string &expression,
                      const std::map<std::string, double> &constants,
                      const std::map<std::string, double> &units,
                      const bool time_dependent,
                      const bool use_degrees)
{
                   // initialize with the things
                   // we got.
  initialize (variables,
              Utilities::split_string_list (expression, ';'),
              constants,
              units,
              time_dependent,
              use_degrees);
}



template <int dim>
double FunctionParser<dim>::value (const Point<dim>  &p,
				   const unsigned int component) const
{
  Assert (initialized==true, ExcNotInitialized());
  Assert (component < this->n_components,
	  ExcIndexRange(component, 0, this->n_components));

				   // Statically allocate dim+1
				   // double variables.
  double vars[dim+1];

  for (unsigned int i=0; i<dim; ++i)
    vars[i] = p(i);

				   // We need the time variable only
				   // if the number of variables is
				   // different from the dimension. In
				   // this case it can only be dim+1,
				   // otherwise an exception would
				   // have already been thrown
  if (dim != n_vars)
    vars[dim] = this->get_time();

  double my_value = fp[component].Eval((double*)vars);

  AssertThrow (fp[component].EvalError() == 0,
	       ExcMessage(fp[component].ErrorMsg()));
  return my_value;
}



template <int dim>
void FunctionParser<dim>::vector_value (const Point<dim> &p,
					Vector<double>   &values) const
{
  Assert (initialized==true, ExcNotInitialized());
  Assert (values.size() == this->n_components,
	  ExcDimensionMismatch (values.size(), this->n_components));

				   // Statically allocates dim+1
				   // double variables.
  double vars[dim+1];

  for(unsigned int i=0; i<dim; ++i)
    vars[i] = p(i);

				   // We need the time variable only
				   // if the number of variables is
				   // different from the dimension. In
				   // this case it can only be dim+1,
				   // otherwise an exception would
				   // have already been thrown
  if(dim != n_vars)
    vars[dim] = this->get_time();

  for(unsigned int component = 0; component < this->n_components;
      ++component)
    {
      values(component) = fp[component].Eval((double*)vars);
      AssertThrow(fp[component].EvalError() == 0,
		  ExcMessage(fp[component].ErrorMsg()));
    }
}

#else


template <int dim>
void
FunctionParser<dim>::initialize(const std::string &,
				const std::vector<std::string> &,
				const std::map<std::string, double> &,
				const bool,
				const bool)
{
  Assert(false, ExcDisabled("parser"));
}


template <int dim>
void
FunctionParser<dim>::initialize(const std::string &,
				const std::string &,
				const std::map<std::string, double> &,
				const bool,
				const bool)
{
  Assert(false, ExcDisabled("parser"));
}


template <int dim>
double FunctionParser<dim>::value (
  const Point<dim> &, unsigned int) const
{
  Assert(false, ExcDisabled("parser"));
  return 0.;
}


template <int dim>
void FunctionParser<dim>::vector_value (
  const Point<dim>&, Vector<double>&) const
{
  Assert(false, ExcDisabled("parser"));
}


#endif

// Explicit Instantiations.

template class FunctionParser<1>;
template class FunctionParser<2>;
template class FunctionParser<3>;

DEAL_II_NAMESPACE_CLOSE
