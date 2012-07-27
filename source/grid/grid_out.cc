//---------------------------------------------------------------------------
//    $Id: grid_out.cc 23709 2011-05-17 04:34:08Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2008, 2010, 2011 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//---------------------------------------------------------------------------


#include <deal.II/grid/grid_out.h>
#include <deal.II/base/parameter_handler.h>
#include <deal.II/base/point.h>
#include <deal.II/base/quadrature.h>
#include <deal.II/base/qprojector.h>
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/fe/mapping.h>

#include <cstring>
#include <iomanip>
#include <algorithm>
#include <list>
#include <set>
#include <ctime>
#include <cmath>


DEAL_II_NAMESPACE_OPEN


namespace GridOutFlags
{
  DX::DX (const bool write_cells,
	  const bool write_faces,
	  const bool write_diameter,
	  const bool write_measure,
	  const bool write_all_faces) :
    write_cells (write_cells),
    write_faces (write_faces),
    write_diameter (write_diameter),
    write_measure (write_measure),
    write_all_faces (write_all_faces)
  {}

  void DX::declare_parameters (ParameterHandler& param)
  {
    param.declare_entry("Write cells", "true", Patterns::Bool(),
			"Write the mesh connectivity as DX grid cells");
    param.declare_entry("Write faces", "false", Patterns::Bool(),
			"Write faces of cells. These may be boundary faces "
			"or all faces between mesh cells, according to "
			"\"Write all faces\"");
    param.declare_entry("Write diameter", "false", Patterns::Bool(),
			"If cells are written, additionally write their"
			" diameter as data for visualization");
    param.declare_entry("Write measure", "false", Patterns::Bool(),
			"Write the volume of each cell as data");
    param.declare_entry("Write all faces", "true", Patterns::Bool(),
			"Write all faces, not only boundary");
  }

  void DX::parse_parameters (ParameterHandler& param)
  {
    write_cells = param.get_bool("Write cells");
    write_faces = param.get_bool("Write faces");
    write_diameter = param.get_bool("Write diameter");
    write_measure = param.get_bool("Write measure");
    write_all_faces = param.get_bool("Write all faces");
  }


  Msh::Msh (const bool write_faces,
	    const bool write_lines) :
    write_faces (write_faces),
    write_lines (write_lines)
  {}

  void Msh::declare_parameters (ParameterHandler& param)
  {
    param.declare_entry("Write faces", "false", Patterns::Bool());
    param.declare_entry("Write lines", "false", Patterns::Bool());
  }


  void Msh::parse_parameters (ParameterHandler& param)
  {
    write_faces = param.get_bool("Write faces");
    write_lines = param.get_bool("Write lines");
  }


  Ucd::Ucd (const bool write_preamble,
	    const bool write_faces,
	    const bool write_lines) :
		  write_preamble (write_preamble),
		  write_faces (write_faces),
		  write_lines (write_lines)
  {}



  void Ucd::declare_parameters (ParameterHandler& param)
  {
    param.declare_entry("Write preamble", "true", Patterns::Bool());
    param.declare_entry("Write faces", "false", Patterns::Bool());
    param.declare_entry("Write lines", "false", Patterns::Bool());
  }


  void Ucd::parse_parameters (ParameterHandler& param)
  {
    write_preamble = param.get_bool("Write preamble");
    write_faces = param.get_bool("Write faces");
    write_lines = param.get_bool("Write lines");
  }


  Gnuplot::Gnuplot (const bool write_cell_numbers,
		    const unsigned int n_boundary_face_points,
		    const bool         curved_inner_cells) :
		  write_cell_numbers (write_cell_numbers),
		  n_boundary_face_points(n_boundary_face_points),
		  curved_inner_cells(curved_inner_cells)
  {}



  void Gnuplot::declare_parameters (ParameterHandler& param)
  {
    param.declare_entry("Cell number", "false", Patterns::Bool());
    param.declare_entry("Boundary points", "2", Patterns::Integer());
  }


  void Gnuplot::parse_parameters (ParameterHandler& param)
  {
    write_cell_numbers = param.get_bool("Cell number");
    n_boundary_face_points = param.get_integer("Boundary points");
  }


  EpsFlagsBase::EpsFlagsBase (const SizeType     size_type,
			      const unsigned int size,
			      const double       line_width,
			      const bool color_lines_on_user_flag,
			      const unsigned int n_boundary_face_points,
			      const bool color_lines_level) :
		  size_type (size_type),
		  size (size),
		  line_width (line_width),
		  color_lines_on_user_flag(color_lines_on_user_flag),
		  n_boundary_face_points(n_boundary_face_points),
		  color_lines_level(color_lines_level)
  {}


  void EpsFlagsBase::declare_parameters (ParameterHandler& param)
  {
    param.declare_entry("Size by", "width",
			    Patterns::Selection("width|height"),
			"Depending on this parameter, either the"
			"width or height "
			"of the eps is scaled to \"Size\"");
    param.declare_entry("Size", "300", Patterns::Integer(),
			"Size of the output in points");
    param.declare_entry("Line width", "0.5", Patterns::Double(),
			"Width of the lines drawn in points");
    param.declare_entry("Color by flag", "false", Patterns::Bool(),
			"Draw lines with user flag set in different color");
    param.declare_entry("Boundary points", "2", Patterns::Integer(),
			"Number of points on boundary edges. "
			"Increase this beyond 2 to see curved boundaries.");
    param.declare_entry("Color by level", "false", Patterns::Bool(),
			"Draw different colors according to grid level.");
  }


  void EpsFlagsBase::parse_parameters (ParameterHandler& param)
  {
    if (param.get("Size by") == std::string("width"))
      size_type = width;
    else if (param.get("Size by") == std::string("height"))
      size_type = height;
    size = param.get_integer("Size");
    line_width = param.get_double("Line width");
    color_lines_on_user_flag = param.get_bool("Color by flag");
    n_boundary_face_points = param.get_integer("Boundary points");
    color_lines_level = param.get_bool("Color by level");
  }



  Eps<1>::Eps (const SizeType     size_type,
	       const unsigned int size,
	       const double       line_width,
	       const bool color_lines_on_user_flag,
	       const unsigned int n_boundary_face_points)
		  :
		  EpsFlagsBase(size_type, size, line_width,
			       color_lines_on_user_flag,
			       n_boundary_face_points)
  {}


  void Eps<1>::declare_parameters (ParameterHandler&)
  {}


  void Eps<1>::parse_parameters (ParameterHandler& param)
  {
    EpsFlagsBase::parse_parameters(param);
  }



  Eps<2>::Eps (const SizeType     size_type,
	       const unsigned int size,
	       const double       line_width,
	       const bool color_lines_on_user_flag,
	       const unsigned int n_boundary_face_points,
	       const bool         write_cell_numbers,
	       const bool         write_cell_number_level,
	       const bool         write_vertex_numbers,
	       const bool         color_lines_level
      )
		  :
		  EpsFlagsBase(size_type, size, line_width,
			       color_lines_on_user_flag,
			       n_boundary_face_points,
			       color_lines_level),
		  write_cell_numbers (write_cell_numbers),
		  write_cell_number_level (write_cell_number_level),
		  write_vertex_numbers (write_vertex_numbers)
  {}


  void Eps<2>::declare_parameters (ParameterHandler& param)
  {
    param.declare_entry("Cell number", "false", Patterns::Bool(),
			"(2D only) Write cell numbers"
			" into the centers of cells");
    param.declare_entry("Level number", "false", Patterns::Bool(),
			"(2D only) if \"Cell number\" is true, write"
			"numbers in the form level.number");
    param.declare_entry("Vertex number", "false", Patterns::Bool(),
			"Write numbers for each vertex");
  }


  void Eps<2>::parse_parameters (ParameterHandler& param)
  {
    EpsFlagsBase::parse_parameters(param);
    write_cell_numbers = param.get_bool("Cell number");
    write_cell_number_level = param.get_bool("Level number");
    write_vertex_numbers = param.get_bool("Vertex number");
  }



  Eps<3>::Eps (const SizeType     size_type,
	       const unsigned int size,
	       const double       line_width,
	       const bool color_lines_on_user_flag,
	       const unsigned int n_boundary_face_points,
	       const double        azimut_angle,
	       const double        turn_angle)
		  :
		  EpsFlagsBase(size_type, size, line_width,
			       color_lines_on_user_flag,
			       n_boundary_face_points),
		  azimut_angle (azimut_angle),
		  turn_angle (turn_angle)
  {}


  void Eps<3>::declare_parameters (ParameterHandler& param)
  {
    param.declare_entry("Azimuth", "30", Patterns::Double(),
			"Azimuth of the viw point, that is, the angle "
			"in the plane from the x-axis.");
    param.declare_entry("Elevation", "30", Patterns::Double(),
			"Elevation of the view point above the xy-plane.");
  }


  void Eps<3>::parse_parameters (ParameterHandler& param)
  {
    EpsFlagsBase::parse_parameters(param);
    azimut_angle = 90- param.get_double("Elevation");
    turn_angle = param.get_double("Azimuth");
  }



  XFig::XFig ()
		  :
    draw_boundary(true),
    level_color(false),
    level_depth(true),
    n_boundary_face_points(0),
    scaling(1.,1.),
    fill_style (20),
    line_style(0),
    line_thickness(1),
    boundary_style(0),
    boundary_thickness(3)
  {}


  void XFig::declare_parameters (ParameterHandler& param)
  {
    param.declare_entry("Boundary", "true", Patterns::Bool());
    param.declare_entry("Level color", "false", Patterns::Bool());
    param.declare_entry("Level depth", "true", Patterns::Bool());
//TODO: Unify this number with other output formats
    param.declare_entry("Boundary points", "0", Patterns::Integer());
    param.declare_entry("Fill style", "20", Patterns::Integer());
    param.declare_entry("Line style", "0", Patterns::Integer());
    param.declare_entry("Line width", "1", Patterns::Integer());
    param.declare_entry("Boundary style", "0", Patterns::Integer());
    param.declare_entry("Boundary width", "3", Patterns::Integer());
  }


  void XFig::parse_parameters (ParameterHandler& param)
  {
    draw_boundary = param.get_bool("Boundary");
    level_color = param.get_bool("Level color");
    level_depth = param.get_bool("Level depth");
    n_boundary_face_points = param.get_integer("Boundary points");
    fill_style = param.get_integer("Fill style");
    line_style = param.get_integer("Line style");
    line_thickness = param.get_integer("Line width");
    boundary_style = param.get_integer("Boundary style");
    boundary_thickness = param.get_integer("Boundary width");
  }


}  // end namespace GridOutFlags



GridOut::GridOut ()
		:
		default_format (none)
{}


void GridOut::set_flags (const GridOutFlags::DX &flags)
{
  dx_flags = flags;
}



void GridOut::set_flags (const GridOutFlags::Msh &flags)
{
  msh_flags = flags;
}


void GridOut::set_flags (const GridOutFlags::Ucd &flags)
{
  ucd_flags = flags;
}



void GridOut::set_flags (const GridOutFlags::Gnuplot &flags)
{
  gnuplot_flags = flags;
}



void GridOut::set_flags (const GridOutFlags::Eps<1> &flags)
{
  eps_flags_1 = flags;
}



void GridOut::set_flags (const GridOutFlags::Eps<2> &flags)
{
  eps_flags_2 = flags;
}



void GridOut::set_flags (const GridOutFlags::Eps<3> &flags)
{
  eps_flags_3 = flags;
}



void GridOut::set_flags (const GridOutFlags::XFig &flags)
{
  xfig_flags = flags;
}



std::string
GridOut::default_suffix (const OutputFormat output_format)
{
  switch (output_format)
    {
      case none:
	return "";
      case dx:
        return ".dx";
      case gnuplot:
	return ".gnuplot";
      case ucd:
	return ".inp";
      case eps:
	return ".eps";
      case xfig:
	return ".fig";
      case msh:
	return ".msh";
      default:
	    Assert (false, ExcNotImplemented());
	    return "";
    };
}



std::string
GridOut::default_suffix () const
{
  return default_suffix(default_format);
}



GridOut::OutputFormat
GridOut::parse_output_format (const std::string &format_name)
{
  if (format_name == "none" || format_name == "false")
    return none;

  if (format_name == "dx")
    return dx;

  if (format_name == "ucd")
    return ucd;

  if (format_name == "gnuplot")
    return gnuplot;

  if (format_name == "eps")
    return eps;

  if (format_name == "xfig")
    return xfig;

  if (format_name == "msh")
    return msh;

  AssertThrow (false, ExcInvalidState ());
				   // return something weird
  return OutputFormat(-1);
}



std::string GridOut::get_output_format_names ()
{
  return "none|dx|gnuplot|eps|ucd|xfig|msh";
}


void
GridOut::declare_parameters(ParameterHandler& param)
{
  param.declare_entry("Format", "none",
		      Patterns::Selection(get_output_format_names()));

  param.enter_subsection("DX");
  GridOutFlags::DX::declare_parameters(param);
  param.leave_subsection();

  param.enter_subsection("Msh");
  GridOutFlags::Msh::declare_parameters(param);
  param.leave_subsection();

  param.enter_subsection("Ucd");
  GridOutFlags::Ucd::declare_parameters(param);
  param.leave_subsection();

  param.enter_subsection("Gnuplot");
  GridOutFlags::Gnuplot::declare_parameters(param);
  param.leave_subsection();

  param.enter_subsection("Eps");
  GridOutFlags::EpsFlagsBase::declare_parameters(param);
  GridOutFlags::Eps<1>::declare_parameters(param);
  GridOutFlags::Eps<2>::declare_parameters(param);
  GridOutFlags::Eps<3>::declare_parameters(param);
  param.leave_subsection();

  param.enter_subsection("XFig");
  GridOutFlags::XFig::declare_parameters(param);
  param.leave_subsection();
}



void
GridOut::parse_parameters(ParameterHandler& param)
{
  default_format = parse_output_format(param.get("Format"));

  param.enter_subsection("DX");
  dx_flags.parse_parameters(param);
  param.leave_subsection();

  param.enter_subsection("Msh");
  msh_flags.parse_parameters(param);
  param.leave_subsection();

  param.enter_subsection("Ucd");
  ucd_flags.parse_parameters(param);
  param.leave_subsection();

  param.enter_subsection("Gnuplot");
  gnuplot_flags.parse_parameters(param);
  param.leave_subsection();

  param.enter_subsection("Eps");
  eps_flags_1.parse_parameters(param);
  eps_flags_2.parse_parameters(param);
  eps_flags_3.parse_parameters(param);
  param.leave_subsection();

  param.enter_subsection("XFig");
  xfig_flags.parse_parameters(param);
  param.leave_subsection();
}



std::size_t
GridOut::memory_consumption () const
{
  return (sizeof(dx_flags) +
	  sizeof(msh_flags) +
	  sizeof(ucd_flags) +
	  sizeof(gnuplot_flags) +
	  sizeof(eps_flags_1) +
	  sizeof(eps_flags_2) +
	  sizeof(eps_flags_3) +
	  sizeof(xfig_flags));
}



template <>
void GridOut::write_dx (const Triangulation<1> &,
			std::ostream             &) const
{
  Assert (false, ExcNotImplemented());
}



template <int dim>
void GridOut::write_dx (const Triangulation<dim> &tria,
			std::ostream             &out) const
{
//TODO:[GK] allow for boundary faces only
  Assert(dx_flags.write_all_faces, ExcNotImplemented());
  AssertThrow (out, ExcIO());
				   // Copied and adapted from write_ucd
  const std::vector<Point<dim> > &vertices    = tria.get_vertices();
  const std::vector<bool>        &vertex_used = tria.get_used_vertices();

  const unsigned int n_vertices = tria.n_used_vertices();

				   // vertices are implicitly numbered from 0 to
				   // n_vertices-1. we have to renumber the
				   // vertices, because otherwise we would end
				   // up with wrong results, if there are unused
				   // vertices
  std::vector<unsigned int> renumber(vertices.size());
				   // fill this vector with new vertex numbers
				   // ranging from 0 to n_vertices-1
  unsigned int new_number=0;
  for (unsigned int i=0; i<vertices.size(); ++i)
    if (vertex_used[i])
      renumber[i]=new_number++;
  Assert(new_number==n_vertices, ExcInternalError());

  typename Triangulation<dim>::active_cell_iterator       cell;
  const typename Triangulation<dim>::active_cell_iterator endc=tria.end();


				   // write the vertices
  out << "object \"vertices\" class array type float rank 1 shape " << dim
      << " items " << n_vertices << " data follows"
      << '\n';

  for (unsigned int i=0; i<vertices.size(); ++i)
    if (vertex_used[i])
      {
	out << '\t' << vertices[i] << '\n';
      };

				   // write cells or faces
  const bool write_cells = dx_flags.write_cells;
  const bool write_faces = (dim>1) ? dx_flags.write_faces : false;

  const unsigned int n_cells = tria.n_active_cells();
  const unsigned int n_faces = tria.n_active_cells()
			       * GeometryInfo<dim>::faces_per_cell;

  const unsigned int n_vertices_per_cell = GeometryInfo<dim>::vertices_per_cell;
  const unsigned int n_vertices_per_face = GeometryInfo<dim>::vertices_per_face;

  if (write_cells)
    {
      out << "object \"cells\" class array type int rank 1 shape "
	  << n_vertices_per_cell
	  << " items " << n_cells << " data follows" << '\n';

      for (cell = tria.begin_active(); cell != endc; ++cell)
	{
	  for (unsigned int v=0; v<GeometryInfo<dim>::vertices_per_cell; ++v)
	    out << '\t' << renumber[cell->vertex_index(GeometryInfo<dim>::dx_to_deal[v])];
	  out << '\n';
	}
      out << "attribute \"element type\" string \"";
      if (dim==1) out << "lines";
      if (dim==2) out << "quads";
      if (dim==3) out << "cubes";
      out << "\"" << '\n'
	  << "attribute \"ref\" string \"positions\"" << '\n' << '\n';

				       // Additional cell information

      out << "object \"material\" class array type int rank 0 items "
	  << n_cells << " data follows" << '\n';
      for (cell = tria.begin_active(); cell != endc; ++cell)
	out << ' ' << (unsigned int)cell->material_id();
      out  << '\n'
	   << "attribute \"dep\" string \"connections\"" << '\n' << '\n';

      out << "object \"level\" class array type int rank 0 items "
	  << n_cells << " data follows" << '\n';
      for (cell = tria.begin_active(); cell != endc; ++cell)
	out << ' ' << cell->level();
      out  << '\n'
	   << "attribute \"dep\" string \"connections\"" << '\n' << '\n';

      if (dx_flags.write_measure)
	{
	  out << "object \"measure\" class array type float rank 0 items "
	      << n_cells << " data follows" << '\n';
	  for (cell = tria.begin_active(); cell != endc; ++cell)
	    out << '\t' << cell->measure();
	  out  << '\n'
	       << "attribute \"dep\" string \"connections\"" << '\n' << '\n';
	}

      if (dx_flags.write_diameter)
	{
	  out << "object \"diameter\" class array type float rank 0 items "
	      << n_cells << " data follows" << '\n';
	  for (cell = tria.begin_active(); cell != endc; ++cell)
	    out << '\t' << cell->diameter();
	  out  << '\n'
	       << "attribute \"dep\" string \"connections\"" << '\n' << '\n';
	}
    }

  if (write_faces)
    {
      out << "object \"faces\" class array type int rank 1 shape "
	  << n_vertices_per_face
	  << " items " << n_faces << " data follows"
	  << '\n';

      for (cell = tria.begin_active(); cell != endc; ++cell)
	{
	  for (unsigned int f=0;f<GeometryInfo<dim>::faces_per_cell;++f)
	    {
	      typename Triangulation<dim>::face_iterator face = cell->face(f);

	      for (unsigned int v=0; v<GeometryInfo<dim>::vertices_per_face; ++v)
		out << '\t' << renumber[face->vertex_index(GeometryInfo<dim-1>::dx_to_deal[v])];
	      out << '\n';
	    }
	}
      out << "attribute \"element type\" string \"";
      if (dim==2) out << "lines";
      if (dim==3) out << "quads";
      out << "\"" << '\n'
	  << "attribute \"ref\" string \"positions\"" << '\n' << '\n';


				       // Additional face information

      out << "object \"boundary\" class array type int rank 0 items "
	  << n_faces << " data follows" << '\n';
      for (cell = tria.begin_active(); cell != endc; ++cell)
	{
					   // Little trick to get -1
					   // for the interior
	  for (unsigned int f=0;f<GeometryInfo<dim>::faces_per_cell;++f)
	    out << ' ' << (int)(signed char)cell->face(f)->boundary_indicator();
	  out << '\n';
	}
      out << "attribute \"dep\" string \"connections\"" << '\n' << '\n';

      if (dx_flags.write_measure)
	{
	  out << "object \"face measure\" class array type float rank 0 items "
	      << n_faces << " data follows" << '\n';
	  for (cell = tria.begin_active(); cell != endc; ++cell)
	    {
	      for (unsigned int f=0;f<GeometryInfo<dim>::faces_per_cell;++f)
		out << ' ' << cell->face(f)->measure();
	      out << '\n';
	    }
	  out << "attribute \"dep\" string \"connections\"" << '\n' << '\n';
	}

      if (dx_flags.write_diameter)
	{
	  out << "object \"face diameter\" class array type float rank 0 items "
	      << n_faces << " data follows" << '\n';
	  for (cell = tria.begin_active(); cell != endc; ++cell)
	    {
	      for (unsigned int f=0;f<GeometryInfo<dim>::faces_per_cell;++f)
		out << ' ' << cell->face(f)->diameter();
	      out << '\n';
	    }
	  out << "attribute \"dep\" string \"connections\"" << '\n' << '\n';
	}

    }


				   // Write additional face information

  if (write_faces)
    {

    }
  else
    {
    }

				   // The wrapper
  out << "object \"deal data\" class field" << '\n'
      << "component \"positions\" value \"vertices\"" << '\n'
      << "component \"connections\" value \"cells\"" << '\n';

  if (write_cells)
    {
      out << "object \"cell data\" class field" << '\n'
	  << "component \"positions\" value \"vertices\"" << '\n'
	  << "component \"connections\" value \"cells\"" << '\n';
      out << "component \"material\" value \"material\"" << '\n';
      out << "component \"level\" value \"level\"" << '\n';
      if (dx_flags.write_measure)
	out << "component \"measure\" value \"measure\"" << '\n';
      if (dx_flags.write_diameter)
	out << "component \"diameter\" value \"diameter\"" << '\n';
    }

  if (write_faces)
    {
      out << "object \"face data\" class field" << '\n'
	  << "component \"positions\" value \"vertices\"" << '\n'
	  << "component \"connections\" value \"faces\"" << '\n';
      out << "component \"boundary\" value \"boundary\"" << '\n';
      if (dx_flags.write_measure)
	out << "component \"measure\" value \"face measure\"" << '\n';
      if (dx_flags.write_diameter)
	out << "component \"diameter\" value \"face diameter\"" << '\n';
    }

  out << '\n'
      << "object \"grid data\" class group" << '\n';
  if (write_cells)
    out << "member \"cells\" value \"cell data\"" << '\n';
  if (write_faces)
    out << "member \"faces\" value \"face data\"" << '\n';
  out << "end" << '\n';

				   // make sure everything now gets to
				   // disk
  out.flush ();

  AssertThrow (out, ExcIO());
}




template <int dim, int spacedim>
void GridOut::write_msh (const Triangulation<dim, spacedim> &tria,
			 std::ostream             &out) const
{
  AssertThrow (out, ExcIO());

				   // get the positions of the
				   // vertices and whether they are
				   // used.
  const std::vector<Point<spacedim> > &vertices    = tria.get_vertices();
  const std::vector<bool>        &vertex_used = tria.get_used_vertices();

  const unsigned int n_vertices = tria.n_used_vertices();

  typename Triangulation<dim,spacedim>::active_cell_iterator       cell=tria.begin_active();
  const typename Triangulation<dim,spacedim>::active_cell_iterator endc=tria.end();

				   // Write Header
				   // The file format is:
				   /*


				   $NOD
				   number-of-nodes
				   node-number x-coord y-coord z-coord
				   ...
				   $ENDNOD
				   $ELM
				   number-of-elements
				   elm-number elm-type reg-phys reg-elem number-of-nodes node-number-list
				   ...
				   $ENDELM
				   */
  out << "$NOD" << std::endl
      << n_vertices << std::endl;

				   // actually write the vertices.
				   // note that we shall number them
				   // with first index 1 instead of 0
  for (unsigned int i=0; i<vertices.size(); ++i)
    if (vertex_used[i])
      {
	out << i+1                 // vertex index
	    << "  "
	    << vertices[i];
	for (unsigned int d=spacedim+1; d<=3; ++d)
	  out << " 0";             // fill with zeroes
	out << std::endl;
      };

				   // Write cells preamble
  out << "$ENDNOD" << std::endl
      << "$ELM" << std::endl
      << tria.n_active_cells() + ( (msh_flags.write_faces ?
				    n_boundary_faces(tria) :0 ) +
				   ( msh_flags.write_lines ?
				     n_boundary_lines(tria) : 0 ) ) << std::endl;

				   /*
				     elm-type
				     defines the geometrical type of the n-th element:
				     1
				     Line (2 nodes).
				     2
				     Triangle (3 nodes).
				     3
				     Quadrangle (4 nodes).
				     4
				     Tetrahedron (4 nodes).
				     5
				     Hexahedron (8 nodes).
				     6
				     Prism (6 nodes).
				     7
				     Pyramid (5 nodes).
				     8
				     Second order line (3 nodes: 2 associated with the vertices and 1 with the edge).
				     9
				     Second order triangle (6 nodes: 3 associated with the vertices and 3 with the edges).
				     10
				     Second order quadrangle (9 nodes: 4 associated with the vertices, 4 with the edges and 1 with the face).
				     11
				     Second order tetrahedron (10 nodes: 4 associated with the vertices and 6 with the edges).
				     12
				     Second order hexahedron (27 nodes: 8 associated with the vertices, 12 with the edges, 6 with the faces and 1 with the volume).
				     13
				     Second order prism (18 nodes: 6 associated with the vertices, 9 with the edges and 3 with the quadrangular faces).
				     14
				     Second order pyramid (14 nodes: 5 associated with the vertices, 8 with the edges and 1 with the quadrangular face).
				     15
				     Point (1 node).
				   */
  unsigned int elm_type;
  switch(dim) {
    case 1:
	  elm_type = 1;
	  break;
    case 2:
	  elm_type = 3;
	  break;
    case 3:
	  elm_type = 5;
	  break;
    default:
	  Assert(false, ExcNotImplemented());
  }

				   // write cells. Enumerate cells
				   // consecutively, starting with 1
  unsigned int cell_index=1;
  for (cell=tria.begin_active();
       cell!=endc; ++cell, ++cell_index)
    {
      out << cell_index << ' ' << elm_type << ' '
	  << static_cast<unsigned int>(cell->material_id()) << ' '
	  << cell->subdomain_id() << ' '
	  << GeometryInfo<dim>::vertices_per_cell << ' ';

				       // Vertex numbering follows UCD conventions.

      for (unsigned int vertex=0; vertex<GeometryInfo<dim>::vertices_per_cell;
	   ++vertex)
	out << cell->vertex_index(GeometryInfo<dim>::ucd_to_deal[vertex])+1 << ' ';
      out << std::endl;
    };

				   // write faces and lines with
				   // non-zero boundary indicator
  if (msh_flags.write_faces)
    write_msh_faces (tria, cell_index, out);
  if (msh_flags.write_lines)
    write_msh_lines (tria, cell_index, out);

  out << "$ENDELM" << std::endl;

				   // make sure everything now gets to
				   // disk
  out.flush ();

  AssertThrow (out, ExcIO());
}


template <int dim, int spacedim>
void GridOut::write_ucd (const Triangulation<dim,spacedim> &tria,
			 std::ostream             &out) const
{
  AssertThrow (out, ExcIO());

				   // get the positions of the
				   // vertices and whether they are
				   // used.
  const std::vector<Point<spacedim> > &vertices    = tria.get_vertices();
  const std::vector<bool>             &vertex_used = tria.get_used_vertices();

  const unsigned int n_vertices = tria.n_used_vertices();

  typename Triangulation<dim,spacedim>::active_cell_iterator       cell=tria.begin_active();
  const typename Triangulation<dim,spacedim>::active_cell_iterator endc=tria.end();

				   // write preamble
  if (ucd_flags.write_preamble)
    {
				       // block this to have local
				       // variables destroyed after
				       // use
      std::time_t  time1= std::time (0);
      std::tm     *time = std::localtime(&time1);
      out << "# This file was generated by the deal.II library." << '\n'
	  << "# Date =  "
	  << time->tm_year+1900 << "/"
	  << time->tm_mon+1 << "/"
	  << time->tm_mday << '\n'
	  << "# Time =  "
	  << time->tm_hour << ":"
	  << std::setw(2) << time->tm_min << ":"
	  << std::setw(2) << time->tm_sec << '\n'
	  << "#" << '\n'
	  << "# For a description of the UCD format see the AVS Developer's guide."
	  << '\n'
	  << "#" << '\n';
    };

				   // start with ucd data
  out << n_vertices << ' '
      << tria.n_active_cells() + ( (ucd_flags.write_faces ?
				    n_boundary_faces(tria) : 0) +
				   (ucd_flags.write_lines ?
				    n_boundary_lines(tria) : 0) )
      << " 0 0 0"                  // no data
      << '\n';

				   // actually write the vertices.
				   // note that we shall number them
				   // with first index 1 instead of 0
  for (unsigned int i=0; i<vertices.size(); ++i)
    if (vertex_used[i])
      {
	out << i+1                 // vertex index
	    << "  "
	    << vertices[i];
	for (unsigned int d=spacedim+1; d<=3; ++d)
	  out << " 0";             // fill with zeroes
	out << '\n';
      };

				   // write cells. Enumerate cells
				   // consecutively, starting with 1
  unsigned int cell_index=1;
  for (cell=tria.begin_active();
       cell!=endc; ++cell, ++cell_index)
    {
      out << cell_index << ' '
	  << static_cast<unsigned int>(cell->material_id())
	  << ' ';
      switch (dim)
	{
	  case 1:  out << "line    "; break;
	  case 2:  out << "quad    "; break;
	  case 3:  out << "hex     "; break;
	  default:
		Assert (false, ExcNotImplemented());
	};

				       // it follows a list of the
				       // vertices of each cell. in 1d
				       // this is simply a list of the
				       // two vertices, in 2d its counter
				       // clockwise, as usual in this
				       // library. in 3d, the same applies
				       // (special thanks to AVS for
				       // numbering their vertices in a
				       // way compatible to deal.II!)
				       //
				       // technical reference:
				       // AVS Developer's Guide, Release 4,
				       // May, 1992, p. E6
				       //
				       // note: vertex numbers are 1-base
      for (unsigned int vertex=0; vertex<GeometryInfo<dim>::vertices_per_cell;
	   ++vertex)
	out << cell->vertex_index(GeometryInfo<dim>::ucd_to_deal[vertex])+1 << ' ';
      out << '\n';
    };

				   // write faces and lines with
				   // non-zero boundary indicator
  if (ucd_flags.write_faces)
    write_ucd_faces (tria, cell_index, out);
  if (ucd_flags.write_lines)
    write_ucd_lines (tria, cell_index, out);

				   // make sure everything now gets to
				   // disk
  out.flush ();

  AssertThrow (out, ExcIO());
}



template <int dim>
void GridOut::write_xfig (
  const Triangulation<dim>&,
  std::ostream&,
  const Mapping<dim>*) const
{
  Assert (false, ExcNotImplemented());
}


//TODO:[GK] Obey parameters
//TODO:[GK] Flip y-axis?
template <>
void GridOut::write_xfig (
  const Triangulation<2>& tria,
  std::ostream&             out,
  const Mapping<2>*       /*mapping*/) const
{
  const int dim = 2;

  const unsigned int nv = GeometryInfo<dim>::vertices_per_cell;
  const unsigned int nf = GeometryInfo<dim>::faces_per_cell;
  const unsigned int nvf = GeometryInfo<dim>::vertices_per_face;

				   // The following text was copied
				   // from an existing XFig file.
  out << "#FIG 3.2\nLandscape\nCenter\nInches" << '\n'
      << "A4\n100.00\nSingle" << '\n'
				     // Background is transparent
      << "-3" << '\n'
      << "# generated by deal.II GridOut class" << '\n'
      << "# reduce first number to scale up image" << '\n'
      << "1200 2" << '\n';

				   // We write all cells and cells on
				   // coarser levels are behind cells
				   // on finer levels. Level 0
				   // corresponds to a depth of 900,
				   // each level subtracting 1
  Triangulation<dim>::cell_iterator cell = tria.begin();
  const Triangulation<dim>::cell_iterator end = tria.end();

  for (;cell != end; ++cell)
    {
				       // If depth is not encoded, write finest level only
      if (!xfig_flags.level_depth && !cell->active())
	continue;
				       // Code for polygon
      out << "2 3  "
	  << xfig_flags.line_style << ' '
	  << xfig_flags.line_thickness
					 // with black line
	  << " 0 ";
				       // Fill color
      if (xfig_flags.level_color)
	out << cell->level() + 8;
      else
	out << cell->material_id() + 1;
				       // Depth, unused, fill
      out << ' '
	  << (xfig_flags.level_depth
	      ? (900-cell->level())
	      : (900+cell->material_id()))
	  << " 0 "
	  << xfig_flags.fill_style << " 0.0 "
					 // some style parameters
	  << " 0 0 -1 0 0 "
					 // number of points
	  << nv+1 << '\n';

				       // For each point, write scaled
				       // and shifted coordinates
				       // multiplied by 1200
				       // (dots/inch)
      for (unsigned int k=0;k<=nv;++k)
	{
	  const Point<dim>& p = cell->vertex(
	    GeometryInfo<dim>::ucd_to_deal[k % nv]);
	  for (unsigned int d=0; d<static_cast<unsigned int>(dim); ++d)
	    {
	      int val = (int)(1200 * xfig_flags.scaling(d) *
			      (p(d)-xfig_flags.offset(d)));
	      out << '\t' << val;
	    }
	  out << '\n';
	}
				       // Now write boundary edges
      static const unsigned int face_reorder[4]={2,1,3,0};
      if (xfig_flags.draw_boundary)
	for (unsigned int f=0;f<nf;++f)
	  {
	    Triangulation<dim>::face_iterator
	      face = cell->face(face_reorder[f]);
	    const unsigned char bi = face->boundary_indicator();
	    if (bi != 255)
	      {
						 // Code for polyline
		out << "2 1 "
						   // with line style and thickness
		    << xfig_flags.boundary_style << ' '
		    << xfig_flags.boundary_thickness << ' '
		    << (1 + (unsigned int) bi);
						 // Fill color
		out << " -1 ";
						 // Depth 100 less than cells
		out << (xfig_flags.level_depth
			? (800-cell->level())
			: 800+bi)
						   // unused, no fill
		    << " 0 -1 0.0 "
						   // some style parameters
		    << " 0 0 -1 0 0 "
						   // number of points
		    << nvf << '\n';

						 // For each point, write scaled
						 // and shifted coordinates
						 // multiplied by 1200
						 // (dots/inch)

		for (unsigned int k=0;k<nvf;++k)
		  {
		    const Point<dim>& p = face->vertex(k % nv);
		    for (unsigned int d=0; d<static_cast<unsigned int>(dim); ++d)
		      {
			int val = (int)(1200 * xfig_flags.scaling(d) *
					(p(d)-xfig_flags.offset(d)));
			out << '\t' << val;
		      }
		    out << '\n';
		  }
	      }
	  }
    }

				   // make sure everything now gets to
				   // disk
  out.flush ();

  AssertThrow (out, ExcIO());
}





unsigned int GridOut::n_boundary_faces (const Triangulation<1> &) const
{
  return 0;
}

unsigned int GridOut::n_boundary_lines (const Triangulation<1> &) const
{
  return 0;
}


unsigned int GridOut::n_boundary_faces (const Triangulation<1,2> &) const
{
  return 0;
}

unsigned int GridOut::n_boundary_lines (const Triangulation<1,2> &) const
{
  return 0;
}

unsigned int GridOut::n_boundary_lines (const Triangulation<2> &) const
{
  return 0;
}

unsigned int GridOut::n_boundary_lines (const Triangulation<2,3> &) const
{
  return 0;
}



template <int dim, int spacedim>
unsigned int GridOut::n_boundary_faces (const Triangulation<dim,spacedim> &tria) const
{
    typename Triangulation<dim,spacedim>::active_face_iterator face, endf;
  unsigned int n_faces = 0;

  for (face=tria.begin_active_face(), endf=tria.end_face();
       face != endf; ++face)
    if ((face->at_boundary()) &&
	(face->boundary_indicator() != 0))
      n_faces++;

  return n_faces;
}



template <int dim, int spacedim>
unsigned int GridOut::n_boundary_lines (const Triangulation<dim, spacedim> &tria) const
{
    typename Triangulation<dim, spacedim>::active_line_iterator edge, endedge;
  unsigned int n_lines = 0;

  for (edge=tria.begin_active_line(), endedge=tria.end_line();
       edge != endedge; ++edge)
    if ((edge->at_boundary()) &&
	(edge->boundary_indicator() != 0))
      n_lines++;

  return n_lines;
}




void GridOut::write_msh_faces (const Triangulation<1> &,
			       const unsigned int,
			       std::ostream &) const
{
  return;
}


void GridOut::write_msh_faces (const Triangulation<1,2> &,
			       const unsigned int,
			       std::ostream &) const
{
  return;
}


void GridOut::write_msh_lines (const Triangulation<1> &,
			       const unsigned int,
			       std::ostream &) const
{
  return;
}

void GridOut::write_msh_lines (const Triangulation<1,2> &,
			       const unsigned int,
			       std::ostream &) const
{
  return;
}


void GridOut::write_msh_lines (const Triangulation<2> &,
			       const unsigned int,
			       std::ostream &) const
{
  return;
}

void GridOut::write_msh_lines (const Triangulation<2,3> &,
			       const unsigned int,
			       std::ostream &) const
{
  return;
}




template <int dim, int spacedim>
void GridOut::write_msh_faces (const Triangulation<dim,spacedim> &tria,
			       const unsigned int        starting_index,
			       std::ostream             &out) const
{
    typename Triangulation<dim,spacedim>::active_face_iterator face, endf;
  unsigned int index=starting_index;

  for (face=tria.begin_active_face(), endf=tria.end_face();
       face != endf; ++face)
    if (face->at_boundary() &&
	(face->boundary_indicator() != 0))
      {
	out << index << ' ';
	switch (dim)
	  {
	    case 2: out << 1 << ' ';  break;
	    case 3: out << 3 << ' ';  break;
	    default:
		  Assert (false, ExcNotImplemented());
	  };
	out << static_cast<unsigned int>(face->boundary_indicator())
	    << ' '
	    << static_cast<unsigned int>(face->boundary_indicator())
	    << ' ' << GeometryInfo<dim>::vertices_per_face;
					 // note: vertex numbers are 1-base
	for (unsigned int vertex=0; vertex<GeometryInfo<dim>::vertices_per_face; ++vertex)
	  out << ' '
	      << face->vertex_index(GeometryInfo<dim-1>::ucd_to_deal[vertex])+1;
	out << '\n';

	++index;
      };
}


template <int dim, int spacedim>
void GridOut::write_msh_lines (const Triangulation<dim, spacedim> &tria,
			       const unsigned int        starting_index,
			       std::ostream             &out) const
{
    typename Triangulation<dim,spacedim>::active_line_iterator line, endl;
  unsigned int index=starting_index;

  for (line=tria.begin_active_line(), endl=tria.end_line();
       line != endl; ++line)
    if (line->at_boundary() &&
	(line->boundary_indicator() != 0))
      {
	out << index << " 1 ";
	out << static_cast<unsigned int>(line->boundary_indicator())
	    << ' '
	    << static_cast<unsigned int>(line->boundary_indicator())
	    << " 2 ";
					 // note: vertex numbers are 1-base
	for (unsigned int vertex=0; vertex<2; ++vertex)
	  out << ' '
	      << line->vertex_index(GeometryInfo<dim-2>::ucd_to_deal[vertex])+1;
	out << '\n';

	++index;
      };
}




void GridOut::write_ucd_faces (const Triangulation<1> &,
			       const unsigned int,
			       std::ostream &) const
{
  return;
}

void GridOut::write_ucd_faces (const Triangulation<1,2> &,
			       const unsigned int,
			       std::ostream &) const
{
  return;
}

void GridOut::write_ucd_lines (const Triangulation<1> &,
			       const unsigned int,
			       std::ostream &) const
{
  return;
}

void GridOut::write_ucd_lines (const Triangulation<1,2> &,
			       const unsigned int,
			       std::ostream &) const
{
  return;
}


void GridOut::write_ucd_lines (const Triangulation<2> &,
			       const unsigned int,
			       std::ostream &) const
{
  return;
}

void GridOut::write_ucd_lines (const Triangulation<2,3> &,
			       const unsigned int,
			       std::ostream &) const
{
  return;
}




template <int dim, int spacedim>
void GridOut::write_ucd_faces (const Triangulation<dim, spacedim> &tria,
			       const unsigned int        starting_index,
			       std::ostream             &out) const
{
    typename Triangulation<dim,spacedim>::active_face_iterator face, endf;
  unsigned int index=starting_index;

  for (face=tria.begin_active_face(), endf=tria.end_face();
       face != endf; ++face)
    if (face->at_boundary() &&
	(face->boundary_indicator() != 0))
      {
	out << index << "  "
	    << static_cast<unsigned int>(face->boundary_indicator())
	    << "  ";
	switch (dim)
	  {
	    case 2: out << "line    ";  break;
	    case 3: out << "quad    ";  break;
	    default:
		  Assert (false, ExcNotImplemented());
	  };
					 // note: vertex numbers are 1-base
	for (unsigned int vertex=0; vertex<GeometryInfo<dim>::vertices_per_face; ++vertex)
	  out << face->vertex_index(GeometryInfo<dim-1>::ucd_to_deal[vertex])+1 << ' ';
	out << '\n';

	++index;
      };
}



template <int dim, int spacedim>
void GridOut::write_ucd_lines (const Triangulation<dim, spacedim> &tria,
			       const unsigned int        starting_index,
			       std::ostream             &out) const
{
    typename Triangulation<dim, spacedim>::active_line_iterator line, endl;
  unsigned int index=starting_index;

  for (line=tria.begin_active_line(), endl=tria.end_line();
       line != endl; ++line)
    if (line->at_boundary() &&
	(line->boundary_indicator() != 0))
      {
	out << index << "  "
	    << static_cast<unsigned int>(line->boundary_indicator())
	    << "  line    ";
	// note: vertex numbers are 1-base
	for (unsigned int vertex=0; vertex<2; ++vertex)
	  out << line->vertex_index(GeometryInfo<dim-2>::ucd_to_deal[vertex])+1 << ' ';
	out << '\n';

	++index;
      };
}


namespace internal
{
  namespace
  {
    template <int spacedim>
    void write_gnuplot (const dealii::Triangulation<1,spacedim> &tria,
			std::ostream             &out,
			const Mapping<1,spacedim>       *,
			const GridOutFlags::Gnuplot &gnuplot_flags)
    {
      AssertThrow (out, ExcIO());

      const int dim = 1;

      typename dealii::Triangulation<dim,spacedim>::active_cell_iterator
	cell=tria.begin_active();
      const typename dealii::Triangulation<dim,spacedim>::active_cell_iterator
	endc=tria.end();
      for (; cell!=endc; ++cell)
	{
	  if (gnuplot_flags.write_cell_numbers)
	    out << "# cell " << cell << '\n';

	  out << cell->vertex(0)
	      << ' ' << cell->level()
	      << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
	      << cell->vertex(1)
	      << ' ' << cell->level()
	      << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
	      << '\n';
	}

				       // make sure everything now gets to
				       // disk
      out.flush ();

      AssertThrow (out, ExcIO());
    }



    template <int spacedim>
    void write_gnuplot (const dealii::Triangulation<2,spacedim> &tria,
			std::ostream           &out,
			const Mapping<2,spacedim>       *mapping,
			const GridOutFlags::Gnuplot &gnuplot_flags)
    {
      AssertThrow (out, ExcIO());

      const int dim = 2;

      const unsigned int n_additional_points=
	gnuplot_flags.n_boundary_face_points;
      const unsigned int n_points=2+n_additional_points;

      typename dealii::Triangulation<dim,spacedim>::active_cell_iterator
	cell=tria.begin_active();
      const typename dealii::Triangulation<dim,spacedim>::active_cell_iterator
	endc=tria.end();

				       // if we are to treat curved
				       // boundaries, then generate a
				       // quadrature formula which will be
				       // used to probe boundary points at
				       // curved faces
      Quadrature<dim> *q_projector=0;
      std::vector<Point<dim-1> > boundary_points;
      if (mapping!=0)
	{
	  boundary_points.resize(n_points);
	  boundary_points[0][0]=0;
	  boundary_points[n_points-1][0]=1;
	  for (unsigned int i=1; i<n_points-1; ++i)
	    boundary_points[i](0)= 1.*i/(n_points-1);

	  std::vector<double> dummy_weights(n_points, 1./n_points);
	  Quadrature<dim-1> quadrature(boundary_points, dummy_weights);

	  q_projector = new Quadrature<dim> (QProjector<dim>::project_to_all_faces(quadrature));
	}

      for (; cell!=endc; ++cell)
	{
	  if (gnuplot_flags.write_cell_numbers)
	    out << "# cell " << cell << '\n';

	  if (mapping==0 ||
	      (!cell->at_boundary() && !gnuplot_flags.curved_inner_cells))
	    {
					       // write out the four sides
					       // of this cell by putting
					       // the four points (+ the
					       // initial point again) in
					       // a row and lifting the
					       // drawing pencil at the
					       // end
	      for (unsigned int i=0; i<GeometryInfo<dim>::vertices_per_cell; ++i)
		out << cell->vertex(GeometryInfo<dim>::ucd_to_deal[i])
		    << ' ' << cell->level()
		    << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n';
	      out << cell->vertex(0)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << '\n'  // double new line for gnuplot 3d plots
		  << '\n';
	    }
	  else
					     // cell is at boundary and we
					     // are to treat curved
					     // boundaries. so loop over
					     // all faces and draw them as
					     // small pieces of lines
	    {
	      for (unsigned int face_no=0;
		   face_no<GeometryInfo<dim>::faces_per_cell; ++face_no)
		{
		  const typename dealii::Triangulation<dim,spacedim>::face_iterator
		    face = cell->face(face_no);
		  if (face->at_boundary() || gnuplot_flags.curved_inner_cells)
		    {
						       // compute offset
						       // of quadrature
						       // points within
						       // set of projected
						       // points
		      const unsigned int offset=face_no*n_points;
		      for (unsigned int i=0; i<n_points; ++i)
			out << (mapping->transform_unit_to_real_cell
				(cell, q_projector->point(offset+i)))
			    << ' ' << cell->level()
			    << ' ' << static_cast<unsigned int>(cell->material_id())
			    << '\n';

		      out << '\n'
			  << '\n';
		    }
		  else
		    {
						       // if, however, the
						       // face is not at
						       // the boundary,
						       // then draw it as
						       // usual
		      out << face->vertex(0)
			  << ' ' << cell->level()
			  << ' ' << static_cast<unsigned int>(cell->material_id())
			  << '\n'
			  << face->vertex(1)
			  << ' ' << cell->level()
			  << ' ' << static_cast<unsigned int>(cell->material_id())
			  << '\n'
			  << '\n'
			  << '\n';
		    }
		}
	    }
	}

      if (q_projector != 0)
	delete q_projector;

				       // make sure everything now gets to
				       // disk
      out.flush ();

      AssertThrow (out, ExcIO());
    }



    template <int spacedim>
    void write_gnuplot (const dealii::Triangulation<3,spacedim> &tria,
			std::ostream           &out,
			const Mapping<3,spacedim>       *mapping,
			const GridOutFlags::Gnuplot &gnuplot_flags)
    {
      AssertThrow (out, ExcIO());

      const int dim = 3;

      const unsigned int n_additional_points=
	gnuplot_flags.n_boundary_face_points;
      const unsigned int n_points=2+n_additional_points;

      typename dealii::Triangulation<dim,spacedim>::active_cell_iterator
	cell=tria.begin_active();
      const typename dealii::Triangulation<dim,spacedim>::active_cell_iterator
	endc=tria.end();

				       // if we are to treat curved
				       // boundaries, then generate a
				       // quadrature formula which will be
				       // used to probe boundary points at
				       // curved faces
      Quadrature<dim> *q_projector=0;
      std::vector<Point<1> > boundary_points;
      if (mapping!=0)
	{
	  boundary_points.resize(n_points);
	  boundary_points[0][0]=0;
	  boundary_points[n_points-1][0]=1;
	  for (unsigned int i=1; i<n_points-1; ++i)
	    boundary_points[i](0)= 1.*i/(n_points-1);

	  std::vector<double> dummy_weights(n_points, 1./n_points);
	  Quadrature<1> quadrature1d(boundary_points, dummy_weights);

					   // tensor product of points,
					   // only one copy
	  QIterated<dim-1> quadrature(quadrature1d, 1);
	  q_projector = new Quadrature<dim> (QProjector<dim>::project_to_all_faces(quadrature));
	}

      for (; cell!=endc; ++cell)
	{
	  if (gnuplot_flags.write_cell_numbers)
	    out << "# cell " << cell << '\n';

	  if (mapping==0 || n_points==2 ||
	      (!cell->has_boundary_lines() && !gnuplot_flags.curved_inner_cells))
	    {
					       // front face
	      out << cell->vertex(0)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << cell->vertex(1)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << cell->vertex(5)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << cell->vertex(4)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << cell->vertex(0)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << '\n';
					       // back face
	      out << cell->vertex(2)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << cell->vertex(3)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << cell->vertex(7)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << cell->vertex(6)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << cell->vertex(2)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << '\n';

					       // now for the four connecting lines
	      out << cell->vertex(0)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << cell->vertex(2)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << '\n';
	      out << cell->vertex(1)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << cell->vertex(3)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << '\n';
	      out << cell->vertex(5)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << cell->vertex(7)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << '\n';
	      out << cell->vertex(4)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << cell->vertex(6)
		  << ' ' << cell->level()
		  << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
		  << '\n';
	    }
	  else
	    {
	      for (unsigned int face_no=0; face_no<GeometryInfo<dim>::faces_per_cell; ++face_no)
		{
		  const typename dealii::Triangulation<dim,spacedim>::face_iterator
		    face = cell->face(face_no);

		  if (face->at_boundary())
		    {
		      const unsigned int offset=face_no*n_points*n_points;
		      for (unsigned int i=0; i<n_points-1; ++i)
			for (unsigned int j=0; j<n_points-1; ++j)
			  {
			    const Point<spacedim> p0=mapping->transform_unit_to_real_cell(
			      cell, q_projector->point(offset+i*n_points+j));
			    out << p0
				<< ' ' << cell->level()
				<< ' ' << static_cast<unsigned int>(cell->material_id()) << '\n';
			    out << (mapping->transform_unit_to_real_cell(
				      cell, q_projector->point(offset+(i+1)*n_points+j)))
				<< ' ' << cell->level()
				<< ' ' << static_cast<unsigned int>(cell->material_id()) << '\n';
			    out << (mapping->transform_unit_to_real_cell(
				      cell, q_projector->point(offset+(i+1)*n_points+j+1)))
				<< ' ' << cell->level()
				<< ' ' << static_cast<unsigned int>(cell->material_id()) << '\n';
			    out << (mapping->transform_unit_to_real_cell(
				      cell, q_projector->point(offset+i*n_points+j+1)))
				<< ' ' << cell->level()
				<< ' ' << static_cast<unsigned int>(cell->material_id()) << '\n';
							     // and the
							     // first
							     // point
							     // again
			    out << p0
				<< ' ' << cell->level()
				<< ' ' << static_cast<unsigned int>(cell->material_id()) << '\n';
			    out << '\n' << '\n';
			  }
		    }
		  else
		    {
		      for (unsigned int l=0; l<GeometryInfo<dim>::lines_per_face; ++l)
			{
			  const typename dealii::Triangulation<dim,spacedim>::line_iterator
			    line=face->line(l);

			  const Point<spacedim> &v0=line->vertex(0),
						&v1=line->vertex(1);
			  if (line->at_boundary() || gnuplot_flags.curved_inner_cells)
			    {
							       // transform_real_to_unit_cell
							       // could be
							       // replaced
							       // by using
							       // QProjector<dim>::project_to_line
							       // which is
							       // not yet
							       // implemented
			      const Point<spacedim> u0=mapping->transform_real_to_unit_cell(cell, v0),
						    u1=mapping->transform_real_to_unit_cell(cell, v1);

			      for (unsigned int i=0; i<n_points; ++i)
				out << (mapping->transform_unit_to_real_cell
					(cell, (1-boundary_points[i][0])*u0+boundary_points[i][0]*u1))
				    << ' ' << cell->level()
				    << ' ' << static_cast<unsigned int>(cell->material_id()) << '\n';
			    }
			  else
			    out << v0
				<< ' ' << cell->level()
				<< ' ' << static_cast<unsigned int>(cell->material_id()) << '\n'
				<< v1
				<< ' ' << cell->level()
				<< ' ' << static_cast<unsigned int>(cell->material_id()) << '\n';

			  out << '\n' << '\n';
			}
		    }
		}
	    }
	}

      if (q_projector != 0)
	delete q_projector;


				       // make sure everything now gets to
				       // disk
      out.flush ();

      AssertThrow (out, ExcIO());
    }
  }
}



template <int dim, int spacedim>
void GridOut::write_gnuplot (
  const Triangulation<dim,spacedim> &tria,
  std::ostream             &out,
  const Mapping<dim,spacedim>       *mapping) const
{
  internal::write_gnuplot (tria, out, mapping, gnuplot_flags);
}



namespace internal
{
  namespace
  {
    struct LineEntry
    {
	Point<2> first;
	Point<2> second;
	bool colorize;
	unsigned int level;
	LineEntry (const Point<2>    &f,
		   const Point<2>    &s,
		   const bool         c,
		   const unsigned int l)
			:
			first(f), second(s),
			colorize(c), level(l)
	  {}
    };


    void write_eps (const dealii::Triangulation<1> &,
		    std::ostream &,
		    const Mapping<1> *,
		    const GridOutFlags::Eps<2> &,
		    const GridOutFlags::Eps<3> &)
    {
      Assert(false, ExcNotImplemented());
    }



    template <int dim>
    void write_eps (const dealii::Triangulation<dim> &tria,
		    std::ostream             &out,
		    const Mapping<dim>       *mapping,
		    const GridOutFlags::Eps<2> &eps_flags_2,
		    const GridOutFlags::Eps<3> &eps_flags_3)
    {
      typedef std::list<LineEntry> LineList;

				       // get a pointer to the flags
				       // common to all dimensions, in
				       // order to avoid the recurring
				       // distinctions between
				       // eps_flags_1, eps_flags_2, ...
      const GridOutFlags::EpsFlagsBase
	&eps_flags_base = (dim==2 ?
			   static_cast<const GridOutFlags::EpsFlagsBase&>(eps_flags_2) :
			   (dim==3 ?
			    static_cast<const GridOutFlags::EpsFlagsBase&>(eps_flags_3) :
			    *static_cast<const GridOutFlags::EpsFlagsBase*>(0)));

      AssertThrow (out, ExcIO());
      const unsigned int n_points = eps_flags_base.n_boundary_face_points;

				       // make up a list of lines by which
				       // we will construct the triangulation
				       //
				       // this part unfortunately is a bit
				       // dimension dependent, so we have to
				       // treat every dimension different.
				       // however, by directly producing
				       // the lines to be printed, i.e. their
				       // 2d images, we can later do the
				       // actual output dimension independent
				       // again
      LineList line_list;

      switch (dim)
	{
	  case 1:
	  {
	    Assert(false, ExcInternalError());
	    break;
	  };

	  case 2:
	  {
	    typename dealii::Triangulation<dim>::active_cell_iterator
	      cell=tria.begin_active(),
	      endc=tria.end();

	    for (; cell!=endc; ++cell)
	      for (unsigned int line_no=0;
		   line_no<GeometryInfo<dim>::lines_per_cell; ++line_no)
		{
		  typename dealii::Triangulation<dim>::line_iterator
		    line=cell->line(line_no);

						   // first treat all
						   // interior lines and
						   // make up a list of
						   // them. if curved
						   // lines shall not be
						   // supported (i.e. no
						   // mapping is
						   // provided), then also
						   // treat all other
						   // lines
		  if (!line->has_children() &&
		      (mapping==0 || !line->at_boundary()))
						     // one would expect
						     // make_pair(line->vertex(0),
						     //           line->vertex(1))
						     // here, but that is
						     // not dimension
						     // independent, since
						     // vertex(i) is
						     // Point<dim>, but we
						     // want a Point<2>.
						     // in fact, whenever
						     // we're here, the
						     // vertex is a
						     // Point<dim>, but
						     // the compiler does
						     // not know
						     // this. hopefully,
						     // the compiler will
						     // optimize away this
						     // little kludge
		    line_list.push_back (LineEntry(Point<2>(line->vertex(0)(0),
							    line->vertex(0)(1)),
						   Point<2>(line->vertex(1)(0),
							    line->vertex(1)(1)),
						   line->user_flag_set(),
						   cell->level()));
		}

					     // next if we are to treat
					     // curved boundaries
					     // specially, then add lines
					     // to the list consisting of
					     // pieces of the boundary
					     // lines
	    if (mapping!=0)
	      {
						 // to do so, first
						 // generate a sequence of
						 // points on a face and
						 // project them onto the
						 // faces of a unit cell
		std::vector<Point<dim-1> > boundary_points (n_points);

		for (unsigned int i=0; i<n_points; ++i)
		  boundary_points[i](0) = 1.*(i+1)/(n_points+1);

		Quadrature<dim-1> quadrature (boundary_points);
		Quadrature<dim>   q_projector (QProjector<dim>::project_to_all_faces(quadrature));

						 // next loop over all
						 // boundary faces and
						 // generate the info from
						 // them
		typename dealii::Triangulation<dim>::active_cell_iterator
		  cell=tria.begin_active ();
		const typename dealii::Triangulation<dim>::active_cell_iterator
		  end=tria.end ();
		for (; cell!=end; ++cell)
		  for (unsigned int face_no=0; face_no<GeometryInfo<dim>::faces_per_cell; ++face_no)
		    {
		      const typename dealii::Triangulation<dim>::face_iterator
			face = cell->face(face_no);

		      if (face->at_boundary())
			{
			  Point<dim> p0_dim(face->vertex(0));
			  Point<2>   p0    (p0_dim(0), p0_dim(1));

							   // loop over
							   // all pieces
							   // of the line
							   // and generate
							   // line-lets
			  const unsigned int offset=face_no*n_points;
			  for (unsigned int i=0; i<n_points; ++i)
			    {
			      const Point<dim> p1_dim (mapping->transform_unit_to_real_cell
						       (cell, q_projector.point(offset+i)));
			      const Point<2>   p1     (p1_dim(0), p1_dim(1));

			      line_list.push_back (LineEntry(p0, p1,
							     face->user_flag_set(),
							     cell->level() ));
			      p0=p1;
			    }

							   // generate last piece
			  const Point<dim> p1_dim (face->vertex(1));
			  const Point<2>   p1     (p1_dim(0), p1_dim(1));
			  line_list.push_back (LineEntry(p0, p1,
							 face->user_flag_set(),
							 cell->level()));
			};
		    };
	      };

	    break;
	  };

	  case 3:
	  {
					     // curved boundary output
					     // presently not supported
	    Assert (mapping == 0, ExcNotImplemented());

	    typename dealii::Triangulation<dim>::active_cell_iterator
	      cell=tria.begin_active(),
	      endc=tria.end();

					     // loop over all lines and compute their
					     // projection on the plane perpendicular
					     // to the direction of sight

					     // direction of view equals the unit
					     // vector of the position of the
					     // spectator to the origin.
					     //
					     // we chose here the viewpoint as in
					     // gnuplot as default.
					     //
					     //TODO:[WB] Fix a potential problem with viewing angles in 3d Eps GridOut
					     // note: the following might be wrong
					     // if one of the base vectors below
					     // is in direction of the viewer, but
					     // I am too tired at present to fix
					     // this
	    const double pi = numbers::PI;
	    const double z_angle    = eps_flags_3.azimut_angle;
	    const double turn_angle = eps_flags_3.turn_angle;
	    const Point<dim> view_direction(-std::sin(z_angle * 2.*pi / 360.) * std::sin(turn_angle * 2.*pi / 360.),
					    +std::sin(z_angle * 2.*pi / 360.) * std::cos(turn_angle * 2.*pi / 360.),
					    -std::cos(z_angle * 2.*pi / 360.));

					     // decide about the two unit vectors
					     // in this plane. we chose the first one
					     // to be the projection of the z-axis
					     // to this plane
	    const Point<dim> vector1
	      = Point<dim>(0,0,1) - ((Point<dim>(0,0,1) * view_direction) * view_direction);
	    const Point<dim> unit_vector1 = vector1 / std::sqrt(vector1.square());

					     // now the third vector is fixed. we
					     // chose the projection of a more or
					     // less arbitrary vector to the plane
					     // perpendicular to the first one
	    const Point<dim> vector2
	      = (Point<dim>(1,0,0)
		 - ((Point<dim>(1,0,0) * view_direction) * view_direction)
		 - ((Point<dim>(1,0,0) * unit_vector1)   * unit_vector1));
	    const Point<dim> unit_vector2 = vector2 / std::sqrt(vector2.square());


	    for (; cell!=endc; ++cell)
	      for (unsigned int line_no=0;
		   line_no<GeometryInfo<dim>::lines_per_cell; ++line_no)
		{
		  typename dealii::Triangulation<dim>::line_iterator
		    line=cell->line(line_no);
		  line_list.push_back (LineEntry(Point<2>(line->vertex(0) * unit_vector2,
							  line->vertex(0) * unit_vector1),
						 Point<2>(line->vertex(1) * unit_vector2,
							  line->vertex(1) * unit_vector1),
						 line->user_flag_set(),
						 cell->level()));
		}

	    break;
	  }

	  default:
		Assert (false, ExcNotImplemented());
	}



				       // find out minimum and maximum x and
				       // y coordinates to compute offsets
				       // and scaling factors
      double x_min = tria.begin_active_line()->vertex(0)(0);
      double x_max = x_min;
      double y_min = tria.begin_active_line()->vertex(0)(1);
      double y_max = y_min;
      unsigned int  max_level = line_list.begin()->level;

      for (LineList::const_iterator line=line_list.begin();
	   line!=line_list.end(); ++line)
	{
	  x_min = std::min (x_min, line->first(0));
	  x_min = std::min (x_min, line->second(0));

	  x_max = std::max (x_max, line->first(0));
	  x_max = std::max (x_max, line->second(0));

	  y_min = std::min (y_min, line->first(1));
	  y_min = std::min (y_min, line->second(1));

	  y_max = std::max (y_max, line->first(1));
	  y_max = std::max (y_max, line->second(1));

	  max_level = std::max (max_level,  line->level);
	};

				       // scale in x-direction such that
				       // in the output 0 <= x <= 300.
				       // don't scale in y-direction to
				       // preserve the shape of the
				       // triangulation
      const double scale = (eps_flags_base.size /
			    (eps_flags_base.size_type==GridOutFlags::EpsFlagsBase::width ?
			     x_max - x_min :
			     y_min - y_max));


				       // now write preamble
      if (true)
	{
					   // block this to have local
					   // variables destroyed after
					   // use
	  std::time_t  time1= std::time (0);
	  std::tm     *time = std::localtime(&time1);
	  out << "%!PS-Adobe-2.0 EPSF-1.2" << '\n'
	      << "%%Title: deal.II Output" << '\n'
	      << "%%Creator: the deal.II library" << '\n'
	      << "%%Creation Date: "
	      << time->tm_year+1900 << "/"
	      << time->tm_mon+1 << "/"
	      << time->tm_mday << " - "
	      << time->tm_hour << ":"
	      << std::setw(2) << time->tm_min << ":"
	      << std::setw(2) << time->tm_sec << '\n'
	      << "%%BoundingBox: "
					     // lower left corner
	      << "0 0 "
					     // upper right corner
	      << static_cast<unsigned int>(std::floor(( (x_max-x_min) * scale )+1))
	      << ' '
	      << static_cast<unsigned int>(std::floor(( (y_max-y_min) * scale )+1))
	      << '\n';

					   // define some abbreviations to keep
					   // the output small:
					   // m=move turtle to
					   // x=execute line stroke
					   // b=black pen
					   // r=red pen
	  out << "/m {moveto} bind def" << '\n'
	      << "/x {lineto stroke} bind def" << '\n'
	      << "/b {0 0 0 setrgbcolor} def" << '\n'
	      << "/r {1 0 0 setrgbcolor} def" << '\n';

					   // calculate colors for level
					   // coloring; level 0 is black,
					   // other levels are blue
					   // ... red
	  if (eps_flags_base.color_lines_level)
	    out  << "/l  { neg "
		 << (max_level)
		 << " add "
		 << (0.66666/std::max(1U,(max_level-1)))
		 << " mul 1 0.8 sethsbcolor} def" << '\n';

					   // in 2d, we can also plot cell
					   // and vertex numbers, but this
					   // requires a somewhat more
					   // lengthy preamble. please
					   // don't ask me what most of
					   // this means, it is reverse
					   // engineered from what GNUPLOT
					   // uses in its output
	  if ((dim == 2) && (eps_flags_2.write_cell_numbers ||
			     eps_flags_2.write_vertex_numbers))
	    {
	      out << ("/R {rmoveto} bind def\n"
		      "/Symbol-Oblique /Symbol findfont [1 0 .167 1 0 0] makefont\n"
		      "dup length dict begin {1 index /FID eq {pop pop} {def} ifelse} forall\n"
		      "currentdict end definefont\n"
		      "/MFshow {{dup dup 0 get findfont exch 1 get scalefont setfont\n"
		      "[ currentpoint ] exch dup 2 get 0 exch rmoveto dup dup 5 get exch 4 get\n"
		      "{show} {stringwidth pop 0 rmoveto}ifelse dup 3 get\n"
		      "{2 get neg 0 exch rmoveto pop} {pop aload pop moveto}ifelse} forall} bind def\n"
		      "/MFwidth {0 exch {dup 3 get{dup dup 0 get findfont exch 1 get scalefont setfont\n"
		      "5 get stringwidth pop add}\n"
		      "{pop} ifelse} forall} bind def\n"
		      "/MCshow { currentpoint stroke m\n"
		      "exch dup MFwidth -2 div 3 -1 roll R MFshow } def\n")
		  << '\n';
	    };

	  out << "%%EndProlog" << '\n'
	      << '\n';

					   // set fine lines
	  out << eps_flags_base.line_width << " setlinewidth" << '\n';
	};

				       // now write the lines
      const Point<2> offset(x_min, y_min);

      for (LineList::const_iterator line=line_list.begin();
	   line!=line_list.end(); ++line)
	if (eps_flags_base.color_lines_level && (line->level > 0))
					   // lines colored according to
					   // refinement level,
					   // contributed by J�rg
					   // R. Weimar
	  out << line->level
	      << " l "
	      << (line->first  - offset) * scale << " m "
	      << (line->second - offset) * scale << " x" << '\n';
	else
	  out << ((line->colorize && eps_flags_base.color_lines_on_user_flag) ? "r " : "b ")
	      << (line->first  - offset) * scale << " m "
	      << (line->second - offset) * scale << " x" << '\n';

				       // finally write the cell numbers
				       // in 2d, if that is desired
      if ((dim == 2) && (eps_flags_2.write_cell_numbers == true))
	{
	  out << "(Helvetica) findfont 140 scalefont setfont"
	      << '\n';

	  typename dealii::Triangulation<dim>::active_cell_iterator
	    cell = tria.begin_active (),
	    endc = tria.end ();
	  for (; cell!=endc; ++cell)
	    {
	      out << (cell->center()(0)-offset(0))*scale << ' '
		  << (cell->center()(1)-offset(1))*scale
		  << " m" << '\n'
		  << "[ [(Helvetica) 12.0 0.0 true true (";
	      if (eps_flags_2.write_cell_number_level)
		out << cell;
	      else
		out << cell->index();

	      out << ")] "
		  << "] -6 MCshow"
		  << '\n';
	    };
	};

				       // and the vertex numbers
      if ((dim == 2) && (eps_flags_2.write_vertex_numbers == true))
	{
	  out << "(Helvetica) findfont 140 scalefont setfont"
	      << '\n';

					   // have a list of those
					   // vertices which we have
					   // already tracked, to avoid
					   // doing this multiply
	  std::set<unsigned int> treated_vertices;
	  typename dealii::Triangulation<dim>::active_cell_iterator
	    cell = tria.begin_active (),
	    endc = tria.end ();
	  for (; cell!=endc; ++cell)
	    for (unsigned int vertex=0;
		 vertex<GeometryInfo<dim>::vertices_per_cell;
		 ++vertex)
	      if (treated_vertices.find(cell->vertex_index(vertex))
		  ==
		  treated_vertices.end())
		{
		  treated_vertices.insert (cell->vertex_index(vertex));

		  out << (cell->vertex(vertex)(0)-offset(0))*scale << ' '
		      << (cell->vertex(vertex)(1)-offset(1))*scale
		      << " m" << '\n'
		      << "[ [(Helvetica) 10.0 0.0 true true ("
		      << cell->vertex_index(vertex)
		      << ")] "
		      << "] -6 MCshow"
		      << '\n';
		};
	};

      out << "showpage" << '\n';

				       // make sure everything now gets to
				       // disk
      out.flush ();

      AssertThrow (out, ExcIO());
    }
  }
}


template <int dim>
void GridOut::write_eps (const Triangulation<dim> &tria,
			 std::ostream             &out,
			 const Mapping<dim>       *mapping) const
{
  internal::write_eps (tria, out, mapping,
		       eps_flags_2, eps_flags_3);
}


template <int dim, int spacedim>
void GridOut::write (const Triangulation<dim, spacedim> &tria,
		     std::ostream             &out,
		     const OutputFormat        output_format,
		     const Mapping<dim,spacedim>       *mapping) const
{
  switch (output_format)
    {
      case none:
	    return;

      case dx:
	    write_dx (tria, out);
	    return;

      case ucd:
	    write_ucd (tria, out);
	    return;

      case gnuplot:
	    write_gnuplot (tria, out, mapping);
	    return;

      case eps:
	    write_eps (tria, out, mapping);
	    return;

      case xfig:
	    write_xfig (tria, out, mapping);
	    return;

      case msh:
	    write_msh (tria, out);
	    return;
    }

  Assert (false, ExcInternalError());
}


template <int dim, int spacedim>
void GridOut::write (
    const Triangulation<dim, spacedim> &tria,
  std::ostream             &out,
  const Mapping<dim,spacedim>       *mapping) const
{
  write(tria, out, default_format, mapping);
}


// explicit instantiations
#include "grid_out.inst"


DEAL_II_NAMESPACE_CLOSE
