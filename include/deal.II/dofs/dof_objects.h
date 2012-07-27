//---------------------------------------------------------------------------
//    $Id: dof_objects.h 23876 2011-06-28 18:21:51Z kanschat $
//
//    Copyright (C) 2006 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------
#ifndef __deal2__dof_objects_h
#define __deal2__dof_objects_h


#include <deal.II/base/config.h>
#include <deal.II/base/exceptions.h>
#include <vector>

DEAL_II_NAMESPACE_OPEN

template <int, int> class DoFHandler;

namespace internal
{
  namespace DoFHandler
  {
    template <int> class DoFLevel;
    template <int> class DoFFaces;
    

/**
 * Store the indices of the degrees of freedom which are located on
 * objects of dimension @p dim. 
 *
 * <h3>Information for all DoFObjects classes</h3>
 *
 * The DoFObjects classes store the global indices of the
 * degrees of freedom for each cell on a certain level. The global
 * index or number of a degree of freedom is the zero-based index of
 * the according value in the solution vector and the row and column
 * index in the global matrix or the multigrid matrix for this
 * level. These indices refer to the unconstrained vectors and
 * matrices, where we have not taken account of the constraints
 * introduced by hanging nodes.
 *
 * Since vertices are not associated with a particular level, the
 * indices associated with vertices are not stored in the DoFObjects
 * classes but rather in the DoFHandler::vertex_dofs array.
 *
 * The DoFObjects classes are not used directly, but objects
 * of theses classes are included in the DoFLevel and
 * DoFFaces classes.
 *
 * @ingroup dofs
 * @author Tobias Leicht, 2006
 */
    template <int dim>
    class DoFObjects
    {
      public:
                                         /**
                                          * Store the global indices of
                                          * the degrees of freedom.
                                          */
        std::vector<unsigned int> dofs;

      public:
                                         /**
                                          * Set the global index of
                                          * the @p local_index-th
                                          * degree of freedom located
                                          * on the object with number @p
                                          * obj_index to the value
                                          * given by the last
                                          * argument. The @p
                                          * dof_handler argument is
                                          * used to access the finite
                                          * element that is to be used
                                          * to compute the location
                                          * where this data is stored.
                                          *
                                          * The third argument, @p
                                          * fe_index, must equal
                                          * zero. It is otherwise
                                          * unused, but we retain the
                                          * argument so that we can
                                          * use the same interface for
                                          * non-hp and hp finite
                                          * element methods, in effect
                                          * making it possible to
                                          * share the DoFAccessor
                                          * class hierarchy between hp
                                          * and non-hp classes.
                                          */
	template <int dh_dim, int spacedim>
        void
        set_dof_index (const dealii::DoFHandler<dh_dim,spacedim> &dof_handler,
		       const unsigned int       obj_index,
		       const unsigned int       fe_index,
		       const unsigned int       local_index,
		       const unsigned int       global_index);

                                         /**
                                          * Return the global index of
                                          * the @p local_index-th
                                          * degree of freedom located
                                          * on the object with number @p
                                          * obj_index. The @p
                                          * dof_handler argument is
                                          * used to access the finite
                                          * element that is to be used
                                          * to compute the location
                                          * where this data is stored.
                                          *
                                          * The third argument, @p
                                          * fe_index, must equal
                                          * zero. It is otherwise
                                          * unused, but we retain the
                                          * argument so that we can
                                          * use the same interface for
                                          * non-hp and hp finite
                                          * element methods, in effect
                                          * making it possible to
                                          * share the DoFAccessor
                                          * class hierarchy between hp
                                          * and non-hp classes.
                                          */
	template <int dh_dim, int spacedim>
	unsigned int
        get_dof_index (const dealii::DoFHandler<dh_dim,spacedim> &dof_handler,
		       const unsigned int       obj_index,
		       const unsigned int       fe_index,
		       const unsigned int       local_index) const;

                                         /**
                                          * Return the value 1. The
                                          * meaning of this function
                                          * becomes clear by looking
                                          * at what the corresponding
                                          * functions in the classes
                                          * internal::hp::DoFObjects 
                                          */
        template <int dh_dim, int spacedim>
        unsigned int
        n_active_fe_indices (const dealii::DoFHandler<dh_dim,spacedim> &dof_handler,
                             const unsigned int       index) const;

                                         /**
                                          * Similar to the function
                                          * above. Assert that the
                                          * given index is zero, and
                                          * then return true.
                                          */
        template <int dh_dim, int spacedim>
        bool
        fe_index_is_active (const dealii::DoFHandler<dh_dim,spacedim> &dof_handler,
                            const unsigned int       index,
                            const unsigned int       fe_index) const;

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
                                          * Make the DoFHandler and
                                          * MGDoFHandler classes a
                                          * friend, so that they can
                                          * resize arrays as
                                          * necessary.
                                          */
	template <int> friend class DoFLevel;
	template <int> friend class DoFFaces;	
    };


// --------------------- template and inline functions ------------------

    template <int dim>
    template <int dh_dim, int spacedim>
    inline
    unsigned int
    DoFObjects<dim>::n_active_fe_indices (const dealii::DoFHandler<dh_dim,spacedim> &,
					  const unsigned) const
    {
      return 1;
    }
    

    
    template <int dim>
    template <int dh_dim, int spacedim>
    inline
    bool
    DoFObjects<dim>::fe_index_is_active (const dealii::DoFHandler<dh_dim,spacedim> &,
					 const unsigned int,
					 const unsigned int fe_index) const
    {
      Assert (fe_index == 0,
              ExcMessage ("Only zero fe_index values are allowed for "
                          "non-hp DoFHandlers."));
      return true;
    }

    

    template <int dim>
    template <int dh_dim, int spacedim>
    inline
    unsigned int
    DoFObjects<dim>::
    get_dof_index (const dealii::DoFHandler<dh_dim,spacedim> &dof_handler,
		   const unsigned int       obj_index,
		   const unsigned int       fe_index,
		   const unsigned int       local_index) const
    {
      Assert ((fe_index == dealii::DoFHandler<dh_dim,spacedim>::default_fe_index),
	      ExcMessage ("Only the default FE index is allowed for non-hp DoFHandler objects"));
      Assert (local_index<dof_handler.get_fe().template n_dofs_per_object<dim>(),
	      ExcIndexRange (local_index, 0, dof_handler.get_fe().template n_dofs_per_object<dim>()));
      Assert (obj_index * dof_handler.get_fe().template n_dofs_per_object<dim>()+local_index
	      <
	      dofs.size(),
	      ExcInternalError());
      
      return dofs[obj_index * dof_handler.get_fe()
                  .template n_dofs_per_object<dim>() + local_index];
    }    


    template <int dim>
    template <class Archive>
    void DoFObjects<dim>::serialize(Archive & ar,
				    const unsigned int)
    {
      ar & dofs;
    }

  }
}

DEAL_II_NAMESPACE_CLOSE

#endif
