//---------------------------------------------------------------------------
//    $Id: petsc_precondition.h 23876 2011-06-28 18:21:51Z kanschat $
//
//    Copyright (C) 2004, 2005, 2006, 2007, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__petsc_precondition_h
#define __deal2__petsc_precondition_h


#include <deal.II/base/config.h>

#ifdef DEAL_II_USE_PETSC

#  include <deal.II/lac/exceptions.h>
#  include <petscpc.h>

DEAL_II_NAMESPACE_OPEN



namespace PETScWrappers
{
                                   // forward declarations
  class MatrixBase;
  class VectorBase;
  class SolverBase;


/**
 * Base class for preconditioner classes using the PETSc functionality. The
 * classes in this hierarchy don't do a whole lot, except for providing a
 * function that sets the preconditioner and certain parameters on the
 * preconditioning context of the solver. These classes are basically here
 * only to allow a similar interface as already used for the deal.II solver
 * and preconditioner classes.
 *
 * Note that derived classes only provide interfaces to the relevant
 * functionality of PETSc. PETSc does not implement all preconditioners for
 * all matrix types. In particular, some preconditioners are not going to work
 * for parallel jobs, such as for example the ILU preconditioner.
 *
 * @ingroup PETScWrappers
 * @author Wolfgang Bangerth, Timo Heister, 2004, 2011
 */
  class PreconditionerBase
  {
    public:
                                       /**
                                        * Constructor.
                                        */
      PreconditionerBase ();

                                       /**
                                        * Destructor.
                                        */
      virtual ~PreconditionerBase ();

				       /**
					* Apply the preconditioner once to the
					* given src vector.
					*/
      void vmult (VectorBase       &dst,
		  const VectorBase &src) const;


				       /**
					* Gives access to the underlying PETSc
					* object.
					*/
      const PC & get_pc () const;
      
    protected:
				       /**
					* the PETSc preconditioner object
					*/
      PC pc;
      
                                       /**
                                        * A pointer to the matrix that acts as
                                        * a preconditioner.
                                        */
      Mat matrix;

				       /**
					* Internal function to create the
					* PETSc preconditioner object. Fails
					* if called twice.
					*/
      void create_pc ();
      
                                       /**
                                        * Conversion operator to get a
                                        * representation of the matrix that
                                        * represents this preconditioner. We
                                        * use this inside the actual solver,
                                        * where we need to pass this matrix to
                                        * the PETSc solvers.
                                        */
      operator Mat () const;
      
                                       /**
                                        * Make the solver class a friend,
                                        * since it needs to call the
                                        * conversion operator.
                                        */
      friend class SolverBase;
  };



/**
 * A class that implements the interface to use the PETSc Jacobi
 * preconditioner.
 *
 * See the comment in the base class @ref PreconditionerBase for when this
 * preconditioner may or may not work.
 *
 * @ingroup PETScWrappers
 * @author Wolfgang Bangerth, Timo Heister, 2004, 2011
 */
  class PreconditionJacobi : public PreconditionerBase
  {
    public:
                                       /**
                                        * Standardized data struct to
                                        * pipe additional flags to the
                                        * preconditioner.
                                        */
      struct AdditionalData
      {};

				       /**
					* Empty Constructor. You need to call
					* initialize() before using this
					* object.
					*/
      PreconditionJacobi ();
      
      
                                       /**
                                        * Constructor. Take the matrix which
                                        * is used to form the preconditioner,
                                        * and additional flags if there are
                                        * any.
                                        */
      PreconditionJacobi (const MatrixBase     &matrix,
                          const AdditionalData &additional_data = AdditionalData());

				       /**
					* Initializes the preconditioner
					* object and calculate all data that
					* is necessary for applying it in a
					* solver. This function is
					* automatically called when calling
					* the constructor with the same
					* arguments and is only used if you
					* create the preconditioner without
					* arguments.
					*/
      void initialize (const MatrixBase     &matrix,
		       const AdditionalData &additional_data = AdditionalData());

    protected:
                                       /**
                                        * Store a copy of the flags for this
                                        * particular preconditioner.
                                        */
      AdditionalData additional_data;
  };



/**
 * A class that implements the interface to use the PETSc Block Jacobi
 * preconditioner. The blocking structure of the matrix is determined by the
 * association of degrees of freedom to the individual processors in an
 * MPI-parallel job. If you use this preconditioner on a sequential job (or an
 * MPI job with only one process) then the entire matrix is the only block.
 *
 * By default, PETSc uses an ILU(0) decomposition of each diagonal block of
 * the matrix for preconditioning. This can be changed, as is explained in the
 * relevant section of the PETSc manual, but is not implemented here.
 *
 * See the comment in the base class @ref PreconditionerBase for when this
 * preconditioner may or may not work.
 *
 * @ingroup PETScWrappers
 * @author Wolfgang Bangerth, Timo Heister, 2004, 2011
 */
  class PreconditionBlockJacobi : public PreconditionerBase
  {
    public:
                                       /**
                                        * Standardized data struct to
                                        * pipe additional flags to the
                                        * preconditioner.
                                        */
      struct AdditionalData
      {};
      
				       /**
					* Empty Constructor. You need to call
					* initialize() before using this
					* object.
					*/
      PreconditionBlockJacobi ();

                                       /**
                                        * Constructor. Take the matrix which
                                        * is used to form the preconditioner,
                                        * and additional flags if there are
                                        * any.
                                        */
      PreconditionBlockJacobi (const MatrixBase     &matrix,
                               const AdditionalData &additional_data = AdditionalData());

				       /**
					* Initializes the preconditioner
					* object and calculate all data that
					* is necessary for applying it in a
					* solver. This function is
					* automatically called when calling
					* the constructor with the same
					* arguments and is only used if you
					* create the preconditioner without
					* arguments.
					*/
      void initialize (const MatrixBase     &matrix,
		       const AdditionalData &additional_data = AdditionalData());

    protected:
                                       /**
                                        * Store a copy of the flags for this
                                        * particular preconditioner.
                                        */
      AdditionalData additional_data;
  };



/**
 * A class that implements the interface to use the PETSc SOR
 * preconditioner.
 *
 * See the comment in the base class @ref PreconditionerBase for when this
 * preconditioner may or may not work.
 *
 * @ingroup PETScWrappers
 * @author Wolfgang Bangerth, Timo Heister, 2004, 2011
 */
  class PreconditionSOR : public PreconditionerBase
  {
    public:
                                       /**
                                        * Standardized data struct to
                                        * pipe additional flags to the
                                        * preconditioner.
                                        */
      struct AdditionalData
      {
                                           /**
                                            * Constructor. By default,
                                            * set the damping parameter
                                            * to one.
                                            */
          AdditionalData (const double omega = 1);

                                           /**
                                            * Relaxation parameter.
                                            */
          double omega;
      };

				       /**
					* Empty Constructor. You need to call
					* initialize() before using this
					* object.
					*/
      PreconditionSOR ();

                                       /**
                                        * Constructor. Take the matrix which
                                        * is used to form the preconditioner,
                                        * and additional flags if there are
                                        * any.
                                        */
      PreconditionSOR (const MatrixBase     &matrix,
                       const AdditionalData &additional_data = AdditionalData());

				       /**
					* Initializes the preconditioner
					* object and calculate all data that
					* is necessary for applying it in a
					* solver. This function is
					* automatically called when calling
					* the constructor with the same
					* arguments and is only used if you
					* create the preconditioner without
					* arguments.
					*/
      void initialize (const MatrixBase     &matrix,
                       const AdditionalData &additional_data = AdditionalData());

    protected:
                                       /**
                                        * Store a copy of the flags for this
                                        * particular preconditioner.
                                        */
      AdditionalData additional_data;
  };



/**
 * A class that implements the interface to use the PETSc SSOR
 * preconditioner.
 *
 * See the comment in the base class @ref PreconditionerBase for when this
 * preconditioner may or may not work.
 *
 * @ingroup PETScWrappers
 * @author Wolfgang Bangerth, Timo Heister, 2004, 2011
 */
  class PreconditionSSOR : public PreconditionerBase
  {
    public:
                                       /**
                                        * Standardized data struct to
                                        * pipe additional flags to the
                                        * preconditioner.
                                        */
      struct AdditionalData
      {
                                           /**
                                            * Constructor. By default,
                                            * set the damping parameter
                                            * to one.
                                            */
          AdditionalData (const double omega = 1);

                                           /**
                                            * Relaxation parameter.
                                            */
          double omega;
      };

				       /**
					* Empty Constructor. You need to call
					* initialize() before using this
					* object.
					*/
      PreconditionSSOR ();

                                       /**
                                        * Constructor. Take the matrix which
                                        * is used to form the preconditioner,
                                        * and additional flags if there are
                                        * any.
                                        */
      PreconditionSSOR (const MatrixBase     &matrix,
                        const AdditionalData &additional_data = AdditionalData());

				       /**
					* Initializes the preconditioner
					* object and calculate all data that
					* is necessary for applying it in a
					* solver. This function is
					* automatically called when calling
					* the constructor with the same
					* arguments and is only used if you
					* create the preconditioner without
					* arguments.
					*/
      void initialize (const MatrixBase     &matrix,
                       const AdditionalData &additional_data = AdditionalData());

    protected:
                                       /**
                                        * Store a copy of the flags for this
                                        * particular preconditioner.
                                        */
      AdditionalData additional_data;
  };



/**
 * A class that implements the interface to use the PETSc Eisenstat
 * preconditioner.
 *
 * See the comment in the base class @ref PreconditionerBase for when this
 * preconditioner may or may not work.
 *
 * @ingroup PETScWrappers
 * @author Wolfgang Bangerth, Timo Heister, 2004, 2011
 */
  class PreconditionEisenstat : public PreconditionerBase
  {
    public:
                                       /**
                                        * Standardized data struct to
                                        * pipe additional flags to the
                                        * preconditioner.
                                        */
      struct AdditionalData
      {
                                           /**
                                            * Constructor. By default,
                                            * set the damping parameter
                                            * to one.
                                            */
          AdditionalData (const double omega = 1);

                                           /**
                                            * Relaxation parameter.
                                            */
          double omega;
      };

				       /**
					* Empty Constructor. You need to call
					* initialize() before using this
					* object.
					*/
      PreconditionEisenstat ();

                                       /**
                                        * Constructor. Take the matrix which
                                        * is used to form the preconditioner,
                                        * and additional flags if there are
                                        * any.
                                        */
      PreconditionEisenstat (const MatrixBase     &matrix,
                             const AdditionalData &additional_data = AdditionalData());

				       /**
					* Initializes the preconditioner
					* object and calculate all data that
					* is necessary for applying it in a
					* solver. This function is
					* automatically called when calling
					* the constructor with the same
					* arguments and is only used if you
					* create the preconditioner without
					* arguments.
					*/
      void initialize (const MatrixBase     &matrix,
                       const AdditionalData &additional_data = AdditionalData());

    protected:
                                       /**
                                        * Store a copy of the flags for this
                                        * particular preconditioner.
                                        */
      AdditionalData additional_data;
  };



/**
 * A class that implements the interface to use the PETSc Incomplete Cholesky
 * preconditioner.
 *
 * See the comment in the base class @ref PreconditionerBase for when this
 * preconditioner may or may not work.
 *
 * @ingroup PETScWrappers
 * @author Wolfgang Bangerth, Timo Heister, 2004, 2011
 */
  class PreconditionICC : public PreconditionerBase
  {
    public:
                                       /**
                                        * Standardized data struct to
                                        * pipe additional flags to the
                                        * preconditioner.
                                        */
      struct AdditionalData
      {
                                           /**
                                            * Constructor. By default,
                                            * set the fill-in parameter
                                            * to zero.
                                            */
          AdditionalData (const unsigned int levels = 0);

                                           /**
                                            * Fill-in parameter.
                                            */
          unsigned int levels;
      };

				       /**
					* Empty Constructor. You need to call
					* initialize() before using this
					* object.
					*/
      PreconditionICC ();

                                       /**
                                        * Constructor. Take the matrix which
                                        * is used to form the preconditioner,
                                        * and additional flags if there are
                                        * any.
                                        */
      PreconditionICC (const MatrixBase     &matrix,
                       const AdditionalData &additional_data = AdditionalData());

				       /**
					* Initializes the preconditioner
					* object and calculate all data that
					* is necessary for applying it in a
					* solver. This function is
					* automatically called when calling
					* the constructor with the same
					* arguments and is only used if you
					* create the preconditioner without
					* arguments.
					*/
      void initialize (const MatrixBase     &matrix,
                       const AdditionalData &additional_data = AdditionalData());

    protected:
                                       /**
                                        * Store a copy of the flags for this
                                        * particular preconditioner.
                                        */
      AdditionalData additional_data;
  };



/**
 * A class that implements the interface to use the PETSc ILU
 * preconditioner.
 *
 * See the comment in the base class @ref PreconditionerBase for when this
 * preconditioner may or may not work.
 *
 * @ingroup PETScWrappers
 * @author Wolfgang Bangerth, Timo Heister, 2004, 2011
 */
  class PreconditionILU : public PreconditionerBase
  {
    public:
                                       /**
                                        * Standardized data struct to
                                        * pipe additional flags to the
                                        * preconditioner.
                                        */
      struct AdditionalData
      {
                                           /**
                                            * Constructor. By default,
                                            * set the fill-in parameter
                                            * to zero.
                                            */
          AdditionalData (const unsigned int levels = 0);

                                           /**
                                            * Fill-in parameter.
                                            */
          unsigned int levels;
      };

				       /**
					* Empty Constructor. You need to call
					* initialize() before using this
					* object.
					*/
      PreconditionILU ();

                                       /**
                                        * Constructor. Take the matrix which
                                        * is used to form the preconditioner,
                                        * and additional flags if there are
                                        * any.
                                        */
      PreconditionILU (const MatrixBase     &matrix,
                       const AdditionalData &additional_data = AdditionalData());

				       /**
					* Initializes the preconditioner
					* object and calculate all data that
					* is necessary for applying it in a
					* solver. This function is
					* automatically called when calling
					* the constructor with the same
					* arguments and is only used if you
					* create the preconditioner without
					* arguments.
					*/
      void initialize (const MatrixBase     &matrix,
                       const AdditionalData &additional_data = AdditionalData());

    protected:
                                       /**
                                        * Store a copy of the flags for this
                                        * particular preconditioner.
                                        */
      AdditionalData additional_data;
  };



/**
 * A class that implements the interface to use the PETSc LU
 * preconditioner. The LU decomposition is only implemented for single
 * processor machines. It should provide a convenient interface to
 * another direct solver.
 *
 * See the comment in the base class @ref PreconditionerBase for when this
 * preconditioner may or may not work.
 *
 * @ingroup PETScWrappers
 * @author Oliver Kayser-Herold, 2004
 */
  class PreconditionLU : public PreconditionerBase
  {
    public:
                                       /**
                                        * Standardized data struct to
                                        * pipe additional flags to the
                                        * preconditioner.
                                        */
      struct AdditionalData
      {
                                           /**
                                            * Constructor. (Default values
					    * taken from function PCCreate_LU
					    * of the PetSC lib.)
                                            */
          AdditionalData (const double pivoting = 1.e-6,
			  const double zero_pivot = 1.e-12,
			  const double damping = 0.0);

                                           /**
                                            * Determines, when Pivoting is
					    * done during LU decomposition.
					    * 0.0 indicates no pivoting,
					    * and 1.0 complete pivoting.
					    * Confer PetSC manual for more
					    * details.
                                            */
          double pivoting;

                                           /**
                                            * Size at which smaller pivots
					    * are declared to be zero.
					    * Confer PetSC manual for more
					    * details.
					    */
	  double zero_pivot;

                                           /**
                                            * This quantity is added to the
					    * diagonal of the matrix during
					    * factorisation.
					    */
	  double damping;
      };

				       /**
					* Empty Constructor. You need to call
					* initialize() before using this
					* object.
					*/
      PreconditionLU ();

                                       /**
                                        * Constructor. Take the matrix which
                                        * is used to form the preconditioner,
                                        * and additional flags if there are
                                        * any.
                                        */
      PreconditionLU (const MatrixBase     &matrix,
		      const AdditionalData &additional_data = AdditionalData());

				       /**
					* Initializes the preconditioner
					* object and calculate all data that
					* is necessary for applying it in a
					* solver. This function is
					* automatically called when calling
					* the constructor with the same
					* arguments and is only used if you
					* create the preconditioner without
					* arguments.
					*/
      void initialize (const MatrixBase     &matrix,
                       const AdditionalData &additional_data = AdditionalData());

    protected:
                                       /**
                                        * Store a copy of the flags for this
                                        * particular preconditioner.
                                        */
      AdditionalData additional_data;
  };




/**
 * A class that implements the interface to use the BoomerAMG algebraic
 * multigrid preconditioner from the HYPRE suite. Note that PETSc has to be
 * configured with HYPRE (e.g. with --download-hypre=1).
 *
 * The preconditioner does support parallel distributed computations. See
 * step-40 for an example.
 *
 * @ingroup PETScWrappers
 * @author Timo Heister, 2010
 */
  class PreconditionBoomerAMG : public PreconditionerBase
  {
    public:
                                       /**
                                        * Standardized data struct to
                                        * pipe additional flags to the
                                        * preconditioner.
                                        */
      struct AdditionalData
      {
                                           /**
                                            * Constructor. Note that BoomerAMG
                                            * offers a lot more options to set
                                            * than what is exposed here.
                                            */
          AdditionalData (
	    const bool symmetric_operator = false,
	    const double strong_threshold = 0.25,
	    const double max_row_sum = 0.9,
	    const unsigned int aggressive_coarsening_num_levels = 0,
	    const bool output_details = false
	  );

					   /**
					    * Set this flag to true if you
					    * have a symmetric system matrix
					    * and you want to use a solver
					    * which asumes a symmetric
					    * preconditioner like CG. The
					    * relaxation is done with
					    * SSOR/Jacobi when set to true and
					    * with SOR/Jacobi otherwise.
					    */
	  bool symmetric_operator;

					   /**
					    * Threshold of when nodes are
					    * considered strongly
					    * connected. See
					    * HYPRE_BoomerAMGSetStrongThreshold(). Recommended
					    * values are 0.25 for 2d and 0.5
					    * for 3d problems, but it is
					    * problem dependent.
					    */
	  double strong_threshold;

					   /**
					    * If set to a value smaller than
					    * 1.0 then diagonally dominant
					    * parts of the matrix are treated
					    * as having no strongly connected
					    * nodes. If the row sum weighted
					    * by the diagonal entry is bigger
					    * than the given value, it is
					    * considered diagonally
					    * dominant. This feature is turned
					    * of by setting the value to
					    * 1.0. This is the default as some
					    * matrices can result in having
					    * only diagonally dominant entries
					    * and thus no multigrid levels are
					    * constructed. The default in
					    * BoomerAMG for this is 0.9. When
					    * you try this, check for a
					    * reasonable number of levels
					    * created.
					    */
	  double max_row_sum;

					     /**
					    * Number of levels of aggressive
					    * coarsening. Increasing this
					    * value reduces the construction
					    * time and memory requirements but
					    * may decrease effectiveness.*/
	  unsigned int aggressive_coarsening_num_levels;

       					   /**
					    * Setting this flag to true
					    * produces debug output from
					    * HYPRE, when the preconditioner
					    * is constructed.
					    */
	  bool output_details;
      };



				       /**
					* Empty Constructor. You need to call
					* initialize() before using this
					* object.
					*/
      PreconditionBoomerAMG ();

                                       /**
                                        * Constructor. Take the matrix which
                                        * is used to form the preconditioner,
                                        * and additional flags if there are
                                        * any.
                                        */
      PreconditionBoomerAMG (const MatrixBase     &matrix,
			     const AdditionalData &additional_data = AdditionalData());

				       /**
					* Initializes the preconditioner
					* object and calculate all data that
					* is necessary for applying it in a
					* solver. This function is
					* automatically called when calling
					* the constructor with the same
					* arguments and is only used if you
					* create the preconditioner without
					* arguments.
					*/
      void initialize (const MatrixBase     &matrix,
                       const AdditionalData &additional_data = AdditionalData());

    protected:
                                       /**
                                        * Store a copy of the flags for this
                                        * particular preconditioner.
                                        */
      AdditionalData additional_data;
  };
}



DEAL_II_NAMESPACE_CLOSE


#endif // DEAL_II_USE_PETSC

/*----------------------------   petsc_precondition.h     ---------------------------*/

#endif
/*----------------------------   petsc_precondition.h     ---------------------------*/