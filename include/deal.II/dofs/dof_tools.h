//---------------------------------------------------------------------------
//    $Id: dof_tools.h 24520 2011-10-04 02:23:51Z kanschat $
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__dof_tools_h
#define __deal2__dof_tools_h


#include <deal.II/base/config.h>
#include <deal.II/base/exceptions.h>
#include <deal.II/base/table.h>
#include <deal.II/base/index_set.h>
#include <deal.II/base/point.h>
#include <deal.II/lac/constraint_matrix.h>
#include <deal.II/lac/sparsity_pattern.h>
#include <deal.II/dofs/function_map.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/fe/fe.h>

#include <vector>
#include <set>
#include <map>

DEAL_II_NAMESPACE_OPEN

template<int dim, class T> class Table;
class SparsityPattern;
template <typename number> class Vector;
template <int dim> class Function;
template <int dim, int spacedim> class FiniteElement;
template <int dim, int spacedim> class DoFHandler;
namespace hp
{
  template <int dim, int spacedim> class DoFHandler;
}
template <int dim, int spacedim> class MGDoFHandler;
class ConstraintMatrix;
template <class GridClass> class InterGridMap;
template <int dim, int spacedim> class Mapping;


//TODO: map_support_points_to_dofs should generate a multimap, rather than just a map, since several dofs may be located at the same support point

/**
 * This is a collection of functions operating on, and manipulating
 * the numbers of degrees of freedom. The documentation of the member
 * functions will provide more information, but for functions that
 * exist in multiple versions, there are sections in this global
 * documentation stating some commonalities.
 *
 * All member functions are static, so there is no need to create an
 * object of class DoFTools.
 *
 *
 * <h3>Setting up sparsity patterns</h3>
 *
 * When assembling system matrices, the entries are usually of the form
 * $a_{ij} = a(\phi_i, \phi_j)$, where $a$ is a bilinear functional, often an
 * integral. When using sparse matrices, we therefore only need to reserve space
 * for those $a_{ij}$ only, which are nonzero, which is the same as to say that
 * the basis functions $\phi_i$ and $\phi_j$ have a nonempty intersection of
 * their support. Since the support of basis functions is bound only on cells
 * on which they are located or to which they are adjacent, to
 * determine the sparsity pattern it is sufficient to loop over all
 * cells and connect all basis functions on each cell with all other
 * basis functions on that cell.  There may be finite elements for
 * which not all basis functions on a cell connect with each other,
 * but no use of this case is made since no examples where this occurs
 * are known to the author.
 *
 *
 * <h3>DoF numberings on boundaries</h3>
 *
 * When projecting the traces of functions to the boundary or parts thereof,
 * one needs to build matrices and vectors that act only on those degrees of
 * freedom that are located on the boundary, rather than on all degrees of
 * freedom. One could do that by simply building matrices in which the entries
 * for all interior DoFs are zero, but such matrices are always very rank
 * deficient and not very practical to work with.
 *
 * What is needed instead in this case is a numbering of the boundary degrees
 * of freedom, i.e. we should enumerate all the degrees of freedom that are
 * sitting on the boundary, and exclude all other (interior) degrees of
 * freedom. The map_dof_to_boundary_indices() function does exactly this: it
 * provides a vector with as many entries as there are degrees of freedom on
 * the whole domain, with each entry being the number in the numbering of the
 * boundary or DoFHandler::invalid_dof_index if the dof is not on the
 * boundary.
 *
 * With this vector, one can get, for any given degree of freedom, a unique
 * number among those DoFs that sit on the boundary; or, if your DoF was
 * interior to the domain, the result would be DoFHandler::invalid_dof_index.
 * We need this mapping, for example, to build the mass matrix on the boundary
 * (for this, see make_boundary_sparsity_pattern() function, the corresponding
 * section below, as well as the MatrixCreator class documentation).
 *
 * Actually, there are two map_dof_to_boundary_indices() functions, one
 * producing a numbering for all boundary degrees of freedom and one producing
 * a numbering for only parts of the boundary, namely those parts for which
 * the boundary indicator is listed in a set of indicators given to the
 * function. The latter case is needed if, for example, we would only want to
 * project the boundary values for the Dirichlet part of the boundary. You
 * then give the function a list of boundary indicators referring to Dirichlet
 * parts on which the projection is to be performed. The parts of the boundary
 * on which you want to project need not be contiguous; however, it is not
 * guaranteed that the indices of each of the boundary parts are continuous,
 * i.e. the indices of degrees of freedom on different parts may be
 * intermixed.
 *
 * Degrees of freedom on the boundary but not on one of the specified
 * boundary parts are given the index DoFHandler::invalid_dof_index, as if
 * they were in the interior. If no boundary indicator was given or if
 * no face of a cell has a boundary indicator contained in the given
 * list, the vector of new indices consists solely of
 * DoFHandler::invalid_dof_index.
 *
 * (As a side note, for corner cases: The question what a degree of freedom on
 * the boundary is, is not so easy.  It should really be a degree of freedom
 * of which the respective basis function has nonzero values on the
 * boundary. At least for Lagrange elements this definition is equal to the
 * statement that the off-point, or what deal.II calls support_point, of the
 * shape function, i.e. the point where the function assumes its nominal value
 * (for Lagrange elements this is the point where it has the function value
 * 1), is located on the boundary. We do not check this directly, the
 * criterion is rather defined through the information the finite element
 * class gives: the FiniteElement class defines the numbers of basis functions
 * per vertex, per line, and so on and the basis functions are numbered after
 * this information; a basis function is to be considered to be on the face of
 * a cell (and thus on the boundary if the cell is at the boundary) according
 * to it belonging to a vertex, line, etc but not to the interior of the
 * cell. The finite element uses the same cell-wise numbering so that we can
 * say that if a degree of freedom was numbered as one of the dofs on lines,
 * we assume that it is located on the line. Where the off-point actually is,
 * is a secret of the finite element (well, you can ask it, but we don't do it
 * here) and not relevant in this context.)
 *
 *
 * <h3>Setting up sparsity patterns for boundary matrices</h3>
 *
 * In some cases, one wants to only work with DoFs that sit on the
 * boundary. One application is, for example, if rather than interpolating
 * non-homogenous boundary values, one would like to project them. For this,
 * we need two things: a way to identify nodes that are located on (parts of)
 * the boundary, and a way to build matrices out of only degrees of freedom
 * that are on the boundary (i.e. much smaller matrices, in which we do not
 * even build the large zero block that stems from the fact that most degrees
 * of freedom have no support on the boundary of the domain). The first of
 * these tasks is done by the map_dof_to_boundary_indices() function of this
 * class (described above).
 *
 * The second part requires us first to build a sparsity pattern for the
 * couplings between boundary nodes, and then to actually build the components
 * of this matrix. While actually computing the entries of these small
 * boundary matrices is discussed in the MatrixCreator class, the creation of
 * the sparsity pattern is done by the create_boundary_sparsity_pattern()
 * function. For its work, it needs to have a numbering of all those degrees
 * of freedom that are on those parts of the boundary that we are interested
 * in. You can get this from the map_dof_to_boundary_indices() function. It
 * then builds the sparsity pattern corresponding to integrals like
 * $\int_\Gamma \varphi_{b2d(i)} \varphi_{b2d(j)} dx$, where $i$ and $j$ are
 * indices into the matrix, and $b2d(i)$ is the global DoF number of a degree
 * of freedom sitting on a boundary (i.e., $b2d$ is the inverse of the mapping
 * returned by map_dof_to_boundary_indices() function).
 *
 *
 * @ingroup dofs
 * @author Wolfgang Bangerth, Guido Kanschat and others
 */
namespace DoFTools
{
				   /**
				    * The flags used in tables by certain
				    * <tt>make_*_pattern</tt> functions to
				    * describe whether two components of the
				    * solution couple in the bilinear forms
				    * corresponding to cell or face
				    * terms. An example of using these flags
				    * is shown in the introduction of
				    * step-46.
				    *
				    * In the descriptions of the individual
				    * elements below, remember that these
				    * flags are used as elements of tables
				    * of size FiniteElement::n_components
				    * times FiniteElement::n_components
				    * where each element indicates whether
				    * two components do or do not couple.
				    */
  enum Coupling
  {
					 /**
					  * Two components do not
					  * couple.
					  */
	none,
					 /**
					  * Two components do couple.
					  */
	always,
					 /**
					  * Two components couple only
					  * if their shape functions are
					  * both nonzero on a given
					  * face. This flag is only used
					  * when computing integrals over
					  * faces of cells.
					  */
	nonzero
  };

				   /**
				    * @name Auxiliary functions
				    * @{
				    */
				   /**
				    * Maximal number of degrees of
				    * freedom on a cell.
				    *
				    * @relates DoFHandler
				    */
  template <int dim, int spacedim>
  unsigned int
  max_dofs_per_cell (const DoFHandler<dim,spacedim> &dh);

				   /**
				    * Maximal number of degrees of
				    * freedom on a cell.
				    *
				    * @relates hp::DoFHandler
				    */
  template <int dim, int spacedim>
  unsigned int
  max_dofs_per_cell (const hp::DoFHandler<dim,spacedim> &dh);


				   /**
				    * Maximal number of degrees of
				    * freedom on a face.
				    *
				    * This function exists for both non-hp
				    * and hp DoFHandlers, to allow for a
				    * uniform interface to query this
				    * property.
				    *
				    * @relates DoFHandler
				    */
  template <int dim, int spacedim>
  unsigned int
  max_dofs_per_face (const DoFHandler<dim,spacedim> &dh);

				   /**
				    * Maximal number of degrees of
				    * freedom on a face.
				    *
				    * This function exists for both non-hp
				    * and hp DoFHandlers, to allow for a
				    * uniform interface to query this
				    * property.
				    *
				    * @relates hp::DoFHandler
				    */
  template <int dim, int spacedim>
  unsigned int
  max_dofs_per_face (const hp::DoFHandler<dim,spacedim> &dh);

				   /**
				    * Maximal number of degrees of
				    * freedom on a vertex.
				    *
				    * This function exists for both non-hp
				    * and hp DoFHandlers, to allow for a
				    * uniform interface to query this
				    * property.
				    *
				    * @relates DoFHandler
				    */
  template <int dim, int spacedim>
  unsigned int
  max_dofs_per_vertex (const DoFHandler<dim,spacedim> &dh);

				   /**
				    * Maximal number of degrees of
				    * freedom on a vertex.
				    *
				    * This function exists for both non-hp
				    * and hp DoFHandlers, to allow for a
				    * uniform interface to query this
				    * property.
				    *
				    * @relates hp::DoFHandler
				    */
  template <int dim, int spacedim>
  unsigned int
  max_dofs_per_vertex (const hp::DoFHandler<dim,spacedim> &dh);

				   /**
				    * Number of vector components in the
				    * finite element object used by this
				    * DoFHandler.
				    *
				    * This function exists for both non-hp
				    * and hp DoFHandlers, to allow for a
				    * uniform interface to query this
				    * property.
				    *
				    * @relates DoFHandler
				    */
  template <int dim, int spacedim>
  unsigned int
  n_components (const DoFHandler<dim,spacedim> &dh);

				   /**
				    * Number of vector components in the
				    * finite element object used by this
				    * DoFHandler.
				    *
				    * This function exists for both non-hp
				    * and hp DoFHandlers, to allow for a
				    * uniform interface to query this
				    * property.
				    *
				    * @relates hp::DoFHandler
				    */
  template <int dim, int spacedim>
  unsigned int
  n_components (const hp::DoFHandler<dim,spacedim> &dh);

				   /**
				    * Find out whether the FiniteElement
				    * used by this DoFHandler is primitive
				    * or not.
				    *
				    * This function exists for both non-hp
				    * and hp DoFHandlers, to allow for a
				    * uniform interface to query this
				    * property.
				    *
				    * @relates DoFHandler
				    */
  template <int dim, int spacedim>
  bool
  fe_is_primitive (const DoFHandler<dim,spacedim> &dh);

				   /**
				    * Find out whether the FiniteElement
				    * used by this DoFHandler is primitive
				    * or not.
				    *
				    * This function exists for both non-hp
				    * and hp DoFHandlers, to allow for a
				    * uniform interface to query this
				    * property.
				    *
				    * @relates hp::DoFHandler
				    */
  template <int dim, int spacedim>
  bool
  fe_is_primitive (const hp::DoFHandler<dim,spacedim> &dh);

				   /**
				    * @}
				    */

				   /**
				    * @name Sparsity Pattern Generation
				    * @{
				    */

				   /**
				    * Locate non-zero entries of the
				    * system matrix.
				    *
				    * This function computes the
				    * possible positions of non-zero
				    * entries in the global system
				    * matrix. We assume that a
				    * certain finite element basis
				    * function is non-zero on a cell
				    * only if its degree of freedom
				    * is associated with the
				    * interior, a face, an edge or a
				    * vertex of this cell. As a
				    * result, the matrix entry
				    * between two basis functions
				    * can be non-zero only if they
				    * correspond to degrees of
				    * freedom of at least one common
				    * cell. Therefore,
				    * @p make_sparsity_pattern just
				    * loops over all cells and
				    * enters all couplings local to
				    * that cell. As the generation
				    * of the sparsity pattern is
				    * irrespective of the equation
				    * which is solved later on, the
				    * resulting sparsity pattern is
				    * symmetric.
				    *
				    * Remember using
				    * SparsityPattern::compress()
				    * after generating the pattern.
				    *
				    * The actual type of the
				    * sparsity pattern may be
				    * SparsityPattern,
				    * CompressedSparsityPattern,
				    * BlockSparsityPattern,
				    * BlockCompressedSparsityPattern,
				    * BlockCompressedSetSparsityPattern,
				    * BlockCompressedSimpleSparsityPattern,
				    * or any other class that
				    * satisfies similar
				    * requirements. It is assumed
				    * that the size of the sparsity
				    * pattern matches the number of
				    * degrees of freedom and that
				    * enough unused nonzero entries
				    * are left to fill the sparsity
				    * pattern. The nonzero entries
				    * generated by this function are
				    * overlaid to possible previous
				    * content of the object, that is
				    * previously added entries are
				    * not deleted.
				    *
				    * Since this process is purely local,
				    * the sparsity pattern does not provide
				    * for entries introduced by the
				    * elimination of hanging nodes. They
				    * have to be taken care of by a call to
				    * ConstraintMatrix::condense()
				    * afterwards.
				    *
				    * Alternatively, the constraints on
				    * degrees of freedom can already be
				    * taken into account at the time of
				    * creating the sparsity pattern. For
				    * this, pass the ConstraintMatrix object
				    * as the third argument to the current
				    * function. No call to
				    * ConstraintMatrix::condense() is then
				    * necessary. This process is explained
				    * in step-27.
				    *
				    * In case the constraints are
				    * already taken care of in this
				    * function, it is possible to
				    * neglect off-diagonal entries
				    * in the sparsity pattern. When
				    * using
				    * ConstraintMatrix::distribute_local_to_global
				    * during assembling, no entries
				    * will ever be written into
				    * these matrix position, so that
				    * one can save some computing
				    * time in matrix-vector products
				    * by not even creating these
				    * elements. In that case, the
				    * variable
				    * <tt>keep_constrained_dofs</tt>
				    * needs to be set to
				    * <tt>false</tt>.
				    *
				    * If the @p subdomain_id parameter is
				    * given, the sparsity pattern is built
				    * only on cells that have a subdomain_id
				    * equal to the given argument. This is
				    * useful in parallel contexts where the
				    * matrix and sparsity pattern (for
				    * example a
				    * TrilinosWrappers::SparsityPattern) may
				    * be distributed and not every MPI
				    * process needs to build the entire
				    * sparsity pattern; in that case, it is
				    * sufficient if every process only
				    * builds that part of the sparsity
				    * pattern that corresponds to the
				    * subdomain_id for which it is
				    * responsible. This feature is
				    * used in step-32.
				    *
				    * @ingroup constraints
				    */
  template <class DH, class SparsityPattern>
  void
  make_sparsity_pattern (const DH               &dof,
			 SparsityPattern        &sparsity_pattern,
			 const ConstraintMatrix &constraints = ConstraintMatrix(),
			 const bool              keep_constrained_dofs = true,
			 const types::subdomain_id_t subdomain_id = types::invalid_subdomain_id);

				   /**
				    * Locate non-zero entries for
				    * vector valued finite elements.
				    * This function does mostly the
				    * same as the previous
				    * @p make_sparsity_pattern, but
				    * it is specialized for vector
				    * finite elements and allows to
				    * specify which variables couple
				    * in which equation. For
				    * example, if wanted to solve
				    * the Stokes equations,
				    * @f{align*}
				    * -\Delta \mathbf u + \nabla p &= 0,\\
				    * \text{div}\ u                    &= 0
				    * @f}
				    * in two space dimensions,
				    * using stable Q2/Q1 mixed
				    * elements (using the FESystem
				    * class), then you don't want
				    * all degrees of freedom to
				    * couple in each equation. You
				    * rather may want to give the
				    * following pattern of
				    * couplings:
				    * @f[
				    * \begin{array}{ccc}
				    *   1 & 0 & 1 \\
				    *   0 & 1 & 1 \\
				    *   1 & 1 & 0
				    * \end{array}
				    * @f]
				    * where "1" indicates that two
				    * variables (i.e. components of
				    * the FESystem) couple in the
				    * respective equation, and a "0"
				    * means no coupling, in which
				    * case it is not necessary to
				    * allocate space in the matrix
				    * structure. Obviously, the mask
				    * refers to components of the
				    * composed FESystem, rather
				    * than to the degrees of freedom
				    * contained in there.
				    *
				    * This function is designed to
				    * accept a coupling pattern, like the one
				    * shown above, through the
				    * @p couplings parameter, which
				    * contains values of type #Coupling. It
				    * builds the matrix structure
				    * just like the previous
				    * function, but does not create
				    * matrix elements if not
				    * specified by the coupling pattern. If the
				    * couplings are symmetric, then so
				    * will be the resulting sparsity
				    * pattern.
				    *
				    * The actual type of the
				    * sparsity pattern may be
				    * SparsityPattern,
				    * CompressedSparsityPattern,
				    * BlockSparsityPattern,
				    * BlockCompressedSparsityPattern,
				    * BlockCompressedSetSparsityPattern,
				    * or any other class that
				    * satisfies similar
				    * requirements.
				    *
				    * There is a complication if
				    * some or all of the shape
				    * functions of the finite
				    * element in use are non-zero in
				    * more than one component (in
				    * deal.II speak: they are
				    * non-primitive). In this case,
				    * the coupling element
				    * correspoding to the first
				    * non-zero component is taken
				    * and additional ones for this
				    * component are ignored.
				    *
				    * Not implemented for
				    * hp::DoFHandler.
				    *
				    * As mentioned before, the
				    * creation of the sparsity
				    * pattern is a purely local
				    * process and the sparsity
				    * pattern does not provide for
				    * entries introduced by the
				    * elimination of hanging
				    * nodes. They have to be taken
				    * care of by a call to
				    * ConstraintMatrix::condense()
				    * afterwards.
				    *
				    * Alternatively, the constraints
				    * on degrees of freedom can
				    * already be taken into account
				    * at the time of creating the
				    * sparsity pattern. For this,
				    * pass the ConstraintMatrix
				    * object as the third argument
				    * to the current function. No
				    * call to
				    * ConstraintMatrix::condense()
				    * is then necessary. This
				    * process is explained in @ref
				    * step_27 "step-27".
				    *
				    * In case the constraints are
				    * already taken care of in this
				    * function, it is possible to
				    * neglect off-diagonal entries
				    * in the sparsity pattern. When
				    * using
				    * ConstraintMatrix::distribute_local_to_global
				    * during assembling, no entries
				    * will ever be written into
				    * these matrix position, so that
				    * one can save some computing
				    * time in matrix-vector products
				    * by not even creating these
				    * elements. In that case, the
				    * variable
				    * <tt>keep_constrained_dofs</tt>
				    * needs to be set to
				    * <tt>false</tt>.
				    *
				    * If the @p subdomain_id parameter is
				    * given, the sparsity pattern is built
				    * only on cells that have a subdomain_id
				    * equal to the given argument. This is
				    * useful in parallel contexts where the
				    * matrix and sparsity pattern (for
				    * example a
				    * TrilinosWrappers::SparsityPattern) may
				    * be distributed and not every MPI
				    * process needs to build the entire
				    * sparsity pattern; in that case, it is
				    * sufficient if every process only
				    * builds that part of the sparsity
				    * pattern that corresponds to the
				    * subdomain_id for which it is
				    * responsible. This feature is
				    * used in step-32.
				    *
				    * @ingroup constraints
				    */
  template <class DH, class SparsityPattern>
  void
  make_sparsity_pattern (const DH                 &dof,
			 const Table<2, Coupling> &coupling,
			 SparsityPattern          &sparsity_pattern,
			 const ConstraintMatrix   &constraints = ConstraintMatrix(),
			 const bool                keep_constrained_dofs = true,
			 const types::subdomain_id_t subdomain_id = types::invalid_subdomain_id);

				   /**
				    * @deprecated This is the old
				    * form of the previous
				    * function. It generates a table
				    * of DoFTools::Coupling values
				    * (where a <code>true</code>
				    * value in the mask is
				    * translated into a
				    * Coupling::always value in the
				    * table) and calls the function
				    * above.
				    */
  template <class DH, class SparsityPattern>
  void
  make_sparsity_pattern (const DH                              &dof,
			 const std::vector<std::vector<bool> > &mask,
			 SparsityPattern                       &sparsity_pattern);

				   /**
				    * Construct a sparsity pattern that
				    * allows coupling degrees of freedom on
				    * two different but related meshes.
				    *
				    * The idea is that if the two given
				    * DoFHandler objects correspond to two
				    * different meshes (and potentially to
				    * different finite elements used on
				    * these cells), but that if the two
				    * triangulations they are based on are
				    * derived from the same coarse mesh
				    * through hierarchical refinement, then
				    * one may set up a problem where one
				    * would like to test shape functions
				    * from one mesh against the shape
				    * functions from another mesh. In
				    * particular, this means that shape
				    * functions from a cell on the first
				    * mesh are tested against those on the
				    * second cell that are located on the
				    * corresponding cell; this
				    * correspondence is something that the
				    * IntergridMap class can determine.
				    *
				    * This function then constructs a
				    * sparsity pattern for which the degrees
				    * of freedom that represent the rows
				    * come from the first given DoFHandler,
				    * whereas the ones that correspond to
				    * columns come from the second
				    * DoFHandler.
				    */
  template <class DH, class SparsityPattern>
  void
  make_sparsity_pattern (const DH        &dof_row,
			 const DH        &dof_col,
			 SparsityPattern &sparsity);

				   /**
				    * Create the sparsity pattern for
				    * boundary matrices. See the general
				    * documentation of this class for more
				    * information.
				    *
				    * The actual type of the sparsity
				    * pattern may be SparsityPattern,
				    * CompressedSparsityPattern,
				    * BlockSparsityPattern,
				    * BlockCompressedSparsityPattern,
				    * BlockCompressedSetSparsityPattern, or
				    * any other class that satisfies similar
				    * requirements. It is assumed that the
				    * size of the sparsity pattern is
				    * already correct.
				    */
  template <class DH, class SparsityPattern>
  void
  make_boundary_sparsity_pattern (const DH                        &dof,
				  const std::vector<unsigned int> &dof_to_boundary_mapping,
				  SparsityPattern                 &sparsity_pattern);

				   /**
				    * Write the sparsity structure of the
				    * matrix composed of the basis functions
				    * on the boundary into the
				    * matrix structure. In contrast to the
				    * previous function, only those parts
				    * of the boundary are considered of which
				    * the boundary indicator is listed in the
				    * set of numbers passed to this function.
				    *
				    * In fact, rather than a @p set
				    * of boundary indicators, a
				    * @p map needs to be passed,
				    * since most of the functions
				    * handling with boundary
				    * indicators take a mapping of
				    * boundary indicators and the
				    * respective boundary
				    * functions. The boundary
				    * function, however, is ignored
				    * in this function.  If you have
				    * no functions at hand, but only
				    * the boundary indicators, set
				    * the function pointers to null
				    * pointers.
				    *
				    * For the type of the sparsity
				    * pattern, the same holds as
				    * said above.
				    */
  template <class DH, class SparsityPattern>
  void
  make_boundary_sparsity_pattern (const DH &dof,
				  const typename FunctionMap<DH::space_dimension>::type &boundary_indicators,
				  const std::vector<unsigned int> &dof_to_boundary_mapping,
				  SparsityPattern    &sparsity);

				   /**
				    * Generate sparsity pattern for
				    * fluxes, i.e. formulations of
				    * the discrete problem with
				    * discontinuous elements which
				    * couple across faces of cells.
				    * This is a replacement of the
				    * function
				    * @p make_sparsity_pattern for
				    * discontinuous methods. Since
				    * the fluxes include couplings
				    * between neighboring elements,
				    * the normal couplings and these
				    * extra matrix entries are
				    * considered.
				    */
  template<class DH, class SparsityPattern>
  void
  make_flux_sparsity_pattern (const DH        &dof_handler,
			      SparsityPattern &sparsity_pattern);

				   /**
				    * This function does the same as
				    * the other with the same name,
				    * but it gets a ConstraintMatrix
				    * additionally.
				    * This is for the case where you
				    * have fluxes but constraints as
				    * well.
				    *
				    * @ingroup constraints
				    */
  template<class DH, class SparsityPattern>
  void
  make_flux_sparsity_pattern (const DH        &dof_handler,
			      SparsityPattern &sparsity_pattern,
			      const ConstraintMatrix   &constraints,
			      const bool                keep_constrained_dofs = true,
			      const types::subdomain_id_t  subdomain_id = numbers::invalid_unsigned_int);

				   /**
				    * This function does the same as
				    * the other with the same name,
				    * but it gets two additional
				    * coefficient matrices. A matrix
				    * entry will only be generated
				    * for two basis functions, if
				    * there is a non-zero entry
				    * linking their associated
				    * components in the coefficient
				    * matrix.
				    *
				    * There is one matrix for
				    * couplings in a cell and one
				    * for the couplings occuring in
				    * fluxes.
				    *
				    * Not implemented for
				    * hp::DoFHandler.
				    */
  template <class DH, class SparsityPattern>
  void
  make_flux_sparsity_pattern (const DH                &dof,
			      SparsityPattern         &sparsity,
			      const Table<2,Coupling> &int_mask,
			      const Table<2,Coupling> &flux_mask);

				   //@}
				   /**
				    * @name Hanging Nodes
				    * @{
				    */

				   /**
				    * Compute the constraints resulting from
				    * the presence of hanging nodes. Hanging
				    * nodes are best explained using a small
				    * picture:
				    *
				    * @image html hanging_nodes.png
				    *
				    * In order to make a finite element
				    * function globally continuous, we have
				    * to make sure that the dark red nodes
				    * have values that are compatible with
				    * the adjacent yellow nodes, so that the
				    * function has no jump when coming from
				    * the small cells to the large one at
				    * the top right. We therefore have to
				    * add conditions that constrain those
				    * "hanging nodes".
				    *
				    * The object into
				    * which these are inserted is
				    * later used to condense the
				    * global system matrix and right
				    * hand side, and to extend the
				    * solution vectors from the true
				    * degrees of freedom also to the
				    * constraint nodes. This
				    * function is explained in
				    * detail in the @ref step_6
				    * "step-6" tutorial program and
				    * is used in almost all
				    * following programs as well.
				    *
				    * This function does not clear
				    * the constraint matrix object
				    * before use, in order to allow
				    * adding constraints from
				    * different sources to the same
				    * object. You therefore need to
				    * make sure it contains only
				    * constraints you still want;
				    * otherwise call the
				    * ConstraintMatrix::clear()
				    * function.  Likewise, this
				    * function does not close the
				    * object since you may want to
				    * enter other constraints later
				    * on yourself.
				    *
				    * In the hp-case, i.e. when the
				    * argument is of type
				    * hp::DoFHandler, we consider
				    * constraints due to different
				    * finite elements used on two
				    * sides of a face between cells
				    * as hanging nodes as well. In
				    * other words, for hp finite
				    * elements, this function
				    * computes all constraints due
				    * to differing mesh sizes (h) or
				    * polynomial degrees (p) between
				    * adjacent cells.
				    *
				    * The template argument (and by
				    * consequence the type of the
				    * first argument to this
				    * function) can be either a
				    * ::DoFHandler, hp::DoFHandler,
				    * or MGDoFHandler.
				    *
				    * @ingroup constraints
				    */
  template <class DH>
  void
  make_hanging_node_constraints (const DH         &dof_handler,
				 ConstraintMatrix &constraints);
				   //@}

				   /**
				    * Take a vector of values which live on
				    * cells (e.g. an error per cell) and
				    * distribute it to the dofs in such a
				    * way that a finite element field
				    * results, which can then be further
				    * processed, e.g. for output. You should
				    * note that the resulting field will not
				    * be continuous at hanging nodes. This
				    * can, however, easily be arranged by
				    * calling the appropriate @p distribute
				    * function of a ConstraintMatrix
				    * object created for this
				    * DoFHandler object, after the
				    * vector has been fully assembled.
				    *
				    * It is assumed that the number
				    * of elements in @p cell_data
				    * equals the number of active
				    * cells and that the number of
				    * elements in @p dof_data equals
				    * <tt>dof_handler.n_dofs()</tt>.
				    *
				    * Note that the input vector may
				    * be a vector of any data type
				    * as long as it is convertible
				    * to @p double.  The output
				    * vector, being a data vector on
				    * a DoF handler, always consists of
				    * elements of type @p double.
				    *
				    * In case the finite element
				    * used by this DoFHandler
				    * consists of more than one
				    * component, you need to specify
				    * which component in the output
				    * vector should be used to store
				    * the finite element field in;
				    * the default is zero (no other
				    * value is allowed if the finite
				    * element consists only of one
				    * component). All other
				    * components of the vector
				    * remain untouched, i.e. their
				    * contents are not changed.
				    *
				    * This function cannot be used
				    * if the finite element in use
				    * has shape functions that are
				    * non-zero in more than one
				    * vector component (in deal.II
				    * speak: they are
				    * non-primitive).
				    */
  template <class DH, typename Number>
  void
  distribute_cell_to_dof_vector (const DH              &dof_handler,
				 const Vector<Number>  &cell_data,
				 Vector<double>        &dof_data,
				 const unsigned int     component = 0);

				   /**
				    * Extract the indices of the
				    * degrees of freedom belonging
				    * to certain vector components
				    * or blocks (if the last
				    * argument is <tt>true</tt>) of
				    * a vector-valued finite
				    * element. The bit vector @p
				    * select defines, which
				    * components or blocks of an
				    * FESystem are to be extracted
				    * from the DoFHandler @p
				    * dof. The entries in @p
				    * selected_dofs corresponding to
				    * degrees of freedom belonging
				    * to these components are then
				    * flagged @p true, while all
				    * others are set to @p false.
				    *
				    * The size of @p select must
				    * equal the number of components
				    * or blocks in the FiniteElement
				    * used by @p dof, depending on
				    * the argument
				    * <tt>blocks</tt>. The size of
				    * @p selected_dofs must equal
				    * DoFHandler::n_dofs(). Previous
				    * contents of this array are
				    * overwritten.
				    *
				    * If the finite element under
				    * consideration is not
				    * primitive, that is some or all
				    * of its shape functions are
				    * non-zero in more than one
				    * vector component (which holds,
				    * for example, for FE_Nedelec or
				    * FE_RaviartThomas elements), then
				    * shape functions cannot be
				    * associated with a single
				    * vector component. In this
				    * case, if <em>one</em> shape
				    * vector component of this
				    * element is flagged in
				    * @p component_select, then
				    * this is equivalent to
				    * selecting <em>all</em> vector
				    * components corresponding to
				    * this non-primitive base
				    * element.
				    */
  template <int dim, int spacedim>
  void
  extract_dofs (const DoFHandler<dim,spacedim>   &dof_handler,
		const std::vector<bool> &select,
		std::vector<bool>       &selected_dofs,
		const bool               blocks = false);

				   /**
				    * The same function as above,
				    * but for a hp::DoFHandler.
				    */
  template <int dim, int spacedim>
  void
  extract_dofs (const hp::DoFHandler<dim,spacedim>   &dof_handler,
		const std::vector<bool> &select,
		std::vector<bool>       &selected_dofs,
		const bool               blocks = false);

				   /**
				    * Do the same thing as
				    * extract_dofs() for one level
				    * of a multi-grid DoF numbering.
				    */
  template <int dim, int spacedim>
  void
  extract_level_dofs (const unsigned int       level,
		      const MGDoFHandler<dim,spacedim> &dof,
		      const std::vector<bool> &select,
		      std::vector<bool>       &selected_dofs,
		      const bool               blocks = false);

				   /**
				    * Extract all degrees of freedom
				    * which are at the boundary and
				    * belong to specified components
				    * of the solution. The function
				    * returns its results in the
				    * last non-default-valued
				    * parameter which contains
				    * @p true if a degree of
				    * freedom is at the boundary and
				    * belongs to one of the selected
				    * components, and @p false
				    * otherwise.
				    *
				    * By specifying the
				    * @p boundary_indicator
				    * variable, you can select which
				    * boundary indicators the faces
				    * have to have on which the
				    * degrees of freedom are located
				    * that shall be extracted. If it
				    * is an empty list, then all
				    * boundary indicators are
				    * accepted.
				    *
				    * The size of @p component_select
				    * shall equal the number of
				    * components in the finite
				    * element used by @p dof. The
				    * size of @p selected_dofs shall
				    * equal
				    * <tt>dof_handler.n_dofs()</tt>. Previous
				    * contents of this array or
				    * overwritten.
				    *
				    * Using the usual convention, if
				    * a shape function is non-zero
				    * in more than one component
				    * (i.e. it is non-primitive),
				    * then the element in the
				    * component mask is used that
				    * corresponds to the first
				    * non-zero components. Elements
				    * in the mask corresponding to
				    * later components are ignored.
				    */
  template <class DH>
  void
  extract_boundary_dofs (const DH                   &dof_handler,
			 const std::vector<bool>    &component_select,
			 std::vector<bool>          &selected_dofs,
			 const std::set<unsigned char> &boundary_indicators = std::set<unsigned char>());

				   /**
				    * This function is similar to
				    * the extract_boundary_dofs()
				    * function but it extracts those
				    * degrees of freedom whose shape
				    * functions are nonzero on at
				    * least part of the selected
				    * boundary. For continuous
				    * elements, this is exactly the
				    * set of shape functions whose
				    * degrees of freedom are defined
				    * on boundary faces. On the
				    * other hand, if the finite
				    * element in used is a
				    * discontinuous element, all
				    * degrees of freedom are defined
				    * in the inside of cells and
				    * consequently none would be
				    * boundary degrees of
				    * freedom. Several of those
				    * would have shape functions
				    * that are nonzero on the
				    * boundary, however. This
				    * function therefore extracts
				    * all those for which the
				    * FiniteElement::has_support_on_face
				    * function says that it is
				    * nonzero on any face on one of
				    * the selected boundary parts.
				    */
  template <class DH>
  void
  extract_dofs_with_support_on_boundary (const DH                   &dof_handler,
					 const std::vector<bool>    &component_select,
					 std::vector<bool>          &selected_dofs,
					 const std::set<unsigned char> &boundary_indicators = std::set<unsigned char>());

				   /**
				    * @name Hanging Nodes
				    * @{
				    */

				   /**
				    * Select all dofs that will be
				    * constrained by interface
				    * constraints, i.e. all hanging
				    * nodes.
				    *
				    * The size of @p selected_dofs
				    * shall equal
				    * <tt>dof_handler.n_dofs()</tt>. Previous
				    * contents of this array or
				    * overwritten.
				    */
  template <int dim, int spacedim>
  void
  extract_hanging_node_dofs (const DoFHandler<dim,spacedim> &dof_handler,
			     std::vector<bool>              &selected_dofs);
				   //@}

				   /**
				    * @name Parallelization and domain decomposition
				    * @{
				    */
				   /**
				    * Flag all those degrees of
				    * freedom which are on cells
				    * with the given subdomain
				    * id. Note that DoFs on faces
				    * can belong to cells with
				    * differing subdomain ids, so
				    * the sets of flagged degrees of
				    * freedom are not mutually
				    * exclusive for different
				    * subdomain ids.
				    *
				    * If you want to get a unique
				    * association of degree of freedom with
				    * subdomains, use the
				    * @p get_subdomain_association
				    * function.
				    */
  template <class DH>
  void
  extract_subdomain_dofs (const DH           &dof_handler,
			  const types::subdomain_id_t subdomain_id,
			  std::vector<bool>  &selected_dofs);


				   /**
				    * Extract the set of global DoF
				    * indices that are owned by the
				    * current processor. For regular
				    * DoFHandler objects, this set
				    * is the complete set with all
				    * DoF indices. In either case,
				    * it equals what
				    * DoFHandler::locally_owned_dofs()
				    * returns.
				    */
  template <class DH>
  void
  extract_locally_owned_dofs (const DH & dof_handler,
			      IndexSet & dof_set);


				   /**
				    * Extract the set of global DoF
				    * indices that are active on the
				    * current DoFHandler. For
				    * regular DoFHandlers, these are
				    * all DoF indices, but for
				    * DoFHandler objects built on
				    * parallel::distributed::Triangulation
				    * this set is a superset of
				    * DoFHandler::locally_owned_dofs()
				    * and contains all DoF indices
				    * that live on all locally owned
				    * cells (including on the
				    * interface to ghost
				    * cells). However, it does not
				    * contain the DoF indices that
				    * are exclusively defined on
				    * ghost or artificial cells (see
				    * @ref GlossArtificialCell "the
				    * glossary").
				    *
				    * The degrees of freedom identified by
				    * this function equal those obtained
				    * from the
				    * dof_indices_with_subdomain_association()
				    * function when called with the locally
				    * owned subdomain id.
				    */
  template <class DH>
  void
  extract_locally_active_dofs (const DH & dof_handler,
			       IndexSet & dof_set);

				   /**
				    * Extract the set of global DoF
				    * indices that are active on the
				    * current DoFHandler. For
				    * regular DoFHandlers, these are
				    * all DoF indices, but for
				    * DoFHandler objects built on
				    * parallel::distributed::Triangulation
				    * this set is the union of
				    * DoFHandler::locally_owned_dofs()
				    * and the DoF indices on all
				    * ghost cells. In essence, it is
				    * the DoF indices on all cells
				    * that are not artificial (see
				    * @ref GlossArtificialCell "the glossary").
				    */
  template <class DH>
  void
  extract_locally_relevant_dofs (const DH & dof_handler,
				 IndexSet & dof_set);

				   /**
				    * For each DoF, return in the output
				    * array to which subdomain (as given by
				    * the <tt>cell->subdomain_id()</tt> function)
				    * it belongs. The output array is
				    * supposed to have the right size
				    * already when calling this function.
				    *
				    * Note that degrees of freedom
				    * associated with faces, edges, and
				    * vertices may be associated with
				    * multiple subdomains if they are
				    * sitting on partition boundaries. In
				    * these cases, we put them into one of
				    * the associated partitions in an
				    * undefined way. This may sometimes lead
				    * to different numbers of degrees of
				    * freedom in partitions, even if the
				    * number of cells is perfectly
				    * equidistributed. While this is
				    * regrettable, it is not a problem in
				    * practice since the number of degrees
				    * of freedom on partition boundaries is
				    * asymptotically vanishing as we refine
				    * the mesh as long as the number of
				    * partitions is kept constant.
				    *
				    * This function returns the association
				    * of each DoF with one subdomain. If you
				    * are looking for the association of
				    * each @em cell with a subdomain, either
				    * query the
				    * <tt>cell->subdomain_id()</tt>
				    * function, or use the
				    * <tt>GridTools::get_subdomain_association</tt>
				    * function.
				    *
				    * Note that this function is of
				    * questionable use for DoFHandler objects built on
				    * parallel::distributed::Triangulation
				    * since in that case ownership of
				    * individual degrees of freedom by MPI
				    * processes is controlled by the DoF
				    * handler object, not based on some
				    * geometric algorithm in conjunction
				    * with subdomain id. In particular, the
				    * degrees of freedom identified by the
				    * functions in this namespace as
				    * associated with a subdomain are not
				    * the same the
				    * DoFHandler class
				    * identifies as those it owns.
				    */
  template <class DH>
  void
  get_subdomain_association (const DH                  &dof_handler,
			     std::vector<types::subdomain_id_t> &subdomain);

				   /**
				    * Count how many degrees of freedom are
				    * uniquely associated with the given
				    * @p subdomain index.
				    *
				    * Note that there may be rare cases
				    * where cells with the given @p
				    * subdomain index exist, but none of its
				    * degrees of freedom are actually
				    * associated with it. In that case, the
				    * returned value will be zero.
				    *
				    * This function will generate an
				    * exception if there are no cells with
				    * the given @p subdomain index.
				    *
				    * This function returns the number of
				    * DoFs associated with one subdomain. If
				    * you are looking for the association of
				    * @em cells with this subdomain, use the
				    * <tt>GridTools::count_cells_with_subdomain_association</tt>
				    * function.
				    *
				    * Note that this function is of
				    * questionable use for DoFHandler objects built on
				    * parallel::distributed::Triangulation
				    * since in that case ownership of
				    * individual degrees of freedom by MPI
				    * processes is controlled by the DoF
				    * handler object, not based on some
				    * geometric algorithm in conjunction
				    * with subdomain id. In particular, the
				    * degrees of freedom identified by the
				    * functions in this namespace as
				    * associated with a subdomain are not
				    * the same the
				    * DoFHandler class
				    * identifies as those it owns.
				    */
  template <class DH>
  unsigned int
  count_dofs_with_subdomain_association (const DH           &dof_handler,
					 const types::subdomain_id_t subdomain);

				   /**
				    * Count how many degrees of freedom are
				    * uniquely associated with the given
				    * @p subdomain index.
				    *
				    * This function does what the previous
				    * one does except that it splits the
				    * result among the vector components of
				    * the finite element in use by the
				    * DoFHandler object. The last argument
				    * (which must have a length equal to the
				    * number of vector components) will
				    * therefore store how many degrees of
				    * freedom of each vector component are
				    * associated with the given subdomain.
				    *
				    * Note that this function is of
				    * questionable use for DoFHandler objects built on
				    * parallel::distributed::Triangulation
				    * since in that case ownership of
				    * individual degrees of freedom by MPI
				    * processes is controlled by the DoF
				    * handler object, not based on some
				    * geometric algorithm in conjunction
				    * with subdomain id. In particular, the
				    * degrees of freedom identified by the
				    * functions in this namespace as
				    * associated with a subdomain are not
				    * the same the
				    * DoFHandler class
				    * identifies as those it owns.
				    */
  template <class DH>
  void
  count_dofs_with_subdomain_association (const DH           &dof_handler,
					 const types::subdomain_id_t subdomain,
					 std::vector<unsigned int> &n_dofs_on_subdomain);

				   /**
				    * Return a set of indices that denotes
				    * the degrees of freedom that live on
				    * the given subdomain, i.e. that are on
				    * cells owned by the current
				    * processor. Note that this includes the
				    * ones that this subdomain "owns"
				    * (i.e. the ones for which
				    * get_subdomain_association() returns a
				    * value equal to the subdomain given
				    * here and that are selected by the
				    * extract_locally_owned() function) but
				    * also all of those that sit on the
				    * boundary between the given subdomain
				    * and other subdomain. In essence,
				    * degrees of freedom that sit on
				    * boundaries between subdomain will be
				    * in the index sets returned by this
				    * function for more than one subdomain.
				    *
				    * Note that this function is of
				    * questionable use for DoFHandler objects built on
				    * parallel::distributed::Triangulation
				    * since in that case ownership of
				    * individual degrees of freedom by MPI
				    * processes is controlled by the DoF
				    * handler object, not based on some
				    * geometric algorithm in conjunction
				    * with subdomain id. In particular, the
				    * degrees of freedom identified by the
				    * functions in this namespace as
				    * associated with a subdomain are not
				    * the same the
				    * DoFHandler class
				    * identifies as those it owns.
				    */
  template <class DH>
  IndexSet
  dof_indices_with_subdomain_association (const DH           &dof_handler,
					  const types::subdomain_id_t subdomain);

				   // @}
				   /**
				    * @name Dof indices for patches
				    *
				    * Create structures containing a
				    * large set of degrees of freedom
				    * for small patches of cells. The
				    * resulting objects can be used in
				    * RelaxationBlockSOR and related
				    * classes to implement Schwarz
				    * preconditioners and smoothers,
				    * where the subdomains consist of
				    * small numbers of cells only.
				    */
				   //@{
				   /**
				    * Create a sparsity pattern which
				    * in each row lists the degrees of
				    * freedom associated to the
				    * corresponding cell.
				    *
				    * Ordering follows the ordering of
				    * the standard cell iterators.
				    */
  template <class DH, class Sparsity>
  void make_cell_patches(Sparsity& block_list,
			 const DH& dof_handler,
			 const unsigned int level);
  
				    /**
				     * Create a sparsity pattern which
				     * in each row lists the degrees of
				     * freedom associated to the
				     * corresponding cells
				     * corresponding to a vertex.
				     *
				     * The function has some boolean
				     * arguments (lsited below)
				     * controlling details of the
				     * generated patches. The default
				     * settings are those for
				     * Arnold-Falk-Winther type
				     * smoothers for divergence and
				     * curl conforming finite elements
				     * with essential boundary
				     * conditions. Other applications
				     * are possible, in particular
				     * changing
				     * <tt>boundary_patches</tt> for
				     * non-essential boundary conditions.
				     *
				     * @arg <tt>block_list</tt>: the
				     * SparsityPattern into which the
				     * patches will be stored.
				     * @arg <tt>dof_handler</tt>: The
				     * multilevel dof handler
				     * providing the topology operated
				     * on.
				     * @arg
				     * <tt>interior_dofs_only</tt>:
				     * for each patch of cells around
				     * a vertex, collect only the
				     * interior degrees of freedom of
				     * the patch and disregard those
				     * on the boundary of the
				     * patch. This is for instance the
				     * setting for smoothers of
				     * Arnold-Falk-Winther type.
				     * @arg <tt>boundary_patches</tt>:
				     * include patches around vertices
				     * at the boundary of the
				     * domain. If not, only patches
				     * around interior vertices will
				     * be generated.
				     * @arg
				     * <tt>level_boundary_patches</tt>:
				     * same for refinement edges
				     * towards coarser cells.
				     * @arg
				     * <tt>single_cell_patches</tt>:
				     * if not true, patches containing
				     * a single cell are eliminated.
				     */
   template <class DH>
   void make_vertex_patches(SparsityPattern& block_list,
			    const DH& dof_handler,
			    const unsigned int level,
			    const bool interior_dofs_only,
			    const bool boundary_patches = false,
			    const bool level_boundary_patches = false,
			    const bool single_cell_patches = false);
  
				   //@}
				   /**
				    * Extract a vector that represents the
				    * constant modes of the DoFHandler for
				    * the components chosen by
				    * <tt>component_select</tt>.  The
				    * constant modes on a discretization are
				    * the null space of a Laplace operator
				    * on the selected components with
				    * Neumann boundary conditions
				    * applied. The null space is a necessary
				    * ingredient for obtaining a good AMG
				    * preconditioner when using the class
				    * TrilinosWrappers::PreconditionAMG.
				    * Since the ML AMG package only works on
				    * algebraic properties of the respective
				    * matrix, it has no chance to detect
				    * whether the matrix comes from a scalar
				    * or a vector valued problem. However, a
				    * near null space supplies exactly the
				    * needed information about these
				    * components.  The null space will
				    * consist of as many vectors as there
				    * are true arguments in
				    * <tt>component_select</tt>, each of
				    * which will be one in one vector component and
				    * zero in all others. We store this
				    * object in a vector of vectors, where
				    * the outer vector is of the size of the
				    * number of selected components, and
				    * each inner vector has as many
				    * components as there are (locally owned) degrees of
				    * freedom in the selected
				    * components. Note that any matrix
				    * associated with this null space must
				    * have been constructed using the same
				    * <tt>component_select</tt> argument,
				    * since the numbering of DoFs is done
				    * relative to the selected dofs, not to
				    * all dofs.
				    *
				    * The main reason for this
				    * program is the use of the
				    * null space with the
				    * AMG preconditioner.
				    */
  template <class DH>
  void
  extract_constant_modes (const DH                        &dof_handler,
			  const std::vector<bool>         &component_select,
			  std::vector<std::vector<bool> > &constant_modes);

				   /**
				    * For each active cell of a DoFHandler
				    * or hp::DoFHandler, extract the active
				    * finite element index and fill the
				    * vector given as second argument. This
				    * vector is assumed to have as many
				    * entries as there are active cells.
				    *
				    * For non-hp DoFHandler objects given as
				    * first argument, the returned vector
				    * will consist of only zeros, indicating
				    * that all cells use the same finite
				    * element. For a hp::DoFHandler, the
				    * values may be different, though.
				    */
  template <class DH>
  void
  get_active_fe_indices (const DH                  &dof_handler,
			 std::vector<unsigned int> &active_fe_indices);

				   /**
				    * Count how many degrees of
				    * freedom out of the total
				    * number belong to each
				    * component. If the number of
				    * components the finite element
				    * has is one (i.e. you only have
				    * one scalar variable), then the
				    * number in this component
				    * obviously equals the total
				    * number of degrees of
				    * freedom. Otherwise, the sum of
				    * the DoFs in all the components
				    * needs to equal the total
				    * number.
				    *
				    * However, the last statement
				    * does not hold true if the
				    * finite element is not
				    * primitive, i.e. some or all of
				    * its shape functions are
				    * non-zero in more than one
				    * vector component. This
				    * applies, for example, to the
				    * Nedelec or Raviart-Thomas
				    * elements. In this case, a
				    * degree of freedom is counted
				    * in each component in which it
				    * is non-zero, so that the sum
				    * mentioned above is greater
				    * than the total number of
				    * degrees of freedom.
				    *
				    * This behavior can be switched
				    * off by the optional parameter
				    * <tt>vector_valued_once</tt>. If
				    * this is <tt>true</tt>, the
				    * number of components of a
				    * nonprimitive vector valued
				    * element is collected only in
				    * the first component. All other
				    * components will have a count
				    * of zero.
				    *
				    * The additional optional
				    * argument @p target_component
				    * allows for a re-sorting and
				    * grouping of components. To
				    * this end, it contains for each
				    * component the component number
				    * it shall be counted as. Having
				    * the same number entered
				    * several times sums up several
				    * components as the same. One of
				    * the applications of this
				    * argument is when you want to
				    * form block matrices and
				    * vectors, but want to pack
				    * several components into the
				    * same block (for example, when
				    * you have @p dim velocities
				    * and one pressure, to put all
				    * velocities into one block, and
				    * the pressure into another).
				    *
				    * The result is returned in @p
				    * dofs_per_component. Note that
				    * the size of @p
				    * dofs_per_component needs to be
				    * enough to hold all the indices
				    * specified in @p
				    * target_component. If this is
				    * not the case, an assertion is
				    * thrown. The indices not
				    * targetted by target_components
				    * are left untouched.
				    */
  template <class DH>
  void
  count_dofs_per_component (const DH &     dof_handler,
			    std::vector<unsigned int>& dofs_per_component,
			    const bool vector_valued_once = false,
			    std::vector<unsigned int>  target_component
			    = std::vector<unsigned int>());

				   /**
				    * Count the degrees of freedom
				    * in each block. This function
				    * is similar to
				    * count_dofs_per_component(),
				    * with the difference that the
				    * counting is done by
				    * blocks. See @ref GlossBlock
				    * "blocks" in the glossary for
				    * details. Again the vectors are
				    * assumed to have the correct
				    * size before calling this
				    * function. If this is not the
				    * case, an assertion is thrown.
				    *
				    * This function is used in the
				    * step-22,
				    * step-31, and
				    * step-32 tutorial
				    * programs.
				    *
				    * @pre The dofs_per_block
				    * variable has as many
				    * components as the finite
				    * element used by the
				    * dof_handler argument has
				    * blocks, or alternatively as
				    * many blocks as are enumerated
				    * in the target_blocks argument
				    * if given.
				    */
  template <int dim, int spacedim>
  void
  count_dofs_per_block (const DoFHandler<dim,spacedim>&     dof_handler,
			std::vector<unsigned int>& dofs_per_block,
			std::vector<unsigned int>  target_blocks
			= std::vector<unsigned int>());

				   /**
				    * @deprecated See the previous
				    * function with the same name
				    * for a description. This
				    * function exists for
				    * compatibility with older
				    * versions only.
				    */
  template <int dim, int spacedim>
  void
  count_dofs_per_component (const DoFHandler<dim,spacedim>&     dof_handler,
			    std::vector<unsigned int>& dofs_per_component,
			    std::vector<unsigned int>  target_component);

				   /**
				    * This function can be used when
				    * different variables shall be
				    * discretized on different
				    * grids, where one grid is
				    * coarser than the other. This
				    * idea might seem nonsensical at
				    * first, but has reasonable
				    * applications in inverse
				    * (parameter estimation)
				    * problems, where there might
				    * not be enough information to
				    * recover the parameter on the
				    * same grid as the state
				    * variable; furthermore, the
				    * smoothness properties of state
				    * variable and parameter might
				    * not be too much related, so
				    * using different grids might be
				    * an alternative to using
				    * stronger regularization of the
				    * problem.
				    *
				    * The basic idea of this
				    * function is explained in the
				    * following. Let us, for
				    * convenience, denote by
				    * ``parameter grid'' the coarser
				    * of the two grids, and by
				    * ``state grid'' the finer of
				    * the two. We furthermore assume
				    * that the finer grid can be
				    * obtained by refinement of the
				    * coarser one, i.e. the fine
				    * grid is at least as much
				    * refined as the coarse grid at
				    * each point of the
				    * domain. Then, each shape
				    * function on the coarse grid
				    * can be represented as a linear
				    * combination of shape functions
				    * on the fine grid (assuming
				    * identical ansatz
				    * spaces). Thus, if we
				    * discretize as usual, using
				    * shape functions on the fine
				    * grid, we can consider the
				    * restriction that the parameter
				    * variable shall in fact be
				    * discretized by shape functions
				    * on the coarse grid as a
				    * constraint. These constraints
				    * are linear and happen to have
				    * the form managed by the
				    * ``ConstraintMatrix'' class.
				    *
				    * The construction of these
				    * constraints is done as
				    * follows: for each of the
				    * degrees of freedom (i.e. shape
				    * functions) on the coarse grid,
				    * we compute its representation
				    * on the fine grid, i.e. how the
				    * linear combination of shape
				    * functions on the fine grid
				    * looks like that resembles the
				    * shape function on the coarse
				    * grid. From this information,
				    * we can then compute the
				    * constraints which have to hold
				    * if a solution of a linear
				    * equation on the fine grid
				    * shall be representable on the
				    * coarse grid. The exact
				    * algorithm how these
				    * constraints can be computed is
				    * rather complicated and is best
				    * understood by reading the
				    * source code, which contains
				    * many comments.
				    *
				    * Before explaining the use of
				    * this function, we would like
				    * to state that the total number
				    * of degrees of freedom used for
				    * the discretization is not
				    * reduced by the use of this
				    * function, i.e. even though we
				    * discretize one variable on a
				    * coarser grid, the total number
				    * of degrees of freedom is that
				    * of the fine grid. This seems
				    * to be counter-productive,
				    * since it does not give us a
				    * benefit from using a coarser
				    * grid. The reason why it may be
				    * useful to choose this approach
				    * nonetheless is three-fold:
				    * first, as stated above, there
				    * might not be enough
				    * information to recover a
				    * parameter on a fine grid,
				    * i.e. we chose to discretize it
				    * on the coarse grid not to save
				    * DoFs, but for other
				    * reasons. Second, the
				    * ``ConstraintMatrix'' includes
				    * the constraints into the
				    * linear system of equations, by
				    * which constrained nodes become
				    * dummy nodes; we may therefore
				    * exclude them from the linear
				    * algebra, for example by
				    * sorting them to the back of
				    * the DoF numbers and simply
				    * calling the solver for the
				    * upper left block of the matrix
				    * which works on the
				    * non-constrained nodes only,
				    * thus actually realizing the
				    * savings in numerical effort
				    * from the reduced number of
				    * actual degrees of freedom. The
				    * third reason is that for some
				    * or other reason we have chosen
				    * to use two different grids, it
				    * may be actually quite
				    * difficult to write a function
				    * that assembles the system
				    * matrix for finite element
				    * spaces on different grids;
				    * using the approach of
				    * constraints as with this
				    * function allows to use
				    * standard techniques when
				    * discretizing on only one grid
				    * (the finer one) without having
				    * to take care of the fact that
				    * one or several of the variable
				    * actually belong to different
				    * grids.
				    *
				    * The use of this function is as
				    * follows: it accepts as
				    * parameters two DoF Handlers,
				    * the first of which refers to
				    * the coarse grid and the second
				    * of which is the fine grid. On
				    * both, a finite element is
				    * represented by the DoF handler
				    * objects, which will usually
				    * have several components, which
				    * may belong to different finite
				    * elements. The second and
				    * fourth parameter of this
				    * function therefore state which
				    * variable on the coarse grid
				    * shall be used to restrict the
				    * stated component on the fine
				    * grid. Of course, the finite
				    * elements used for the
				    * respective components on the
				    * two grids need to be the
				    * same. An example may clarify
				    * this: consider the parameter
				    * estimation mentioned briefly
				    * above; there, on the fine grid
				    * the whole discretization is
				    * done, thus the variables are
				    * ``u'', ``q'', and the Lagrange
				    * multiplier ``lambda'', which
				    * are discretized using
				    * continuous linear, piecewise
				    * constant discontinuous, and
				    * continuous linear elements,
				    * respectively. Only the
				    * parameter ``q'' shall be
				    * represented on the coarse
				    * grid, thus the DoFHandler
				    * object on the coarse grid
				    * represents only one variable,
				    * discretized using piecewise
				    * constant discontinuous
				    * elements. Then, the parameter
				    * denoting the component on the
				    * coarse grid would be zero (the
				    * only possible choice, since
				    * the variable on the coarse
				    * grid is scalar), and one on
				    * the fine grid (corresponding
				    * to the variable ``q''; zero
				    * would be ``u'', two would be
				    * ``lambda''). Furthermore, an
				    * object of type IntergridMap
				    * is needed; this could in
				    * principle be generated by the
				    * function itself from the two
				    * DoFHandler objects, but since
				    * it is probably available
				    * anyway in programs that use
				    * this function, we shall use it
				    * instead of re-generating
				    * it. Finally, the computed
				    * constraints are entered into a
				    * variable of type
				    * ConstraintMatrix; the
				    * constraints are added,
				    * i.e. previous contents which
				    * may have, for example, be
				    * obtained from hanging nodes,
				    * are not deleted, so that you
				    * only need one object of this
				    * type.
				    */
  template <int dim, int spacedim>
  void
  compute_intergrid_constraints (const DoFHandler<dim,spacedim>              &coarse_grid,
				 const unsigned int                  coarse_component,
				 const DoFHandler<dim,spacedim>              &fine_grid,
				 const unsigned int                  fine_component,
				 const InterGridMap<DoFHandler<dim,spacedim> > &coarse_to_fine_grid_map,
				 ConstraintMatrix                   &constraints);


				   /**
				    * This function generates a
				    * matrix such that when a vector
				    * of data with as many elements
				    * as there are degrees of
				    * freedom of this component on
				    * the coarse grid is multiplied
				    * to this matrix, we obtain a
				    * vector with as many elements
				    * are there are global degrees
				    * of freedom on the fine
				    * grid. All the elements of the
				    * other components of the finite
				    * element fields on the fine
				    * grid are not touched.
				    *
				    * The output of this function is
				    * a compressed format that can
				    * be given to the @p reinit
				    * functions of the
				    * SparsityPattern ad
				    * SparseMatrix classes.
				    */
  template <int dim, int spacedim>
  void
  compute_intergrid_transfer_representation (const DoFHandler<dim,spacedim>              &coarse_grid,
					     const unsigned int                  coarse_component,
					     const DoFHandler<dim,spacedim>              &fine_grid,
					     const unsigned int                  fine_component,
					     const InterGridMap<DoFHandler<dim,spacedim> > &coarse_to_fine_grid_map,
					     std::vector<std::map<unsigned int, float> > &transfer_representation);

				   /**
				    * Create a mapping from degree
				    * of freedom indices to the
				    * index of that degree of
				    * freedom on the boundary. After
				    * this operation, <tt>mapping[dof]</tt>
				    * gives the index of the
				    * degree of freedom with global
				    * number @p dof in the list of
				    * degrees of freedom on the
				    * boundary.  If the degree of
				    * freedom requested is not on
				    * the boundary, the value of
				    * <tt>mapping[dof]</tt> is
				    * @p invalid_dof_index. This
				    * function is mainly used when
				    * setting up matrices and
				    * vectors on the boundary from
				    * the trial functions, which
				    * have global numbers, while the
				    * matrices and vectors use
				    * numbers of the trial functions
				    * local to the boundary.
				    *
				    * Prior content of @p mapping
				    * is deleted.
				    */
  template <class DH>
  void
  map_dof_to_boundary_indices (const DH                   &dof_handler,
			       std::vector<unsigned int>  &mapping);

				   /**
				    * Same as the previous function,
				    * except that only those parts
				    * of the boundary are considered
				    * for which the boundary
				    * indicator is listed in the
				    * second argument.
				    *
				    * See the general doc of this
				    * class for more information.
				    */
  template <class DH>
  void
  map_dof_to_boundary_indices (const DH                      &dof_handler,
			       const std::set<unsigned char> &boundary_indicators,
			       std::vector<unsigned int>     &mapping);

				   /**
				    * Return a list of support
				    * points for all the degrees of
				    * freedom handled by this DoF
				    * handler object. This function,
				    * of course, only works if the
				    * finite element object used by
				    * the DoF handler object
				    * actually provides support
				    * points, i.e. no edge elements
				    * or the like. Otherwise, an
				    * exception is thrown.
				    *
				    * The given array must have a
				    * length of as many elements as
				    * there are degrees of freedom.
				    */
  template <int dim, int spacedim>
  void
  map_dofs_to_support_points (const Mapping<dim,spacedim>       &mapping,
			      const DoFHandler<dim,spacedim>    &dof_handler,
			      std::vector<Point<spacedim> >     &support_points);

				   /**
				    * This is the opposite function
				    * to the one above. It generates
				    * a map where the keys are the
				    * support points of the degrees
				    * of freedom, while the values
				    * are the DoF indices.
				    *
				    * Since there is no natural
				    * order in the space of points
				    * (except for the 1d case), you
				    * have to provide a map with an
				    * explicitly specified
				    * comparator object. This
				    * function is therefore
				    * templatized on the comparator
				    * object. Previous content of
				    * the map object is deleted in
				    * this function.
				    *
				    * Just as with the function
				    * above, it is assumed that the
				    * finite element in use here
				    * actually supports the notion
				    * of support points of all its
				    * components.
				    */
  template <class DH, class Comp>
  void
  map_support_points_to_dofs (const Mapping<DH::dimension, DH::space_dimension> &mapping,
			      const DH                                          &dof_handler,
			      std::map<Point<DH::space_dimension>, unsigned int, Comp> &point_to_index_map);

				   /**
				    * Map a coupling table from the
				    * user friendly organization by
				    * components to the organization
				    * by blocks. Specializations of
				    * this function for DoFHandler
				    * and hp::DoFHandler are
				    * required due to the different
				    * results of their finite
				    * element access.
				    *
				    * The return vector will be
				    * initialized to the correct
				    * length inside this function.
				    */
  template <int dim, int spacedim>
  void
  convert_couplings_to_blocks (const hp::DoFHandler<dim,spacedim>& dof_handler,
			       const Table<2, Coupling>& table_by_component,
			       std::vector<Table<2,Coupling> >& tables_by_block);

				   /**
				    * Make a constraint matrix for the
				    * constraints that result from zero
				    * boundary values.
				    *
				    * This function constrains all
				    * degrees of freedom on the
				    * boundary. Optionally, you can
				    * add a component mask, which
				    * restricts this functionality
				    * to a subset of an FESystem.
				    *
				    * For non-@ref GlossPrimitive "primitive"
				    * shape functions, any degree of freedom
				    * is affected that belongs to a
				    * shape function where at least
				    * one of its nonzero components
				    * is affected.
				    *
				    * This function is used
				    * in step-36, for
				    * example.
				    *
				    * @ingroup constraints
				    */
  template <int dim, int spacedim, template <int, int> class DH>
  void
  make_zero_boundary_constraints (const DH<dim,spacedim> &dof,
				  ConstraintMatrix        &zero_boundary_constraints,
				  const std::vector<bool> &component_mask_=std::vector<bool>());

				   /**
				    * Map a coupling table from the
				    * user friendly organization by
				    * components to the organization
				    * by blocks. Specializations of
				    * this function for DoFHandler
				    * and hp::DoFHandler are
				    * required due to the different
				    * results of their finite
				    * element access.
				    *
				    * The return vector will be
				    * initialized to the correct
				    * length inside this function.
				    */
  template <int dim, int spacedim>
  void
  convert_couplings_to_blocks (const DoFHandler<dim,spacedim>& dof_handler,
			       const Table<2, Coupling>& table_by_component,
			       std::vector<Table<2,Coupling> >& tables_by_block);

				   /**
				    * Given a finite element and a table how
				    * the vector components of it couple
				    * with each other, compute and return a
				    * table that describes how the
				    * individual shape functions couple with
				    * each other.
				    */
  template <int dim, int spacedim>
  Table<2,Coupling>
  dof_couplings_from_component_couplings (const FiniteElement<dim,spacedim> &fe,
					  const Table<2,Coupling> &component_couplings);

				   /**
				    * Same function as above for a
				    * collection of finite elements,
				    * returning a collection of tables.
				    *
				    * The function currently treats
				    * DoFTools::Couplings::nonzero the same
				    * as DoFTools::Couplings::always .
				    */
  template <int dim, int spacedim>
  std::vector<Table<2,Coupling> >
  dof_couplings_from_component_couplings (const hp::FECollection<dim,spacedim> &fe,
					  const Table<2,Coupling> &component_couplings);
				   /**
				    * Exception
				    * @ingroup Exceptions
				    */
  DeclException0 (ExcFiniteElementsDontMatch);
				   /**
				    * Exception
				    * @ingroup Exceptions
				    */
  DeclException0 (ExcGridNotCoarser);
				   /**
				    * Exception
				    * @ingroup Exceptions
				    */
  DeclException0 (ExcGridsDontMatch);
				   /**
				    * Exception
				    * @ingroup Exceptions
				    */
  DeclException0 (ExcNoFESelected);
				   /**
				    * Exception
				    * @ingroup Exceptions
				    */
  DeclException0 (ExcInvalidBoundaryIndicator);
}



/* ------------------------- inline functions -------------- */

#ifndef DOXYGEN

namespace DoFTools
{
/**
 * Operator computing the maximum coupling out of two.
 *
 * @relates DoFTools
 */
  inline
  Coupling operator |= (Coupling& c1,
			const Coupling c2)
  {
    if (c2 == always)
      c1 = always;
    else if (c1 != always && c2 == nonzero)
      return c1 = nonzero;
    return c1;
  }


/**
 * Operator computing the maximum coupling out of two.
 *
 * @relates DoFTools
 */
  inline
  Coupling operator | (const Coupling c1,
		       const Coupling c2)
  {
    if (c1 == always || c2 == always)
      return always;
    if (c1 == nonzero || c2 == nonzero)
      return nonzero;
    return none;
  }


// ---------------------- inline and template functions --------------------

  template <int dim, int spacedim>
  inline
  unsigned int
  max_dofs_per_cell (const DoFHandler<dim,spacedim> &dh)
  {
    return dh.get_fe().dofs_per_cell;
  }


  template <int dim, int spacedim>
  inline
  unsigned int
  max_dofs_per_face (const DoFHandler<dim,spacedim> &dh)
  {
    return dh.get_fe().dofs_per_face;
  }


  template <int dim, int spacedim>
  inline
  unsigned int
  max_dofs_per_vertex (const DoFHandler<dim,spacedim> &dh)
  {
    return dh.get_fe().dofs_per_vertex;
  }


  template <int dim, int spacedim>
  inline
  unsigned int
  n_components (const DoFHandler<dim,spacedim> &dh)
  {
    return dh.get_fe().n_components();
  }



  template <int dim, int spacedim>
  inline
  bool
  fe_is_primitive (const DoFHandler<dim,spacedim> &dh)
  {
    return dh.get_fe().is_primitive();
  }


  template <int dim, int spacedim>
  inline
  unsigned int
  max_dofs_per_cell (const hp::DoFHandler<dim,spacedim> &dh)
  {
    return dh.get_fe().max_dofs_per_cell ();
  }


  template <int dim, int spacedim>
  inline
  unsigned int
  max_dofs_per_face (const hp::DoFHandler<dim,spacedim> &dh)
  {
    return dh.get_fe().max_dofs_per_face ();
  }


  template <int dim, int spacedim>
  inline
  unsigned int
  max_dofs_per_vertex (const hp::DoFHandler<dim,spacedim> &dh)
  {
    return dh.get_fe().max_dofs_per_vertex ();
  }


  template <int dim, int spacedim>
  inline
  unsigned int
  n_components (const hp::DoFHandler<dim,spacedim> &dh)
  {
    return dh.get_fe()[0].n_components();
  }


  template <int dim, int spacedim>
  inline
  bool
  fe_is_primitive (const hp::DoFHandler<dim,spacedim> &dh)
  {
    return dh.get_fe()[0].is_primitive();
  }


  template <class DH, class SparsityPattern>
  inline
  void
  make_sparsity_pattern (const DH                              &dof,
			 const std::vector<std::vector<bool> > &mask,
			 SparsityPattern                       &sparsity_pattern)
  {
    const unsigned int ncomp = dof.get_fe().n_components();

    Assert (mask.size() == ncomp,
	    ExcDimensionMismatch(mask.size(), ncomp));
    for (unsigned int i=0; i<mask.size(); ++i)
      Assert (mask[i].size() == ncomp,
	      ExcDimensionMismatch(mask[i].size(), ncomp));
				     // Create a coupling table out of the mask
    Table<2,DoFTools::Coupling> couplings(ncomp, ncomp);
    for (unsigned int i=0;i<ncomp;++i)
      for (unsigned int j=0;j<ncomp;++j)
	if (mask[i][j])
	  couplings(i,j) = always;
	else
	  couplings(i,j) = none;

				     // Call the new function
    make_sparsity_pattern(dof, couplings, sparsity_pattern);
  }


  template <class DH, class Comp>
  void
  map_support_points_to_dofs (
    const Mapping<DH::dimension,DH::space_dimension>         &mapping,
    const DH                                                 &dof_handler,
    std::map<Point<DH::space_dimension>, unsigned int, Comp> &point_to_index_map)
  {
				     // let the checking of arguments be
				     // done by the function first
				     // called
    std::vector<Point<DH::space_dimension> > support_points (dof_handler.n_dofs());
    map_dofs_to_support_points (mapping, dof_handler, support_points);
				     // now copy over the results of the
				     // previous function into the
				     // output arg
    point_to_index_map.clear ();
    for (unsigned int i=0; i<dof_handler.n_dofs(); ++i)
      point_to_index_map[support_points[i]] = i;
  }
}

#endif

DEAL_II_NAMESPACE_CLOSE

#endif
