//---------------------------------------------------------------------------
//    $Id: mg_dof_accessor.h 24157 2011-08-21 04:17:31Z bangerth $
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__mg_dof_accessor_h
#define __deal2__mg_dof_accessor_h


#include <deal.II/base/config.h>
#include <deal.II/dofs/dof_accessor.h>
#include <deal.II/multigrid/mg_dof_iterator_selector.h>

DEAL_II_NAMESPACE_OPEN

template <int dim, int spacedim> class MGDoFHandler;

template <int structdim, int dim, int spacedim=dim> class MGDoFAccessor;

namespace internal
{
  namespace MGDoFHandler
  {
    struct Implementation;
  }
}


/*!@addtogroup mg */
/*@{*/


namespace internal
{
  namespace MGDoFAccessor
  {
/**
 * This is a switch class which only declares a @p typedef. It is meant to
 * determine which class aMGDoFAccessor class is to be derived
 * from. By default, <tt>MGDoFAccessor<structdim,dim></tt> derives from
 * the @p typedef in the general <tt>MGDoFAccessor_Inheritance<structdim,dim></tt>
 * class, which is <tt>DoFAccessor<structdim,dim></tt>,
 * but if <tt>structdim==dim</tt>, then the specialization <tt>MGDoFAccessor_Inheritance<dim,dim></tt>
 * is used which declares its local type to be <tt>DoFCellAccessor<dim></tt>. Therefore,
 * the inheritance is automatically chosen to be from @p DoFCellAccessor if the
 * object under consideration has full dimension, i.e. constitutes a cell.
 *
 * @author Wolfgang Bangerth, 1999
 */
    template <int structdim, int dim, int spacedim>
    struct Inheritance
    {
					 /**
					  * Declaration of the @p typedef.
					  * See the full documentation for
					  * more information.
					  */
	typedef dealii::DoFAccessor<structdim, dealii::DoFHandler<dim,spacedim> > BaseClass;
    };


/**
 * This is a switch class which only declares a @p typwdef. It is
 * meant to determine which class a DoFAccessor class is to be
 * derived from. By default, DoFAccessor<structdim,dim>
 * derives from the @p typedef in the general
 * <tt>DoFAccessor_Inheritance<structdim,dim></tt> class, which is
 * <tt>TriaAccessor<structdim,dim></tt>, but if
 * <tt>structdim==dim</tt>, then the specialization
 * <tt>TriaAccessor<dim,dim></tt> is used which declares its
 * local type to be <tt>CellAccessor<dim></tt>. Therefore, the
 * inheritance is automatically chosen to be from @p CellAccessor if
 * the object under consideration has full dimension, i.e. constitutes
 * a cell.
 *
 * @author Wolfgang Bangerth, 1999
 */
    template <int dim, int spacedim>
    struct Inheritance<dim,dim,spacedim>
    {
					 /**
					  * Declaration of the @p typedef.
					  * See the full documentation for
					  * more information.
					  */
	typedef dealii::DoFCellAccessor<dealii::DoFHandler<dim,spacedim> > BaseClass;
    };
  }
}


/* -------------------------------------------------------------------------- */



/**
 * A class that gives access to the degrees of freedom stored in a
 * MGDoFHandler.  Accessors are used to, well, access the data that pertains
 * to edges, faces, and cells of a triangulation. The concept is explained in
 * more detail in connection to @ref Iterators.
 *
 * This class follows mainly the route laid out by the accessor library
 * declared in the triangulation library (TriaAccessor). It enables the user
 * to access the degrees of freedom on lines, quads, or hexes, as well as the
 * multigrid degrees of freedom associated with these objects. The first
 * template argument of this class determines the dimensionality of the object
 * under consideration: 1 for lines, 2 for quads, and 3 for hexes. The second
 * template argument determines the dimensionality of the triangulation to
 * which this object belongs, and the third the dimensionality of the space in
 * which it is embedded.
 *
 * Depending on whether the structural dimension of the object
 * accessed equals the dimension on which the DoF handler object
 * operates, this class is derived from CellAccessor or
 * TriaAccessor. This means that, for example accessors to quads
 * in 2d have access to all the mesh aspects of cells, whereas
 * accessors to quads in 3d can only access things that make sense for
 * faces.
 *
 *
 * <h3>Usage</h3>
 *
 * Usage is best to happen through the typedefs to the various kinds
 * of iterators provided by the MGDoFHandler class,
 * since they are more secure to changes in the class naming and
 * template interface as well as providing easier typing (much less
 * complicated names!).
 *
 *
 * <h3>Inheritance</h3>
 *
 * If the structural dimension given by the first template argument
 * equals the dimension of the MGDoFHandler (given as the second
 * template argument), then we are obviously dealing with cells,
 * rather than lower-dimensional objects. In that case, inheritance is
 * from DoFCellAccessor, to provide access to all the cell specific
 * information afforded by that class. Otherwise, i.e. for
 * lower-dimensional objects, inheritance is from DoFAccessor.
 *
 * There is a MGDoFCellAccessor class that provides the
 * equivalent to the CellAccessor and DoFCellAccessor classes.
 *
 * @ingroup dofs
 * @ingroup Accessors
 * @author Wolfgang Bangerth, 1998, 2006, 2008
 */
template <int structdim, int dim, int spacedim>
class MGDoFAccessor : public internal::MGDoFAccessor::Inheritance<structdim,dim,spacedim>::BaseClass
{
  public:
				     /**
				      * Declare a typedef to the base
				      * class to make accessing some
				      * of the exception classes
				      * simpler.
				      */
    typedef
    typename internal::MGDoFAccessor::Inheritance<structdim,dim,spacedim>::BaseClass
    BaseClass;

				     /**
				      * A typedef necessary for the
				      * iterator classes.
				      */
    typedef MGDoFHandler<dim,spacedim> AccessorData;

				     /**
				      * The dof handler type used by
				      * this accessor.
				      */
    typedef MGDoFHandler<dim, spacedim> Container;

				     /**
				      * @name Constructors
				      */
				     /**
				      * @{
				      */

				     /**
				      * Default constructor. Creates
				      * an object that is not
				      * usable.
				      */
    MGDoFAccessor ();

				     /**
				      * Constructor.
				      */
    MGDoFAccessor (const Triangulation<dim,spacedim> *tria,
		   const int                 level,
		   const int                 index,
		   const AccessorData       *local_data);

				     /**
				      * Conversion constructor. This
				      * constructor exists to make certain
				      * constructs simpler to write in
				      * dimension independent code. For
				      * example, it allows assigning a face
				      * iterator to a line iterator, an
				      * operation that is useful in 2d but
				      * doesn't make any sense in 3d. The
				      * constructor here exists for the
				      * purpose of making the code conform to
				      * C++ but it will unconditionally abort;
				      * in other words, assigning a face
				      * iterator to a line iterator is better
				      * put into an if-statement that checks
				      * that the dimension is two, and assign
				      * to a quad iterator in 3d (an operator
				      * that, without this constructor would
				      * be illegal if we happen to compile for
				      * 2d).
				      */
    template <int structdim2, int dim2, int spacedim2>
    MGDoFAccessor (const InvalidAccessor<structdim2,dim2,spacedim2> &);

				     /**
				      * Another conversion operator
				      * between objects that don't
				      * make sense, just like the
				      * previous one.
				      */
    template <int structdim2, int dim2, int spacedim2>
    MGDoFAccessor (const MGDoFAccessor<structdim2,dim2,spacedim2> &);

				     /**
				      * @}
				      */

				     /**
				      *  @name Accessing the DoF indices of this object
				      */
				     /**
				      * @{
				      */

    				     /**
				      * Return the indices of the dofs of this
				      * object in the standard ordering: dofs
				      * on vertex 0, dofs on vertex 1, ...
				      * dofs on line 0, dofs on line 1, ...,
				      * then quads, then hexes.
				      *
				      * It is assumed that the vector already
				      * has the right size beforehand. The
				      * indices refer to the local numbering
				      * for the level this line lives on.
				      */
    void get_mg_dof_indices (const int level,
			     std::vector<unsigned int> &dof_indices) const;

    				     /**
				      * Return the value of the given vector
				      * restricted to the dofs of this
				      * cell in the standard ordering: dofs
				      * on vertex 0, dofs on vertex 1, etc,
				      * dofs on line 0, dofs on line 1, etc,
				      * dofs on quad 0, etc.
				      *
				      * It is assumed that the vector already
				      * has the right size beforehand. The
				      * indices refer to the multilevel
				      * numbering local to the present
				      * level of this cell. The vector shall
				      * therefore have the same number of
				      * entries as there are degrees of
				      * freedom on this level.
				      */
    template <typename number>
    void get_mg_dof_values (const int level,
			    const Vector<number> &values,
			    Vector<number>       &dof_values) const;

				     /**
				      * Return the index of the @p ith
				      * degree on the @p vertexth
				      * vertex for the level this
				      * object lives on.
				      */
    unsigned int mg_vertex_dof_index (const int level,
				      const unsigned int vertex,
				      const unsigned int i) const;
//TODO: This function is broken for faces
				     /**
				      * @todo This function is broken
				      * for faces.
				      *
				      * Return the index of the @p ith
				      * degree of freedom of this line
				      * on the level this line lives
				      * on.
				      */
    unsigned int mg_dof_index (const int level,
			       const unsigned int i) const;

				     /**
				      * @}
				      */

				     /**
				      *  @name Accessing sub-objects
				      */
				     /**
				      * @{
				      */

				     /**
				      * Return an iterator to the @p
				      * i-th child.
				      */
    TriaIterator<MGDoFAccessor<structdim,dim,spacedim> >
    child (const unsigned int) const;

    				     /**
				      * Return a pointer to the @p ith line
				      * bounding this @p Hex.
				      */
    typename internal::MGDoFHandler::Iterators<dim,spacedim>::line_iterator
    line (const unsigned int i) const;

				     /**
				      * Return a pointer to the @p ith quad
				      * bounding this @p Hex.
				      */
    typename internal::MGDoFHandler::Iterators<dim,spacedim>::quad_iterator
    quad (const unsigned int i) const;

				     /**
				      * @}
				      */

				     /**
				      * Return a handle on the
				      * DoFHandler object which we
				      * are using.
				      */
    const MGDoFHandler<dim,spacedim> &
    get_mg_dof_handler () const;

				     /**
				      * Return an iterator pointing to
				      * the the parent.
				      */
    TriaIterator<MGDoFAccessor<structdim,dim,spacedim> >
    parent () const;

				     /**
				      * Implement the copy operator needed
				      * for the iterator classes.
				      */
    void copy_from (const MGDoFAccessor &a);

    				     /**
				      * Exception for child classes
				      */
    DeclException0 (ExcInvalidObject);

  protected:
				     /**
				      * Store the address of the @p MGDoFHandler object
				      * to be accessed.
				      */
    MGDoFHandler<dim,spacedim> *mg_dof_handler;

				     /**
				      * Reset the DoF handler pointer.
				      */
    void set_mg_dof_handler (MGDoFHandler<dim,spacedim> *dh);

				     /**
				      * Set the index of the @p ith degree
				      * on the @p vertexth vertex to @p index
				      * for the level this object lives on.
				      */
    void set_mg_vertex_dof_index (const int level,
				  const unsigned int vertex,
				  const unsigned int i,
				  const unsigned int index) const;

    				     /**
				      * Set the index of the @p ith degree
				      * of freedom of this line on the
				      * level this line lives on to @p index.
				      */
    void set_mg_dof_index (const int level,
			   const unsigned int i,
			   const unsigned int index) const;

  private:
    				     /**
				      *  Copy operator. This is normally used
				      *  in a context like <tt>iterator a,b;
				      *  *a=*b;</tt>. Presumably, the intent
				      *  here is to copy the object pointed to
				      *  by @p b to the object pointed to by
				      *  @p a. However, the result of
				      *  dereferencing an iterator is not an
				      *  object but an accessor; consequently,
				      *  this operation is not useful for
				      *  iterators on triangulations. We
				      *  declare this function here private,
				      *  thus it may not be used from outside.
				      *  Furthermore it is not implemented and
				      *  will give a linker error if used
				      *  anyway.
				      */
    MGDoFAccessor &
    operator = (const MGDoFAccessor &da);

    template <int, int> friend class MGDoFHandler;
    friend struct internal::MGDoFHandler::Implementation;
};



/**
 * Specialization of the general MGDoFAccessor class template for the
 * case of zero-dimensional objects (a vertex) that are the face of a
 * one-dimensional cell in spacedim space dimensions. Since vertices
 * function differently than general faces, this class does a few
 * things differently than the general template, but the interface
 * should look the same.
 *
 * @author Wolfgang Bangerth, 2010
 */
template <int spacedim>
class MGDoFAccessor<0,1,spacedim> : public internal::MGDoFAccessor::Inheritance<0,1,spacedim>::BaseClass
{
  public:
				     /**
				      * Declare a typedef to the base
				      * class to make accessing some
				      * of the exception classes
				      * simpler.
				      */
    typedef
    typename internal::MGDoFAccessor::Inheritance<0,1,spacedim>::BaseClass
    BaseClass;

				     /**
				      * A typedef necessary for the
				      * iterator classes.
				      */
    typedef MGDoFHandler<1,spacedim> AccessorData;

				     /**
				      * The dof handler type used by
				      * this accessor.
				      */
    typedef MGDoFHandler<1, spacedim> Container;

				     /**
				      * @name Constructors
				      */
				     /**
				      * @{
				      */

				     /**
				      * Default constructor. Provides
				      * an accessor that can't be
				      * used.
				      */
    MGDoFAccessor ();

				     /**
				      * Constructor to be used if the
				      * object here refers to a vertex
				      * of a one-dimensional
				      * triangulation, i.e. a face of
				      * the triangulation.
				      *
				      * Since there is no mapping from
				      * vertices to cells, an accessor
				      * object for a point has no way
				      * to figure out whether it is at
				      * the boundary of the domain or
				      * not. Consequently, the second
				      * argument must be passed by the
				      * object that generates this
				      * accessor -- e.g. a 1d cell
				      * that can figure out whether
				      * its left or right vertex are
				      * at the boundary.
				      *
				      * The third argument is the
				      * global index of the vertex we
				      * point to.
				      *
				      * The fourth argument is a
				      * pointer to the MGDoFHandler
				      * object.
				      *
				      * This iterator can only be
				      * called for one-dimensional
				      * triangulations.
				      */
    MGDoFAccessor (const Triangulation<1,spacedim> * tria,
		   const typename TriaAccessor<0,1,spacedim>::VertexKind vertex_kind,
		   const unsigned int    vertex_index,
		   const MGDoFHandler<1,spacedim> * dof_handler);

				     /**
				      * Constructor. This constructor
				      * exists in order to maintain
				      * interface compatibility with
				      * the other accessor
				      * classes. However, it doesn't
				      * do anything useful here and so
				      * may not actually be called.
				      */
    MGDoFAccessor (const Triangulation<1,spacedim> * =  0,
		   const int = 0,
		   const int = 0,
		   const MGDoFHandler<1,spacedim> * = 0);

				     /**
				      * Conversion constructor. This
				      * constructor exists to make certain
				      * constructs simpler to write in
				      * dimension independent code. For
				      * example, it allows assigning a face
				      * iterator to a line iterator, an
				      * operation that is useful in 2d but
				      * doesn't make any sense in 3d. The
				      * constructor here exists for the
				      * purpose of making the code conform to
				      * C++ but it will unconditionally abort;
				      * in other words, assigning a face
				      * iterator to a line iterator is better
				      * put into an if-statement that checks
				      * that the dimension is two, and assign
				      * to a quad iterator in 3d (an operator
				      * that, without this constructor would
				      * be illegal if we happen to compile for
				      * 2d).
				      */
    template <int structdim2, int dim2, int spacedim2>
    MGDoFAccessor (const InvalidAccessor<structdim2,dim2,spacedim2> &);

				     /**
				      * Another conversion operator
				      * between objects that don't
				      * make sense, just like the
				      * previous one.
				      */
    template <int structdim2, int dim2, int spacedim2>
    MGDoFAccessor (const MGDoFAccessor<structdim2, dim2, spacedim2> &);


				     /**
				      * Another conversion operator
				      * between objects that don't
				      * make sense, just like the
				      * previous one.
				      */
    template <int dim2, int spacedim2>
    MGDoFAccessor (const MGDoFCellAccessor<dim2, spacedim2> &);

  protected:
				     /**
				      * Store the address of the @p MGDoFHandler object
				      * to be accessed.
				      */
    MGDoFHandler<1,spacedim> *mg_dof_handler;
};



/* -------------------------------------------------------------------------- */



/**
 * Grant access to the degrees of freedom on cells, as far as this
 * isn't already covered by the MGDoFAccessor and MGDoFAccessor
 * classes from which the present class is derived. In particular,
 * this function overloads functions from CellAccessor and
 * DoFCellAccessor that return iterators to other objects, such as the
 * face() or neighbor() function. Since the functions in CellAccessor
 * and DoFCellAccessor return iterators into Triangulations and
 * DoFHandlers only, we need to reimplement the functions in this
 * class to make sure we get iterators into MGDoFHandlers instead.
 *
 * @ingroup mg
 * @ingroup Accessors
 * @author Wolfgang Bangerth, 1998, 2006
 */
template <int dim, int spacedim=dim>
class MGDoFCellAccessor :  public MGDoFAccessor<dim,dim,spacedim>
{
  public:
    typedef MGDoFAccessor<dim,dim,spacedim> BaseClass;

				     /**
				      * Type of faces.
				      */
    typedef
    TriaIterator< MGDoFAccessor<dim-1,dim,spacedim> >
    face_iterator;
				     /**
				      * Declare the data type that this accessor
				      * class expects to get passed from the
				      * iterator classes.
				      */
    typedef typename MGDoFAccessor<dim,dim,spacedim>::AccessorData  AccessorData;

				     /**
				      * @name Constructors
				      */
				     /**
				      * @{
				      */

    				     /**
				      * Constructor
				      */
    MGDoFCellAccessor (const Triangulation<dim,spacedim> *tria,
		       const int                 level,
		       const int                 index,
		       const AccessorData       *local_data);

				     /**
				      * Conversion constructor. This
				      * constructor exists to make certain
				      * constructs simpler to write in
				      * dimension independent code. For
				      * example, it allows assigning a face
				      * iterator to a line iterator, an
				      * operation that is useful in 2d but
				      * doesn't make any sense in 3d. The
				      * constructor here exists for the
				      * purpose of making the code conform to
				      * C++ but it will unconditionally abort;
				      * in other words, assigning a face
				      * iterator to a line iterator is better
				      * put into an if-statement that checks
				      * that the dimension is two, and assign
				      * to a quad iterator in 3d (an operator
				      * that, without this constructor would
				      * be illegal if we happen to compile for
				      * 2d).
				      */
    template <int structdim2, int dim2, int spacedim2>
    MGDoFCellAccessor (const InvalidAccessor<structdim2,dim2,spacedim2> &);

				     /**
				      * Another conversion operator
				      * between objects that don't
				      * make sense, just like the
				      * previous one.
				      */
    template <int dim2, class DH2>
    MGDoFCellAccessor (const DoFAccessor<dim2, DH2> &);

				     /**
				      * @}
				      */

				     /**
				      *  @name Accessing the DoF indices of this object
				      */
				     /**
				      * @{
				      */

    				     /**
				      * Return the indices of the dofs of this
				      * hex in the standard ordering: dofs
				      * on vertex 0, dofs on vertex 1, etc,
				      * dofs on line 0, dofs on line 1, etc,
				      * dofs on quad 0, etc.
				      *
				      * It is assumed that the vector already
				      * has the right size beforehand. The
				      * indices refer to the local numbering
				      * for the level this hex lives on.
				      */
    void get_mg_dof_indices (std::vector<unsigned int> &dof_indices) const;

    				     /**
				      * Return the value of the given vector
				      * restricted to the dofs of this
				      * cell in the standard ordering: dofs
				      * on vertex 0, dofs on vertex 1, etc,
				      * dofs on line 0, dofs on line 1, etc,
				      * dofs on quad 0, etc.
				      *
				      * It is assumed that the vector already
				      * has the right size beforehand. The
				      * indices refer to the multilevel
				      * numbering local to the present
				      * level of this cell. The vector shall
				      * therefore have the same number of
				      * entries as there are degrees of
				      * freedom on this level.
				      */
    template <typename number>
    void get_mg_dof_values (const Vector<number> &values,
			    Vector<number>       &dof_values) const;

				     /**
				      * @}
				      */

				     /**
				      *  @name Accessing sub-objects and neighbors
				      */

				     /**
				      * Return the @p ith neighbor as a MGDoF cell
				      * iterator. This function is needed since
				      * the neighbor function of the base
				      * class returns a cell accessor without
				      * access to the MGDoF data.
				      */
    TriaIterator<MGDoFCellAccessor<dim,spacedim> >
    neighbor (const unsigned int) const;

    				     /**
				      * Return the @p ith child as a MGDoF cell
				      * iterator. This function is needed since
				      * the child function of the base
				      * class returns a cell accessor without
				      * access to the DoF data.
				      */
    TriaIterator<MGDoFCellAccessor<dim,spacedim> >
    child (const unsigned int) const;

    				     /**
				      * Return an iterator to the @p ith face
				      * of this cell.
				      *
				      * This function is not implemented in 1D,
				      * and maps to MGDoFAccessor<2, dim>::line in 2D.
				      */
    typename internal::MGDoFHandler::Iterators<dim,spacedim>::face_iterator
    face (const unsigned int i) const;

				     /**
				      * Return the result of the
				      * @p neighbor_child_on_subface
				      * function of the base class,
				      * but convert it so that one can
				      * also access the MGDoF data (the
				      * function in the base class
				      * only returns an iterator with
				      * access to the triangulation
				      * data).
				      */
    typename internal::MGDoFHandler::Iterators<dim,spacedim>::cell_iterator
    neighbor_child_on_subface (const unsigned int face_no,
                               const unsigned int subface_no) const;

				     /**
				      * @}
				      */

    				 /**
				      * Return the parent as a MGDoF cell
				      * iterator. This function is needed since
				      * the parent function of the base
				      * class returns a cell accessor without
				      * access to the DoF data.
				      */
    TriaIterator<MGDoFCellAccessor<dim,spacedim> >
    parent () const;
};

/*@}*/

// ------------------- template and inline functions -------------

#ifndef DOXYGEN

template <int structdim, int dim, int spacedim>
template <int structdim2, int dim2, int spacedim2>
inline
MGDoFAccessor<structdim,dim,spacedim>::
MGDoFAccessor (const InvalidAccessor<structdim2,dim2,spacedim2> &)
		:
		BaseClass (0, -1, -1, 0)
{
  Assert (false, ExcInvalidObject());
}



template <int structdim, int dim, int spacedim>
template <int structdim2, int dim2, int spacedim2>
inline
MGDoFAccessor<structdim,dim,spacedim>::
MGDoFAccessor (const MGDoFAccessor<structdim2,dim2,spacedim2> &)
		:
		BaseClass (0, -1, -1, 0)
{
  Assert (false, ExcInvalidObject());
}



template <int dim, int spacedim>
template <int structdim2, int dim2, int spacedim2>
inline
MGDoFCellAccessor<dim,spacedim>::
MGDoFCellAccessor (const InvalidAccessor<structdim2,dim2,spacedim2> &)
		:
		BaseClass (0, -1, -1, 0)
{
  Assert (false, typename BaseClass::ExcInvalidObject());
}



template <int dim, int spacedim>
template <int dim2, class DH>
inline
MGDoFCellAccessor<dim,spacedim>::
MGDoFCellAccessor (const DoFAccessor<dim2,DH> &)
		:
		BaseClass (0, -1, -1, 0)
{
  Assert (false, typename BaseClass::ExcInvalidObject());
}


#endif



DEAL_II_NAMESPACE_CLOSE

#endif
