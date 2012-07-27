//---------------------------------------------------------------------------
//    $Id: convergence_table.h 23876 2011-06-28 18:21:51Z kanschat $
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__convergence_table_h
#define __deal2__convergence_table_h


#include <deal.II/base/config.h>
#include <deal.II/base/table_handler.h>

DEAL_II_NAMESPACE_OPEN


/**
 * The ConvergenceTable class is an application to the TableHandler
 * class and stores some convergence data, such as residuals of the
 * cg-method, or some evaluated <i>L<sup>2</sup></i>-errors of
 * discrete solutions, etc, and evaluates convergence rates or orders.
 *
 * The already implemented #RateMode's are #reduction_rate, 
 * where the convergence rate is the quotient of two following rows, and 
 * #reduction_rate_log2, that evaluates the order of convergence.
 * These standard evaluations are useful for global refinement, for local refinement
 * this may not be an appropriate method, as the convergence rates should be
 * set in relation to the number of cells or the number of DoFs. The
 * implementations of these non-standard methods is left to a user.
 *
 * The number of cells and the number of DoFs may be added to the table by
 * calling e.g.  <tt>add_value("n cells", n_cells)</tt>. The table data is
 * also added by calling add_value().  Before the output of the table the
 * functions evaluate_convergence_rates() and evaluate_all_convergence_rates()
 * may be called (even multiply).
 *
 * There are two possibilities of how to evaluate the convergence rates of multiple 
 * columns in the same RateMode.
 * <ol>
 * <li> call evaluate_convergence_rates() for all wanted columns
 * <li> call omit_column_from_convergence_rate_evaluation() for all
 * NOT wanted columns and then 
 * evaluate_all_convergence_rates() to evaluate the convergence rates of all columns
 * that are not signed to be omitted.
 * </ol>
 *
 * A detailed discussion of this class can also be found in the step-7 and
 * step-13 example programs.
 *
 * @ingroup textoutput
 * @author Ralf Hartmann, 1999
 */
class ConvergenceTable: public TableHandler
{
  public:
				     /**
				      * Constructor.
				      */
    ConvergenceTable();
    
				     /**
				      * Rate in relation to the rows.
				      */
    enum RateMode {
					   /**
					    * Do not do anything.
					    */
	  none,
					   /**
					    * Quotient of values in
					    * the previous row and in
					    * this row.
					    */
	  reduction_rate,
					   /**
					    * Logarithm of
					    * #reduction_rate to the
					    * base 2 representing the
					    * order of convergence
					    * when halving the grid
					    * size, e.g. from h to
					    * h/2.
					    */
	  reduction_rate_log2
    };
    
				     /**
				      * Evaluates the convergence rates of the
				      * data column <tt>data_column_key</tt> 
				      * due to the #RateMode in relation to
				      * the reference column <tt>reference_column_key</tt>.
				      * Be sure that the value types of the
				      * table entries of the
				      * data column and the reference data column
				      * is a number, i.e. double, float,
				      * (unsigned) int, and so on.
				      *
				      * The new rate column and the data column
				      * will be merged to a supercolumn. The
				      * tex caption of the supercolumn will be
				      * (by default) the same as the one of the
				      * data column. This may be changed by using
				      * the <tt>set_tex_supercaption (...)</tt> function
				      * of the base class TableHandler.
				      *
				      * This method behaves in the following way: 
				      * 
				      * If RateMode is reduction_rate, then the computed
				      * output is 
				      * $ \frac{e_{n-1}/k_{n-1}}{e_n/k_n} $.
				      *
				      * Where $k$ is the reference column.
				      *
				      * If RateMode is reduction_rate_log2, then the 
				      * computed output is
				      * $
				      * 2\frac{\log |e_{n-1}/e_{n}|}{\log |k_n/k_{n-1}|} 
				      * $.
				      *
				      * This is useful, for example, if we use as 
				      * reference key the number of degrees of freedom. 
				      * Assuming that the error is proportional to 
				      * $ C (1/\sqrt{k})^r $, then this method will 
				      * produce the rate $r$ as a result.
				      */
    void
    evaluate_convergence_rates (const std::string &data_column_key,
                                const std::string &reference_column_key,
                                const RateMode     rate_mode);


				     /**
				      * Evaluates the convergence rates of the
				      * data column <tt>data_column_key</tt> 
				      * due to the #RateMode.
				      * Be sure that the value types of the
				      * table entries of the data column
				      * is a number, i.e. double, float,
				      * (unsigned) int, and so on.
				      *
				      * The new rate column and the data column
				      * will be merged to a supercolumn. The
				      * tex caption of the supercolumn will be
				      * (by default) the same as the one of the
				      * data column. This may be changed by using
				      * the set_tex_supercaption() function
				      * of the base class TableHandler.
				      */
    void
    evaluate_convergence_rates (const std::string &data_column_key, 
                                const RateMode     rate_mode);

				     /**
				      * Omit this column <tt>key</tt> 
				      * (not supercolumn!) from the
				      * evaluation of the convergence rates
				      * of `all' columns (see the following 
				      * two functions).
				      *
				      * The Column::flag==1 is reserved for
				      * omitting the column from convergence
				      * rate evalution.
				      */
    void
    omit_column_from_convergence_rate_evaluation(const std::string &key);

				     /**
				      * Evaluates convergence rates
				      * due to the <tt>rate_mode</tt>
				      * in relation to the reference
				      * column
				      * <tt>reference_column_key</tt>. This
				      * function evaluates the rates
				      * of ALL columns except of the
				      * columns that are to be omitted
				      * (see previous function) and
				      * execpt of the columns that are
				      * previously evaluated rate
				      * columns.  This function allows
				      * to evaluate the convergence
				      * rate for almost all columns of
				      * a table without calling
				      * evaluate_convergence_rates()
				      * for each column separately.
				      *
				      * Example: 
				      * Columns like <tt>n cells</tt> or 
				      * <tt>n dofs</tt> columns may be wanted
				      * to be omitted in the evaluation
				      * of the convergence rates. Hence they
				      * should omitted by calling the
				      * omit_column_from_convergence_rate_evaluation().
				      */
    void
    evaluate_all_convergence_rates(const std::string &reference_column_key,
                                   const RateMode     rate_mode);

				     /**
				      * Evaluates convergence rates
				      * due to the <tt>rate_mode</tt>. This
				      * function evaluates the rates of
				      * ALL columns except of the 
				      * columns that are to be omitted
				      * (see previous function)
				      * and execpt of the columns that are
				      * previously
				      * evaluated rate columns.
				      * This function allows to evaluate
				      * the convergence rate for almost all
				      * columns of a table without calling
				      * evaluate_convergence_rates()
				      * for each column seperately.
				      *
				      * Example: 
				      * Columns like <tt>n cells</tt> or 
				      * <tt>n dofs</tt> columns may be wanted
				      * to be omitted in the evaluation
				      * of the convergence rates. Hence they
				      * should omitted by calling the
				      * omit_column_from_convergence_rate_evaluation().
				      */
    void
    evaluate_all_convergence_rates(const RateMode rate_mode);
    
				     /** @addtogroup Exceptions
				      * @{ */
    
    				     /**
				      * Exception
				      */
    DeclException0 (ExcWrongValueType);

    				     /**
				      * Exception
				      */
    DeclException1 (ExcRateColumnAlreadyExists,
		    std::string,
		    << "Rate column <" << arg1 << "> does already exist.");
				     //@}
};


DEAL_II_NAMESPACE_CLOSE

#endif
