//---------------------------------------------------------------------------
//    $Id: fe_dgq.h 23709 2011-05-17 04:34:08Z bangerth $
//
//    Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2009, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__fe_dgq_h
#define __deal2__fe_dgq_h

#include <deal.II/base/config.h>
#include <deal.II/base/tensor_product_polynomials.h>
#include <deal.II/fe/fe_poly.h>

DEAL_II_NAMESPACE_OPEN

template <int dim, int spacedim> class MappingQ;
template <int dim> class Quadrature;

/*!@addtogroup fe */
/*@{*/

/**
 * Implementation of scalar, discontinuous tensor product elements
 * based on equidistant support points.
 *
 * This is a discontinuous finite element based on tensor products of
 * Lagrangian polynomials. The shape functions are Lagrangian
 * interpolants of an equidistant grid of points on the unit cell. The
 * points are numbered in lexicographical order, with <i>x</i> running
 * fastest, then <i>y</i>, then <i>z</i> (if these coordinates are present for a
 * given space dimension at all). For example, these are the node
 * orderings for <tt>FE_DGQ(1)</tt> in 3d:
 *  @verbatim
 *         6-------7        6-------7
 *        /|       |       /       /|
 *       / |       |      /       / |
 *      /  |       |     /       /  |
 *     4   |       |    4-------5   |
 *     |   2-------3    |       |   3
 *     |  /       /     |       |  /
 *     | /       /      |       | /
 *     |/       /       |       |/
 *     0-------1        0-------1
 *  @endverbatim
 * and <tt>FE_DGQ(2)</tt>:
 *  @verbatim
 *         24--25--26       24--25--26
 *        /|       |       /       /|
 *      21 |       |     21  22  23 |
 *      /  15  16  17    /       /  17
 *    18   |       |   18--19--20   |
 *     |12 6---7---8    |       |14 8
 *     9  /       /     9  10  11  /
 *     | 3   4   5      |       | 5
 *     |/       /       |       |/
 *     0---1---2        0---1---2
 *  @endverbatim
 * with node 13 being placed in the interior of the hex.
 *
 * Note, however, that these are just the Lagrange interpolation
 * points of the shape functions. Even though they may physically be
 * on the surface of the cell, they are logically in the interior
 * since there are no continuity requirements for these shape
 * functions across cell boundaries.
 * This class if partially implemented for the codimension one case
 * (<tt>spacedim != dim </tt>), since no passage of information
 * between meshes of different refinement level is possible because
 * the embedding and projection matrices are not computed in the class
 * constructor.
 *
 * @author Ralf Hartmann, Guido Kanschat 2001, 2004
 */
template <int dim, int spacedim=dim>
class FE_DGQ : public FE_Poly<TensorProductPolynomials<dim>, dim, spacedim>
{
  public:
				     /**
				      * Constructor for tensor product
				      * polynomials of degree
				      * <tt>p</tt>. The shape
				      * functions created using this
				      * constructor correspond to
				      * Lagrange interpolation
				      * polynomials for equidistantly
				      * spaced support points in each
				      * coordinate direction.
				      */
    FE_DGQ (const unsigned int p);

				     /**
				      * Return a string that uniquely
				      * identifies a finite
				      * element. This class returns
				      * <tt>FE_DGQ<dim>(degree)</tt> , with
				      * <tt>dim</tt> and <tt>degree</tt>
				      * replaced by appropriate
				      * values.
				      */
    virtual std::string get_name () const;
    
				     /**
				      * Return the matrix
				      * interpolating from the given
				      * finite element to the present
				      * one. The size of the matrix is
				      * then @p dofs_per_cell times
				      * <tt>source.dofs_per_cell</tt>.
				      *
				      * These matrices are only
				      * available if the source
				      * element is also a @p FE_DGQ
				      * element. Otherwise, an
				      * exception of type
				      * FiniteElement<dim>::ExcInterpolationNotImplemented
				      * is thrown.
				      */
    virtual void
    get_interpolation_matrix (const FiniteElement<dim, spacedim> &source,
			      FullMatrix<double>           &matrix) const;
    
				     /**
				      * Return the matrix
				      * interpolating from a face of
				      * of one element to the face of
				      * the neighboring element. 
				      * The size of the matrix is
				      * then <tt>source.dofs_per_face</tt> times
				      * <tt>this->dofs_per_face</tt>.
				      *
				      * Derived elements will have to
				      * implement this function. They
				      * may only provide interpolation
				      * matrices for certain source
				      * finite elements, for example
				      * those from the same family. If
				      * they don't implement
				      * interpolation from a given
				      * element, then they must throw
				      * an exception of type
				      * FiniteElement<dim>::ExcInterpolationNotImplemented.
				      */
    virtual void
    get_face_interpolation_matrix (const FiniteElement<dim, spacedim> &source,
				   FullMatrix<double>       &matrix) const;    

				     /**
				      * Return the matrix
				      * interpolating from a face of
				      * of one element to the face of
				      * the neighboring element. 
				      * The size of the matrix is
				      * then <tt>source.dofs_per_face</tt> times
				      * <tt>this->dofs_per_face</tt>.
				      *
				      * Derived elements will have to
				      * implement this function. They
				      * may only provide interpolation
				      * matrices for certain source
				      * finite elements, for example
				      * those from the same family. If
				      * they don't implement
				      * interpolation from a given
				      * element, then they must throw
				      * an exception of type
				      * FiniteElement<dim>::ExcInterpolationNotImplemented.
				      */
    virtual void
    get_subface_interpolation_matrix (const FiniteElement<dim, spacedim> &source,
				      const unsigned int        subface,
				      FullMatrix<double>       &matrix) const;

				     /**
				      * @name Functions to support hp 
				      * @{
				      */
    
				     /**
				      * If, on a vertex, several
				      * finite elements are active,
				      * the hp code first assigns the
				      * degrees of freedom of each of
				      * these FEs different global
				      * indices. It then calls this
				      * function to find out which of
				      * them should get identical
				      * values, and consequently can
				      * receive the same global DoF
				      * index. This function therefore
				      * returns a list of identities
				      * between DoFs of the present
				      * finite element object with the
				      * DoFs of @p fe_other, which is
				      * a reference to a finite
				      * element object representing
				      * one of the other finite
				      * elements active on this
				      * particular vertex. The
				      * function computes which of the
				      * degrees of freedom of the two
				      * finite element objects are
				      * equivalent, and returns a list
				      * of pairs of global dof indices
				      * in @p identities. The first
				      * index of each pair denotes one
				      * of the vertex dofs of the
				      * present element, whereas the
				      * second is the corresponding
				      * index of the other finite
				      * element.
				      *
				      * This being a discontinuous element,
				      * the set of such constraints is of
				      * course empty.
				      */
    virtual
    std::vector<std::pair<unsigned int, unsigned int> >
    hp_vertex_dof_identities (const FiniteElement<dim, spacedim> &fe_other) const;

				     /**
				      * Same as
				      * hp_vertex_dof_indices(),
				      * except that the function
				      * treats degrees of freedom on
				      * lines.
				      *
				      * This being a discontinuous element,
				      * the set of such constraints is of
				      * course empty.
				      */
    virtual
    std::vector<std::pair<unsigned int, unsigned int> >
    hp_line_dof_identities (const FiniteElement<dim, spacedim> &fe_other) const;

				     /**
				      * Same as
				      * hp_vertex_dof_indices(),
				      * except that the function
				      * treats degrees of freedom on
				      * quads.
				      *
				      * This being a discontinuous element,
				      * the set of such constraints is of
				      * course empty.
				      */
    virtual
    std::vector<std::pair<unsigned int, unsigned int> >
    hp_quad_dof_identities (const FiniteElement<dim, spacedim> &fe_other) const;

                                     /**
                                      * Return whether this element
                                      * implements its hanging node
                                      * constraints in the new way,
				      * which has to be used to make
				      * elements "hp compatible".
                                      *
				      * For the FE_DGQ class the result is
				      * always true (independent of the degree
				      * of the element), as it has no hanging
				      * nodes (being a discontinuous element).
                                      */
    virtual bool hp_constraints_are_implemented () const;

				     /**
				      * Return whether this element dominates
				      * the one given as argument when they
				      * meet at a common face,
				      * whether it is the other way around,
				      * whether neither dominates, or if
				      * either could dominate.
				      *
				      * For a definition of domination, see
				      * FiniteElementBase::Domination and in
				      * particular the @ref hp_paper "hp paper".
				      */
    virtual
    FiniteElementDomination::Domination
    compare_for_face_domination (const FiniteElement<dim, spacedim> &fe_other) const;

				     /**
				      * @}
				      */
    
				     /**
				      * Check for non-zero values on a face.
				      *
				      * This function returns
				      * @p true, if the shape
				      * function @p shape_index has
				      * non-zero values on the face
				      * @p face_index.
				      *
				      * Implementation of the
				      * interface in
				      * FiniteElement
				      */
    virtual bool has_support_on_face (const unsigned int shape_index,
				      const unsigned int face_index) const;

				     /**
				      * Determine an estimate for the
				      * memory consumption (in bytes)
				      * of this object.
				      *
				      * This function is made virtual,
				      * since finite element objects
				      * are usually accessed through
				      * pointers to their base class,
				      * rather than the class itself.
				      */
    virtual std::size_t memory_consumption () const;


  protected:
				     /**
				      * Constructor for tensor product
				      * polynomials based on
				      * Polynomials::Lagrange
				      * interpolation of the support
				      * points in the quadrature rule
				      * <tt>points</tt>. The degree of
				      * these polynomials is
				      * <tt>points.size()-1</tt>.
				      *
				      * Note: The FE_DGQ::clone function
				      * does not work properly for FE with
				      * arbitrary nodes!
				      */
    FE_DGQ (const Quadrature<1>& points);

				     /**
				      * @p clone function instead of
				      * a copy constructor.
				      *
				      * This function is needed by the
				      * constructors of @p FESystem.
				      */
    virtual FiniteElement<dim, spacedim> *clone() const;

  private:
				     /**
				      * Only for internal use. Its
				      * full name is
				      * @p get_dofs_per_object_vector
				      * function and it creates the
				      * @p dofs_per_object vector that is
				      * needed within the constructor to
				      * be passed to the constructor of
				      * @p FiniteElementData.
				      */
    static std::vector<unsigned int> get_dpo_vector (const unsigned int degree);
  
				     /**
				      * Compute renumbering for rotation
				      * of degrees of freedom.
				      *
				      * Rotates a tensor product
				      * numbering of degrees of
				      * freedom by 90 degrees. It is
				      * used to compute the transfer
				      * matrices of the children by
				      * using only the matrix for the
				      * first child.
				      *
				      * The direction parameter
				      * determines the type of
				      * rotation. It is one character
				      * of @p xXyYzZ. The character
				      * determines the axis of
				      * rotation, case determines the
				      * direction. Lower case is
				      * counter-clockwise seen in
				      * direction of the axis.
				      *
				      * Since rotation around the
				      * y-axis is not used, it is not
				      * implemented either.
				      */
    void rotate_indices (std::vector<unsigned int> &indices,
			 const char                 direction) const;
    
				     /**
				      * Allow access from other dimensions.
				      */
    template <int dim1, int spacedim1> friend class FE_DGQ;

				     /**
				      * Allows @p MappingQ class to
				      * access to build_renumbering
				      * function.
				      */
    template <int dim1, int spacedim1> friend class MappingQ;
};



/**
 * Implementation of scalar, discontinuous tensor product elements
 * based on Lagrange polynomials with arbitrary nodes.
 *
 * See base class documentation for details.
 *
 * @author F. Prill 2006
 */
template <int dim>
class FE_DGQArbitraryNodes : public FE_DGQ<dim>
{
  public:
				     /**
				      * Constructor for tensor product
				      * polynomials based on
				      * Polynomials::Lagrange
				      * interpolation of the support
				      * points in the quadrature rule
				      * <tt>points</tt>. The degree of
				      * these polynomials is
				      * <tt>points.size()-1</tt>.
				      */
    FE_DGQArbitraryNodes (const Quadrature<1>& points);

				     /**
				      * Return a string that uniquely
				      * identifies a finite
				      * element. This class returns
				      * <tt>FE_DGQArbitraryNodes<dim>(degree)</tt> ,
				      * with <tt>dim</tt> and <tt>degree</tt>
				      * replaced by appropriate
				      * values.
				      */
    virtual std::string get_name () const;

  protected:
				     /**
				      * @p clone function instead of
				      * a copy constructor.
				      *
				      * This function is needed by the
				      * constructors of @p FESystem.
				      */
    virtual FiniteElement<dim> *clone() const;
};


/*@}*/

DEAL_II_NAMESPACE_CLOSE

#endif
