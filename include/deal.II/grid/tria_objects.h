//---------------------------------------------------------------------------
//    $Id: tria_objects.h 23876 2011-06-28 18:21:51Z kanschat $
//
//    Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__tria_objects_h
#define __deal2__tria_objects_h

#include <deal.II/base/config.h>
#include <deal.II/base/exceptions.h>
#include <deal.II/base/geometry_info.h>
#include <deal.II/grid/tria_object.h>

#include <vector>

DEAL_II_NAMESPACE_OPEN

//TODO: See if we can unify the class hierarchy a bit using partial
// specialization of classes here, e.g. declare a general class
// TriaObjects<dim,G>.
// To consider for this: in that case we would have to duplicate quite a few
// things, e.g. TriaObjectsHex is derived from TriaObjects<Hexahedron> and
// declares mainly additional data. This would have to be changed in case of a
// partial specialization.

//TODO: The TriaObjects class contains a std::vector<G>. This is only an
//efficient storage scheme if G is relatively well packed, i.e. it's not a
//bool and then an integer and then a double, etc. Verify that this is
//actually the case.

template <int dim, int spacedim> class Triangulation;


namespace internal
{
  namespace Triangulation
  {

/**
 * General template for information belonging to the geometrical objects of a
 * triangulation, i.e. lines, quads, hexahedra...  Apart from the vector of
 * objects additional information is included, namely vectors indicating the
 * children, the used-status, user-flags, material-ids..
 *
 * Objects of these classes are included in the TriaLevel and TriaFaces
 * classes.
 *
 * @author Tobias Leicht, Guido Kanschat, 2006, 2007
 */
    
    template <typename G>
    class TriaObjects
    {
      public:
					 /**
					  * Constructor resetting some data.
					  */
	TriaObjects();
	
					 /**
					  *  Vector of the objects belonging to
					  *  this level. The index of the object
					  *  equals the index in this container. 
					  */
	std::vector<G> cells;
					 /**
					  *  Index of the even children of an object.
					  *  Since when objects are refined, all
					  *  children are created at the same
					  *  time, they are appended to the list
					  *  at least in pairs after each other.
					  *  We therefore only store the index
					  *  of the even children, the uneven
					  *  follow immediately afterwards.
					  *
					  *  If an object has no children, -1 is
					  *  stored in this list. An object is
					  *  called active if it has no
					  *  children. The function
					  *  TriaAccessorBase::has_children()
					  *  tests for this.
					  */
	std::vector<int>  children;
	
					 /**
					  * Store the refinement
					  * case each of the
					  * cells is refined
					  * with. This vector
					  * might be replaced by
					  * vector<vector<bool> >
					  * (dim, vector<bool>
					  * (n_cells)) which is
					  * more memory efficient.
					  */
	std::vector<RefinementCase<G::dimension> > refinement_cases;
	
					 /**
					  *  Vector storing whether an object is
					  *  used in the @p cells vector.
					  *
					  *  Since it is difficult to delete
					  *  elements in a @p vector, when an
					  *  element is not needed any more
					  *  (e.g. after derefinement), it is
					  *  not deleted from the list, but
					  *  rather the according @p used flag
					  *  is set to @p false.
					  */
	std::vector<bool> used;
	
					 /**
					  *  Make available a field for user data,
					  *  one bit per object. This field is usually
					  *  used when an operation runs over all
					  *  cells and needs information whether
					  *  another cell (e.g. a neighbor) has
					  *  already been processed.
					  *
					  *  You can clear all used flags using
					  *  Triangulation::clear_user_flags().
					  */
	std::vector<bool> user_flags;
	
					 /**
					  * Store boundary and material data. For
					  * example, in one dimension, this field
					  * stores the material id of a line, which
					  * is a number between 0 and 254. In more
					  * than one dimension, lines have no
					  * material id, but they may be at the
					  * boundary; then, we store the
					  * boundary indicator in this field,
					  * which denotes to which part of the
					  * boundary this line belongs and which
					  * boundary conditions hold on this
					  * part. The boundary indicator also
					  * is a number between zero and 254;
					  * the id 255 is reserved for lines
					  * in the interior and may be used
					  * to check whether a line is at the
					  * boundary or not, which otherwise
					  * is not possible if you don't know
					  * which cell it belongs to.
					  */
	std::vector<unsigned char> material_id;
	
                                         /**
                                          *  Assert that enough space
                                          *  is allocated to
                                          *  accomodate
                                          *  <code>new_objs_in_pairs</code>
                                          *  new objects, stored in
                                          *  pairs, plus
                                          *  <code>new_obj_single</code>
                                          *  stored individually.
                                          *  This function does not
                                          *  only call
                                          *  <code>vector::reserve()</code>,
                                          *  but does really append
                                          *  the needed elements.
					  *
					  *  In 2D e.g. refined lines have to be
					  *  stored in pairs, whereas new lines in the
					  *  interior of refined cells can be stored as
					  *  single lines.
                                          */
        void reserve_space (const unsigned int new_objs_in_pairs,
			    const unsigned int new_objs_single = 0);

					 /**
					  * Return an iterator to the
					  * next free slot for a
					  * single line. Only
					  * implemented for
					  * <code>G=TriaObject<1>
					  * </code>.
					  */
	template <int dim, int spacedim>
	typename dealii::Triangulation<dim,spacedim>::raw_line_iterator 
	next_free_single_line (const dealii::Triangulation<dim,spacedim> &tria);

					 /**
					  * Return an iterator to the
					  * next free slot for a pair
					  * of lines. Only implemented
					  * for <code>G=TriaObject<1>
					  * </code>.
					  */
	template <int dim, int spacedim>
	typename dealii::Triangulation<dim,spacedim>::raw_line_iterator 
	next_free_pair_line (const dealii::Triangulation<dim,spacedim> &tria);
	
					 /**
					  * Return an iterator to the
					  * next free slot for a
					  * single quad. Only
					  * implemented for
					  * <code>G=TriaObject@<2@>
					  * </code>.
					  */
	template <int dim, int spacedim>
	typename dealii::Triangulation<dim,spacedim>::raw_quad_iterator 
	next_free_single_quad (const dealii::Triangulation<dim,spacedim> &tria);

					 /**
					  * Return an iterator to the
					  * next free slot for a pair
					  * of quads. Only implemented
					  * for <code>G=TriaObject@<2@>
					  * </code>.
					  */
	template <int dim, int spacedim>
	typename dealii::Triangulation<dim,spacedim>::raw_quad_iterator 
	next_free_pair_quad (const dealii::Triangulation<dim,spacedim> &tria);
	
					 /**
					  * Return an iterator to the
					  * next free slot for a pair
					  * of hexes. Only implemented
					  * for
					  * <code>G=Hexahedron</code>.
					  */
	template <int dim, int spacedim>
	typename dealii::Triangulation<dim,spacedim>::raw_hex_iterator 
	next_free_hex (const dealii::Triangulation<dim,spacedim> &tria,
		       const unsigned int               level);

					 /**
					  *  Clear all the data contained in this object.
					  */
	void clear();
	
					 /**
					  * The orientation of the
					  * face number <code>face</code>
					  * of the cell with number
					  * <code>cell</code>. The return
					  * value is <code>true</code>, if
					  * the normal vector points
					  * the usual way
					  * (GeometryInfo::unit_normal_orientation)
					  * and <code>false</code> else.
					  *
					  * The result is always
					  * <code>true</code> in this
					  * class, but derived classes
					  * will reimplement this.
					  *
					  * @warning There is a bug in
					  * the class hierarchy right
					  * now. Avoid ever calling
					  * this function through a
					  * reference, since you might
					  * end up with the base class
					  * function instead of the
					  * derived class. Still, we
					  * do not want to make it
					  * virtual for efficiency
					  * reasons.
					  */
	bool face_orientation(const unsigned int cell, const unsigned int face) const;
	
	
					 /**
					  * Access to user pointers.
					  */
	void*& user_pointer(const unsigned int i);
	
					 /**
					  * Read-only access to user pointers.
					  */
	const void* user_pointer(const unsigned int i) const;
	
					 /**
					  * Access to user indices.
					  */
	unsigned int& user_index(const unsigned int i);
	
					 /**
					  * Read-only access to user pointers.
					  */
	unsigned int user_index(const unsigned int i) const;

					 /**
					  * Reset user data to zero.
					  */
	void clear_user_data(const unsigned int i);
	
					 /**
					  * Clear all user pointers or
					  * indices and reset their
					  * type, such that the next
					  * access may be aither or.
					  */
	void clear_user_data();
	
					 /**
					  * Clear all user flags.
					  */
	void clear_user_flags();
	
                                         /**
                                          *  Check the memory consistency of the
                                          *  different containers. Should only be
                                          *  called with the prepro flag @p DEBUG
                                          *  set. The function should be called from
                                          *  the functions of the higher
                                          *  TriaLevel classes.
                                          */
        void monitor_memory (const unsigned int true_dimension) const;

                                         /**
                                          * Determine an estimate for the
                                          * memory consumption (in bytes)
                                          * of this object.
                                          */
        std::size_t memory_consumption () const;

	/**
	 * Read or write the data of this object to or 
	 * from a stream for the purpose of serialization
	 */ 
	template <class Archive>
	void serialize(Archive & ar,
		       const unsigned int version);

                                         /**
                                          *  Exception
                                          */
        DeclException3 (ExcMemoryWasted,
                        char*, int, int,
                        << "The container " << arg1 << " contains "
                        << arg2 << " elements, but it`s capacity is "
                        << arg3 << ".");
                                         /**
                                          *  Exception
					  * @ingroup Exceptions
                                          */
        DeclException2 (ExcMemoryInexact,
                        int, int,
                        << "The containers have sizes " << arg1 << " and "
                        << arg2 << ", which is not as expected.");

                                         /**
                                          *  Exception
                                          */
        DeclException2 (ExcWrongIterator,
                        char*, char*,
                        << "You asked for the next free " << arg1 << "_iterator, "
                        "but you can only ask for " << arg2 <<"_iterators.");

					 /**
					  * Triangulation objacts can
					  * either access a user
					  * pointer or a user
					  * index. What you tried to
					  * do is trying to access one
					  * of those after using the
					  * other.
					  *
					  * @ingroup Exceptions
					  */
	DeclException0 (ExcPointerIndexClash);
	
      protected:
					 /**
					  * Counter for next_free_single_* functions
					  */
	unsigned int next_free_single;

					 /**
					  * Counter for next_free_pair_* functions
					  */
	unsigned int next_free_pair;

					 /**
					  * Bool flag for next_free_single_* functions
					  */
	bool reverse_order_next_free_single;
	
					 /**
					  * The data type storing user
					  * pointers or user indices.
					  */
	struct UserData
	{
	    union Data 
	    {
						 /// The entry used as user
						 /// pointer.
		void* p;
						 /// The entry used as user
						 /// index.
		unsigned int i;
						 /// Default constructor
	    };
	    Data data;

					     /**
					      * Default constructor.
					      */
	    UserData()
	      {
		data.p = 0;
	      }

					     /**
					      * Write the data of this object
					      * to a stream for the purpose of
					      * serialization.
					      */
	    template <class Archive>
	    void serialize (Archive & ar, const unsigned int version);
	};

					 /**
					  * Enum descibing the
					  * possible types of
					  * userdata.
					  */
	enum UserDataType
	{
					       /// No userdata used yet.
	      data_unknown,
					       /// UserData contains pointers.
	      data_pointer,
					       /// UserData contains indices.
	      data_index
	};
	
	
					 /**
					  * Pointer which is not used by the
					  * library but may be accessed and set
					  * by the user to handle data local to
					  * a line/quad/etc.
					  */
	std::vector<UserData> user_data;
					 /**
					  * In order to avoid
					  * confusion between user
					  * pointers and indices, this
					  * enum is set by the first
					  * function accessing either
					  * and subsequent access will
					  * not be allowed to change
					  * the type of data accessed.
					  */
	mutable UserDataType user_data_type;	
    };

/**
 * For hexahedrons the data of TriaObjects needs to be extended, as we can obtain faces
 * (quads) in non-standard-orientation, therefore we declare a class TriaObjectsHex, which
 * additionaly contains a bool-vector of the face-orientations.
 */
    
    class TriaObjectsHex : public TriaObjects<TriaObject<3> >
    {
      public:
					 /**
					  * The orientation of the
					  * face number <code>face</code>
					  * of the cell with number
					  * <code>cell</code>. The return
					  * value is <code>true</code>, if
					  * the normal vector points
					  * the usual way
					  * (GeometryInfo::unit_normal_orientation)
					  * and <code>false</code> if they
					  * point in opposite
					  * direction.
					  */
	bool face_orientation(const unsigned int cell, const unsigned int face) const;
	

					 /**
					  * For edges, we enforce a
					  * standard convention that
					  * opposite edges should be
					  * parallel. Now, that's
					  * enforcable in most cases,
					  * and we have code that
					  * makes sure that if a mesh
					  * allows this to happen,
					  * that we have this
					  * convention. We also know
					  * that it is always possible
					  * to have opposite faces
					  * have parallel normal
					  * vectors. (For both things,
					  * see the Agelek, Anderson,
					  * Bangerth, Barth paper
					  * mentioned in the
					  * publications list.)
					  *
					  * The problem is that we
					  * originally had another
					  * condition, namely that
					  * faces 0, 2 and 6 have
					  * normals that point into
					  * the cell, while the other
					  * faces have normals that
					  * point outward. It turns
					  * out that this is not
					  * always possible. In
					  * effect, we have to store
					  * whether the normal vector
					  * of each face of each cell
					  * follows this convention or
					  * not. If this is so, then
					  * this variable stores a
					  * @p true value, otherwise
					  * a @p false value.
					  *
					  * In effect, this field has
					  * <code>6*n_cells</code> elements,
					  * being the number of cells
					  * times the six faces each
					  * has.
					  */
	std::vector<bool> face_orientations;
					 /**
					  * flip = rotation by 180 degrees
					  */
	std::vector<bool> face_flips;
					 /**
					  * rotation by 90 degrees
					  */
	std::vector<bool> face_rotations;

                                         /**
                                          *  Assert that enough space is
                                          *  allocated to accomodate
                                          *  <code>new_objs</code> new objects.
                                          *  This function does not only call
                                          *  <code>vector::reserve()</code>, but
                                          *  does really append the needed
                                          *  elements.
                                          */
        void reserve_space (const unsigned int new_objs);

					 /**
					  *  Clear all the data contained in this object.
					  */
	void clear();
	
                                         /**
                                          *  Check the memory consistency of the
                                          *  different containers. Should only be
                                          *  called with the prepro flag @p DEBUG
                                          *  set. The function should be called from
                                          *  the functions of the higher
                                          *  TriaLevel classes.
                                          */
        void monitor_memory (const unsigned int true_dimension) const;

                                         /**
                                          * Determine an estimate for the
                                          * memory consumption (in bytes)
                                          * of this object.
                                          */
        std::size_t memory_consumption () const;	    

	/**
	 * Read or write the data of this object to or 
	 * from a stream for the purpose of serialization
	 */ 
	template <class Archive>
	void serialize(Archive & ar,
		       const unsigned int version);
    };


/**
 * For quadrilaterals in 3D the data of TriaObjects needs to be extended, as we
 * can obtain faces (quads) with lines in non-standard-orientation, therefore we
 * declare a class TriaObjectsQuad3D, which additionaly contains a bool-vector
 * of the line-orientations.
 */
    
    class TriaObjectsQuad3D: public TriaObjects<TriaObject<2> >
    {
      public:
					 /**
					  * The orientation of the
					  * face number <code>face</code>
					  * of the cell with number
					  * <code>cell</code>. The return
					  * value is <code>true</code>, if
					  * the normal vector points
					  * the usual way
					  * (GeometryInfo::unit_normal_orientation)
					  * and <code>false</code> if they
					  * point in opposite
					  * direction.
					  */
	bool face_orientation(const unsigned int cell, const unsigned int face) const;
	

					 /**
					  * In effect, this field has
					  * <code>4*n_quads</code> elements,
					  * being the number of quads
					  * times the four lines each
					  * has.
					  */
	std::vector<bool> line_orientations;

                                         /**
                                          *  Assert that enough space
                                          *  is allocated to
                                          *  accomodate
                                          *  <code>new_quads_in_pairs</code>
                                          *  new quads, stored in
                                          *  pairs, plus
                                          *  <code>new_quads_single</code>
                                          *  stored individually.
                                          *  This function does not
                                          *  only call
                                          *  <code>vector::reserve()</code>,
                                          *  but does really append
                                          *  the needed elements.
                                          */
        void reserve_space (const unsigned int new_quads_in_pairs,
			    const unsigned int new_quads_single = 0);

					 /**
					  *  Clear all the data contained in this object.
					  */
	void clear();
	
                                         /**
                                          *  Check the memory consistency of the
                                          *  different containers. Should only be
                                          *  called with the prepro flag @p DEBUG
                                          *  set. The function should be called from
                                          *  the functions of the higher
                                          *  TriaLevel classes.
                                          */
        void monitor_memory (const unsigned int true_dimension) const;

                                         /**
                                          * Determine an estimate for the
                                          * memory consumption (in bytes)
                                          * of this object.
                                          */
        std::size_t memory_consumption () const;	    

	/**
	 * Read or write the data of this object to or 
	 * from a stream for the purpose of serialization
	 */ 
	template <class Archive>
	void serialize(Archive & ar,
		       const unsigned int version);
    };
    
//----------------------------------------------------------------------//
    
    template<typename G>
    inline
    bool
    TriaObjects<G>::
    face_orientation(const unsigned int, const unsigned int) const
    {
      return true;
    }
    
    
    template<typename G>
    inline
    void*&
    TriaObjects<G>::user_pointer (const unsigned int i)
    {
#ifdef DEBUG
      Assert(user_data_type == data_unknown || user_data_type == data_pointer,
	     ExcPointerIndexClash());
      user_data_type = data_pointer;
#endif
      Assert(i<user_data.size(), ExcIndexRange(i,0,user_data.size()));
      return user_data[i].data.p;
    }
    

    template<typename G>
    inline
    const void*
    TriaObjects<G>::user_pointer (const unsigned int i) const
    {
#ifdef DEBUG
      Assert(user_data_type == data_unknown || user_data_type == data_pointer,
	     ExcPointerIndexClash());
      user_data_type = data_pointer;
#endif
      Assert(i<user_data.size(), ExcIndexRange(i,0,user_data.size()));
      return user_data[i].data.p;
    }
    

    template<typename G>
    inline
    unsigned int&
    TriaObjects<G>::user_index (const unsigned int i)
    {
#ifdef DEBUG
      Assert(user_data_type == data_unknown || user_data_type == data_index,
	     ExcPointerIndexClash());
      user_data_type = data_index;
#endif
      Assert(i<user_data.size(), ExcIndexRange(i,0,user_data.size()));
      return user_data[i].data.i;
    }
    
    
    template<typename G>
    inline
    void
    TriaObjects<G>::clear_user_data (const unsigned int i)
    {
      Assert(i<user_data.size(), ExcIndexRange(i,0,user_data.size()));
      user_data[i].data.i = 0;
    }
    
    
    template <typename G>
    inline
    TriaObjects<G>::TriaObjects()
		    :
		    reverse_order_next_free_single (false),
		    user_data_type(data_unknown)
    {}    
    
    
    template<typename G>
    inline
    unsigned int TriaObjects<G>::user_index (const unsigned int i) const
    {
#ifdef DEBUG
      Assert(user_data_type == data_unknown || user_data_type == data_index,
	     ExcPointerIndexClash());
      user_data_type = data_index;
#endif
      Assert(i<user_data.size(), ExcIndexRange(i,0,user_data.size()));
      return user_data[i].data.i;
    }
    
    
    template<typename G>
    inline
    void TriaObjects<G>::clear_user_data ()
    {
      user_data_type = data_unknown;
      for (unsigned int i=0;i<user_data.size();++i)
	user_data[i].data.p = 0;
    }


    template<typename G>
    inline
    void TriaObjects<G>::clear_user_flags ()
    {
      user_flags.assign(user_flags.size(),false);
    }


    template<typename G>
    template <class Archive>
    void
    TriaObjects<G>::UserData::serialize (Archive & ar,
					 const unsigned int)
    {
				       // serialize this as an integer
      ar & data.i;
    }
    
    

    template <typename G>
    template <class Archive>
    void TriaObjects<G>::serialize(Archive & ar,
				   const unsigned int)
    {
      ar & cells & children;
      ar & refinement_cases;
      ar & used;
      ar & user_flags;
      ar & material_id;
      ar & next_free_single & next_free_pair & reverse_order_next_free_single;
      ar & user_data & user_data_type;
    }


    template <class Archive>
    void TriaObjectsHex::serialize(Archive & ar,
				   const unsigned int version)
    {
      this->TriaObjects<TriaObject<3> >::serialize (ar, version);
      
      ar & face_orientations & face_flips & face_rotations;
    }


    template <class Archive>
    void TriaObjectsQuad3D::serialize(Archive & ar,
				      const unsigned int version)
    {
      this->TriaObjects<TriaObject<2> >::serialize (ar, version);
      
      ar & line_orientations;
    }


//----------------------------------------------------------------------//

    inline
    bool
    TriaObjectsHex::face_orientation(const unsigned int cell,
				     const unsigned int face) const
    {
      Assert (cell < face_orientations.size() / GeometryInfo<3>::faces_per_cell,
	      ExcIndexRange(0, cell, face_orientations.size() / GeometryInfo<3>::faces_per_cell));
      Assert (face < GeometryInfo<3>::faces_per_cell,
	      ExcIndexRange(0, face, GeometryInfo<3>::faces_per_cell));
      
      return face_orientations[cell * GeometryInfo<3>::faces_per_cell
			       + face];
    }

//----------------------------------------------------------------------//

    inline
    bool
    TriaObjectsQuad3D::face_orientation(const unsigned int cell, const unsigned int face) const
    {
      return line_orientations[cell * GeometryInfo<2>::faces_per_cell
			       + face];
    }



// declaration of explicit specializations

    template<>
    void
    TriaObjects<TriaObject<1> >::reserve_space (const unsigned int new_lines_in_pairs,
				      const unsigned int new_lines_single);

    template<>
    void
    TriaObjects<TriaObject<2> >::reserve_space (const unsigned int new_quads_in_pairs,
				      const unsigned int new_quads_single);

    template<>
    void
    TriaObjects<TriaObject<2> >::monitor_memory (const unsigned int) const;
    
  }
}



DEAL_II_NAMESPACE_CLOSE

#endif
