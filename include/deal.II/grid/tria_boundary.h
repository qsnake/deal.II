//---------------------------------------------------------------------------
//    $Id: tria_boundary.h 24197 2011-08-25 15:51:59Z bangerth $
//
//    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__tria_boundary_h
#define __deal2__tria_boundary_h


/*----------------------------   boundary-function.h     ---------------------------*/

#include <deal.II/base/config.h>
#include <deal.II/base/subscriptor.h>
#include <deal.II/base/point.h>
#include <deal.II/grid/tria.h>

DEAL_II_NAMESPACE_OPEN

template <int dim, int space_dim> class Triangulation;



/**
 *   This class is used to represent a boundary to a triangulation.
 *   When a triangulation creates a new vertex on the boundary of the
 *   domain, it determines the new vertex' coordinates through the
 *   following code (here in two dimensions):
 *   @verbatim
 *     ...
 *     Point<2> new_vertex = boundary.get_new_point_on_line (line);
 *     ...
 *   @endverbatim
 *   @p line denotes the line at the boundary that shall be refined
 *   and for which we seek the common point of the two child lines.
 *
 *   In 3D, a new vertex may be placed on the middle of a line or on
 *   the middle of a side. Respectively, the library calls
 *   @verbatim
 *     ...
 *     Point<3> new_line_vertices[4]
 *       = { boundary.get_new_point_on_line (face->line(0)),
 *           boundary.get_new_point_on_line (face->line(1)),
 *           boundary.get_new_point_on_line (face->line(2)),
 *           boundary.get_new_point_on_line (face->line(3))  };
 *     ...
 *   @endverbatim
 *   to get the four midpoints of the lines bounding the quad at the
 *   boundary, and after that
 *   @verbatim
 *     ...
 *     Point<3> new_quad_vertex = boundary.get_new_point_on_quad (face);
 *     ...
 *   @endverbatim
 *   to get the midpoint of the face. It is guaranteed that this order
 *   (first lines, then faces) holds, so you can use information from
 *   the children of the four lines of a face, since these already exist
 *   at the time the midpoint of the face is to be computed.
 *   
 *   Since iterators are passed to the functions, you may use information
 *   about boundary indicators and the like, as well as all other information
 *   provided by these objects.
 *
 *   There are specialisations, StraightBoundary, which places
 *   the new point right into the middle of the given points, and
 *   HyperBallBoundary creating a hyperball with given radius
 *   around a given center point.
 *
 * @ingroup boundary
 * @author Wolfgang Bangerth, 1999, 2001, 2009, Ralf Hartmann, 2001, 2008
 */
template <int dim, int spacedim=dim>
class Boundary : public Subscriptor
{
  public:

				     /**
				      * Type keeping information about
				      * the normals at the vertices of
				      * a face of a cell. Thus, there
				      * are
				      * <tt>GeometryInfo<dim>::vertices_per_face</tt>
				      * normal vectors, that define
				      * the tangent spaces of the
				      * boundary at the vertices. Note
				      * that the vectors stored in
				      * this object are not required
				      * to be normalized, nor to
				      * actually point outward, as one
				      * often will only want to check
				      * for orthogonality to define
				      * the tangent plane; if a
				      * function requires the normals
				      * to be normalized, then it must
				      * do so itself.
				      *
				      * For obvious reasons, this
				      * type is not useful in 1d.
				      */
    typedef Tensor<1,spacedim> FaceVertexNormals[GeometryInfo<dim>::vertices_per_face];
	
				     /**
				      * Destructor. Does nothing here, but
				      * needs to be declared to make it
				      * virtual.
				      */
    virtual ~Boundary ();

				     /**
				      * Return the point which shall
				      * become the new middle vertex
				      * of the two children of a
				      * regular line. In 2D, this line
				      * is a line at the boundary,
				      * while in 3d, it is bounding a
				      * face at the boundary (the
				      * lines therefore is also on the
				      * boundary).
				      */
    virtual 
    Point<spacedim>
    get_new_point_on_line (const typename Triangulation<dim,spacedim>::line_iterator &line) const = 0;

				     /**
				      * Return the point which shall
				      * become the common point of the
				      * four children of a quad at the
				      * boundary in three or more
				      * spatial dimensions. This
				      * function therefore is only
				      * useful in at least three
				      * dimensions and should not be
				      * called for lower dimensions.
				      *
				      * This function is called after
				      * the four lines bounding the
				      * given @p quad are refined, so
				      * you may want to use the
				      * information provided by
				      * <tt>quad->line(i)->child(j)</tt>,
				      * <tt>i=0...3</tt>, <tt>j=0,1</tt>.
				      *
				      * Because in 2D, this function
				      * is not needed, it is not made
				      * pure virtual, to avoid the
				      * need to overload it.  The
				      * default implementation throws
				      * an error in any case, however.
				      */
    virtual 
    Point<spacedim>
    get_new_point_on_quad (const typename Triangulation<dim,spacedim>::quad_iterator &quad) const;

				     /**
				      * Depending on <tt>dim=2</tt> or
				      * <tt>dim=3</tt> this function
				      * calls the
				      * get_new_point_on_line or the
				      * get_new_point_on_quad
				      * function. It throws an
				      * exception for
				      * <tt>dim=1</tt>. This wrapper
				      * allows dimension independent
				      * programming.
				      */
    Point<spacedim>
    get_new_point_on_face (const typename Triangulation<dim,spacedim>::face_iterator &face) const;

				     /**
				      * Return equally spaced
				      * intermediate points on a line.
                                      *
				      * The number of points requested
				      * is given by the size of the
				      * vector @p points. It is the
				      * task of the derived classes to
				      * arrange the points in
				      * approximately equal distances.
				      *
				      * This function is called by the
				      * @p MappingQ class. This
				      * happens on each face line of a
				      * cells that has got at least
				      * one boundary line.
				      *
				      * As this function is not needed
				      * for @p MappingQ1, it is not
				      * made pure virtual, to avoid
				      * the need to overload it.  The
				      * default implementation throws
				      * an error in any case, however.
				      */
    virtual
    void
    get_intermediate_points_on_line (const typename Triangulation<dim,spacedim>::line_iterator &line,
				     std::vector<Point<spacedim> > &points) const;
    
				     /**
				      * Return equally spaced
				      * intermediate points on a
				      * boundary quad.
				      *
				      * The number of points requested
				      * is given by the size of the
				      * vector @p points. It is
				      * required that this number is a
				      * square of another integer,
				      * i.e. <tt>n=points.size()=m*m</tt>. It
				      * is the task of the derived
				      * classes to arrange the points
				      * such they split the quad into
				      * <tt>(m+1)(m+1)</tt> approximately
				      * equal-sized subquads.
				      *
				      * This function is called by the
				      * <tt>MappingQ<3></tt> class. This
				      * happens each face quad of
				      * cells in 3d that has got at
				      * least one boundary face quad.
				      *
				      * As this function is not needed
				      * for @p MappingQ1, it is not
				      * made pure virtual, to avoid
				      * the need to overload it.  The
				      * default implementation throws
				      * an error in any case, however.
				      */
    virtual
    void
    get_intermediate_points_on_quad (const typename Triangulation<dim,spacedim>::quad_iterator &quad,
				     std::vector<Point<spacedim> > &points) const;

				     /**
				      * Depending on <tt>dim=2</tt> or
				      * <tt>dim=3</tt> this function
				      * calls the
				      * get_intermediate_points_on_line
				      * or the
				      * get_intermediate_points_on_quad
				      * function. It throws an
				      * exception for
				      * <tt>dim=1</tt>. This wrapper
				      * allows dimension independent
				      * programming.
				      */
    void
    get_intermediate_points_on_face (const typename Triangulation<dim,spacedim>::face_iterator &face,
				     std::vector<Point<spacedim> > &points) const;

				     /**
				      * Return the normal vector to the surface
				      * at the point p. If p is not in fact
				      * on the surface, but only closeby,
				      * try to return something reasonable,
				      * for example the normal vector
				      * at the surface point closest to p.
				      * (The point p will in fact not normally
				      * lie on the actual surface, but rather
				      * be a quadrature point mapped by some
				      * polynomial mapping; the mapped surface,
				      * however, will not usually coincide with
				      * the actual surface.)
				      * 
				      * The face iterator gives an indication
				      * which face this function is supposed
				      * to compute the normal vector for.
				      * This is useful if the boundary of
				      * the domain is composed of different
				      * nondifferential pieces (for example
				      * when using the StraightBoundary class
				      * to approximate a geometry that is
				      * completely described by the coarse mesh,
				      * with piecewise (bi-)linear components
				      * between the vertices, but where the
				      * boundary may have a kink at the vertices
				      * itself).
				      * 
				      * @note Implementations of this function 
				      * should be able to assume that the point p
				      * lies within or close to the face described by the
				      * first argument. In turn, callers of this
				      * function should ensure that this is
				      * in fact the case.
				      */
    virtual
    Tensor<1,spacedim>
    normal_vector (const typename Triangulation<dim,spacedim>::face_iterator &face,
		   const Point<spacedim> &p) const;
    
				     /**
				      * Compute the normal vectors to
				      * the boundary at each vertex of
				      * the given face. It is not
				      * required that the normal
				      * vectors be normed
				      * somehow. Neither is it
				      * required that the normals
				      * actually point outward.
				      *
				      * This function is
				      * needed to compute data for C1
				      * mappings. The default
				      * implementation is to throw an
				      * error, so you need not
				      * overload this function in case
				      * you do not intend to use C1
				      * mappings.
				      *
				      * Note that when computing
				      * normal vectors at a vertex
				      * where the boundary is not
				      * differentiable, you have to
				      * make sure that you compute the
				      * one-sided limits, i.e. limit
				      * with respect to points inside
				      * the given face.
				      */
    virtual
    void
    get_normals_at_vertices (const typename Triangulation<dim,spacedim>::face_iterator &face,
			     FaceVertexNormals &face_vertex_normals) const;

				     /**
				      * Given a candidate point and a
				      * line segment characterized by
				      * the iterator, return a point
				      * that lies on the surface
				      * described by this object. This
				      * function is used in some mesh
				      * smoothing algorithms that try
				      * to move around points in order
				      * to improve the mesh quality
				      * but need to ensure that points
				      * that were on the boundary
				      * remain on the boundary.
				      *
				      * If spacedim==1, then the line
				      * represented by the line
				      * iterator is the entire space
				      * (i.e. it is a cell, not a part
				      * of the boundary), and the
				      * returned point equals the
				      * given input point.
				      *
				      * Derived classes do not need to
				      * implement this function unless
				      * mesh smoothing algorithms are
				      * used with a particular
				      * boundary object. The default
				      * implementation of this
				      * function throws an exception
				      * of type ExcPureFunctionCalled.
				      */
    virtual
    Point<spacedim>
    project_to_surface (const typename Triangulation<dim,spacedim>::line_iterator &line,
			const Point<spacedim> &candidate) const;

				     /**
				      * Same function as above but for
				      * a point that is to be
				      * projected onto the area
				      * characterized by the given
				      * quad.
				      *
				      * If spacedim<=2, then the surface
				      * represented by the quad
				      * iterator is the entire space
				      * (i.e. it is a cell, not a part
				      * of the boundary), and the
				      * returned point equals the
				      * given input point.
				      */
    virtual
    Point<spacedim>
    project_to_surface (const typename Triangulation<dim,spacedim>::quad_iterator &quad,
			const Point<spacedim> &candidate) const;

				     /**
				      * Same function as above but for
				      * a point that is to be
				      * projected onto the area
				      * characterized by the given
				      * quad.
				      *
				      * If spacedim<=3, then the manifold
				      * represented by the hex
				      * iterator is the entire space
				      * (i.e. it is a cell, not a part
				      * of the boundary), and the
				      * returned point equals the
				      * given input point.
				      */
    virtual
    Point<spacedim>
    project_to_surface (const typename Triangulation<dim,spacedim>::hex_iterator &hex,
			const Point<spacedim> &candidate) const;
};



/**
 *   Specialization of Boundary<dim,spacedim>, which places the new point
 *   right into the middle of the given points. The middle is defined
 *   as the arithmetic mean of the points.
 *
 *   This class does not really describe a boundary in the usual
 *   sense. By placing new points in the middle of old ones, it rather
 *   assumes that the boundary of the domain is given by the
 *   polygon/polyhedron defined by the boundary of the initial coarse
 *   triangulation.
 *
 *   @ingroup boundary
 *
 *   @author Wolfgang Bangerth, 1998, 2001, Ralf Hartmann, 2001
 */
template <int dim, int spacedim=dim>
class StraightBoundary : public Boundary<dim,spacedim>
{
  public:
				     /**
				      * Default constructor. Some
				      * compilers require this for
				      * some reasons.
				      */
    StraightBoundary ();
    
				     /**
				      * Let the new point be the
				      * arithmetic mean of the two
				      * vertices of the line.
				      *
				      * Refer to the general
				      * documentation of this class
				      * and the documentation of the
				      * base class for more
				      * information.
				      */
    virtual Point<spacedim>
    get_new_point_on_line (const typename Triangulation<dim,spacedim>::line_iterator &line) const;

				     /**
				      * Let the new point be the
				      * arithmetic mean of the four
				      * vertices of this quad and the
				      * four midpoints of the lines,
				      * which are already created at
				      * the time of calling this
				      * function.
				      *
				      * Refer to the general
				      * documentation of this class
				      * and the documentation of the
				      * base class for more
				      * information.
				      */
    virtual
    Point<spacedim>
    get_new_point_on_quad (const typename Triangulation<dim,spacedim>::quad_iterator &quad) const;

				     /**
				      * Gives <tt>n=points.size()</tt>
				      * points that splits the
				      * StraightBoundary line into
				      * $n+1$ partitions of equal
				      * lengths.
				      *
				      * Refer to the general
				      * documentation of this class
				      * and the documentation of the
				      * base class.
				      */
    virtual
    void
    get_intermediate_points_on_line (const typename Triangulation<dim,spacedim>::line_iterator &line,
				     std::vector<Point<spacedim> > &points) const;

				     /**
				      * Gives <tt>n=points.size()=m*m</tt>
				      * points that splits the
				      * StraightBoundary quad into
				      * $(m+1)(m+1)$ subquads of equal
				      * size.
				      *
				      * Refer to the general
				      * documentation of this class
				      * and the documentation of the
				      * base class.
				      */
    virtual
    void
    get_intermediate_points_on_quad (const typename Triangulation<dim,spacedim>::quad_iterator &quad,
				     std::vector<Point<spacedim> > &points) const;

				     /**
				      * Implementation of the function
				      * declared in the base class.
				      *
				      * Refer to the general
				      * documentation of this class
				      * and the documentation of the
				      * base class.
				      */
    virtual
    Tensor<1,spacedim>
    normal_vector (const typename Triangulation<dim,spacedim>::face_iterator &face,
		   const Point<spacedim> &p) const;

		                     /**
				      * Compute the normals to the
				      * boundary at the vertices of
				      * the given face.
				      *
				      * Refer to the general
				      * documentation of this class
				      * and the documentation of the
				      * base class.
				      */
    virtual
    void
    get_normals_at_vertices (const typename Triangulation<dim,spacedim>::face_iterator &face,
			     typename Boundary<dim,spacedim>::FaceVertexNormals &face_vertex_normals) const;

				     /**
				      * Given a candidate point and a
				      * line segment characterized by
				      * the iterator, return a point
				      * that lies on the surface
				      * described by this object. This
				      * function is used in some mesh
				      * smoothing algorithms that try
				      * to move around points in order
				      * to improve the mesh quality
				      * but need to ensure that points
				      * that were on the boundary
				      * remain on the boundary.
				      *
				      * The point returned is the
				      * projection of the candidate
				      * point onto the line through
				      * the two vertices of the given
				      * line iterator.
				      *
				      * If spacedim==1, then the line
				      * represented by the line
				      * iterator is the entire space
				      * (i.e. it is a cell, not a part
				      * of the boundary), and the
				      * returned point equals the
				      * given input point.
				      */
    virtual
    Point<spacedim>
    project_to_surface (const typename Triangulation<dim,spacedim>::line_iterator &line,
			const Point<spacedim> &candidate) const;

				     /**
				      * Same function as above but for
				      * a point that is to be
				      * projected onto the area
				      * characterized by the given
				      * quad.
				      *
				      * The point returned is the
				      * projection of the candidate
				      * point onto the bilinear
				      * surface spanned by the four
				      * vertices of the given quad
				      * iterator.
				      *
				      * If spacedim<=2, then the surface
				      * represented by the quad
				      * iterator is the entire space
				      * (i.e. it is a cell, not a part
				      * of the boundary), and the
				      * returned point equals the
				      * given input point.
				      */
    virtual
    Point<spacedim>
    project_to_surface (const typename Triangulation<dim,spacedim>::quad_iterator &quad,
			const Point<spacedim> &candidate) const;

				     /**
				      * Same function as above but for
				      * a point that is to be
				      * projected onto the area
				      * characterized by the given
				      * quad.
				      *
				      * The point returned is the
				      * projection of the candidate
				      * point onto the trilinear
				      * manifold spanned by the eight
				      * vertices of the given hex
				      * iterator.
				      *
				      * If spacedim<=3, then the manifold
				      * represented by the hex
				      * iterator is the entire space
				      * (i.e. it is a cell, not a part
				      * of the boundary), and the
				      * returned point equals the
				      * given input point.
				      */
    virtual
    Point<spacedim>
    project_to_surface (const typename Triangulation<dim,spacedim>::hex_iterator &hex,
			const Point<spacedim> &candidate) const;
};



/* -------------- declaration of explicit specializations ------------- */

#ifndef DOXYGEN

template <>
Point<1>
Boundary<1,1>::
get_new_point_on_face (const Triangulation<1,1>::face_iterator &) const;

template <>
void
Boundary<1,1>::
get_intermediate_points_on_face (const Triangulation<1,1>::face_iterator &,
				 std::vector<Point<1> > &) const;

template <>
Point<2>
Boundary<1,2>::
get_new_point_on_face (const Triangulation<1,2>::face_iterator &) const;

template <>
void
Boundary<1,2>::
get_intermediate_points_on_face (const Triangulation<1,2>::face_iterator &,
				 std::vector<Point<2> > &) const;


template <>
void
StraightBoundary<1,1>::
get_normals_at_vertices (const Triangulation<1,1>::face_iterator &,
			 Boundary<1,1>::FaceVertexNormals &) const;
template <>
void
StraightBoundary<2,2>::
get_normals_at_vertices (const Triangulation<2,2>::face_iterator &face,
			 Boundary<2,2>::FaceVertexNormals &face_vertex_normals) const;
template <>
void
StraightBoundary<3,3>::
get_normals_at_vertices (const Triangulation<3,3>::face_iterator &face,
			 Boundary<3,3>::FaceVertexNormals &face_vertex_normals) const;

template <>
Point<3>
StraightBoundary<3,3>::
get_new_point_on_quad (const Triangulation<3,3>::quad_iterator &quad) const;

template <>
void
StraightBoundary<1,1>::
get_intermediate_points_on_line (const Triangulation<1,1>::line_iterator &,
				 std::vector<Point<1> > &) const;

template <>
void
StraightBoundary<3,3>::
get_intermediate_points_on_quad (const Triangulation<3,3>::quad_iterator &quad,
				 std::vector<Point<3> > &points) const;

#endif // DOXYGEN

DEAL_II_NAMESPACE_CLOSE

#endif
