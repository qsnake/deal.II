//----------------------------  make_dependencies.cc  ------------------------
//    $Id: make_dependencies.cc 22940 2010-12-08 02:55:59Z bangerth $
//    Version: $Name$
//
//    Copyright (C) 2003, 2007, 2008, 2010 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  make_dependencies.cc  ------------------------

// Make a dependency file tree
// usage: make_depencies -Iinc_path1 -Iinc_path2 ... -Bbasepath files

// This program is basically a replacement for something like "gcc
// -M", i.e. it generates for each input file a list of other files it
// depends on by direct or indirect inclusion (or at least those files
// that can be found in the directories specified by -I/path/.../
// flags on the command line). The difference to gcc -M is that it is
// much faster, since it doesn't really do much parsing except for
// finding those lines that have a #include at the beginning
//
// The output looks like this:
//
//   $basepath/.o-file $basepath/.g.o-file: file included_files
//
// $basepath is the dir where the object files are to be placed (as
// given by the -B parameter to this script). if no path is given, no
// path is printed. if one is given, a slash is appended if necessary

// If the -n (for "new-style") flag is passed, then the output format
//   $basepath/optimized/.o-file $basepath/debug/.o-file: file included_files
// is used.

// Author: Wolfgang Bangerth, 2003 (and based on a previous perl
// script written 1998, 1999, 2000, 2001, 2002)


#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <sys/stat.h>

                                 // base path for object files,
                                 // including trailing slash if
                                 // non-empty
std::string basepath;

                                 // list of include directories
std::vector<std::string> include_directories;

                                 // for each file that we ever visit,
                                 // store the set of other files it
                                 // includes directly
std::map<std::string,std::set<std::string> > direct_includes;

                                 // A variable that stores whether
                                 // we want old-style or new-style format
enum Format
  {
    old_style, new_style
  } format;


                                 // for the given file, fill a
                                 // respective entry in the "direct_includes"
                                 // map listing the names of those
                                 // files that are directly included
void determine_direct_includes (const std::string &file)
{
                                   // if this file has already been
                                   // treated, then leave it at this
  if (direct_includes.find (file) != direct_includes.end())
    return;

                                   // otherwise, open the file and go
                                   // through it line by line to
                                   // search for other includes. we
                                   // will have to have the path to
                                   // the present file later, so get
                                   // it already here
  std::string present_path;
  if (file.find ('/') != std::string::npos)
    present_path = std::string (file.begin(),
				file.begin()+file.rfind ('/')+1);

  std::ifstream in(file.c_str());
  assert ((bool)in);

  std::string line;
  while (in)
    {
                                       // get one line, eat whitespace
                                       // at the beginning and see
                                       // whether the first
                                       // non-whitespace is a #
                                       // character
      getline (in, line);
      unsigned int pos=0;
      for (; pos<line.length(); ++pos)
        if ((line[pos] != ' ') && (line[pos] != '\t'))
          break;

                                       // if no non-whitespace, or
                                       // something other than #: next
                                       // line
      if ((pos == line.length()) || (line[pos] != '#'))
        continue;

                                       // ok, this is a preprocessor
                                       // line. eat pound sign and
                                       // again the next couple of
                                       // whitespaces
      ++pos;
      for (; pos<line.length(); ++pos)
        if ((line[pos] != ' ') && (line[pos] != '\t'))
          break;

                                       // and let's see whether the
                                       // following is the word
                                       // include
      if ((line.length() < pos+7)
          ||
          (! ((line[pos+0] == 'i') &&
              (line[pos+1] == 'n') &&
              (line[pos+2] == 'c') &&
              (line[pos+3] == 'l') &&
              (line[pos+4] == 'u') &&
              (line[pos+5] == 'd') &&
              (line[pos+6] == 'e'))))
        continue;

                                       // ok, word found. advance pos
                                       // and eat more whitespace
      pos += 7;
      for (; pos<line.length(); ++pos)
        if ((line[pos] != ' ') && (line[pos] != '\t'))
          break;

                                       // check that the next char is
                                       // either '<' or '"'
      if ((line[pos] != '"') && (line[pos] != '<'))
        continue;

                                       // copy out name
      std::string included_file;
      for (unsigned int endpos=pos+1; endpos<line.length(); ++endpos)
        if ((line[endpos]=='"') || (line[endpos] == '>'))
          {
            included_file = std::string (line.begin()+pos+1,
                                         line.begin()+endpos);
            break;
          }
      if (included_file.length() == 0)
	{
	  std::cerr << "Include file name empty in file <" << file << "> line: " << std::endl
		    << line << std::endl;
	  std::abort();
	}

                                       // next try to locate the file
                                       // in absolute paths. this is
                                       // easy if it was included via
                                       // "...", but for <...> we have
                                       // to work a little harder
      if (included_file[0] != '/')
        {
          if (line[pos] == '"')
            included_file = present_path+included_file;
          else
            for (std::vector<std::string>::const_iterator
                   include_dir=include_directories.begin();
                 include_dir!=include_directories.end(); ++include_dir)
	      {
		struct stat buf;
		int error = stat ((*include_dir+included_file).c_str(), &buf);

		if ((error == 0) &&
		    S_ISREG(buf.st_mode))
		  {
		    included_file = *include_dir+included_file;
		    break;
		  }
	      }
        }

                                       // make sure the file exists
                                       // and that we can read from
                                       // it, otherwise just ignore
                                       // the line
      {
	struct stat buf;
	int error = stat (included_file.c_str(), &buf);

	if ((error != 0) ||
	    !S_ISREG(buf.st_mode))
	  continue;
      }



                                       // ok, so we did find an
                                       // appropriate file. add it to
                                       // the correct list
      direct_includes[file].insert (included_file);

                                       // work on the include file
                                       // recursively. note that the
                                       // first line of this file
                                       // saves us from infinite
                                       // recursions in case of
                                       // include loops
      determine_direct_includes (included_file);
    }
}




                                 // return the set of all included
                                 // files, directly or indirectly, for
                                 // the given file. for this purpose,
                                 // we consider the direct_includes
                                 // variable to be a representation of
                                 // a directed graph: given a file (a
                                 // node in the graph), the elements
                                 // of the include-set for this file
                                 // are outgoing edges from this
                                 // node. to get at all includes,
                                 // direct and indirect, we keep a
                                 // list starting with the direct
                                 // includes, the collect all direct
                                 // includes of the files in the list,
                                 // then their include files, etc. We
                                 // thus march in fronts through the
                                 // graph.
                                 //
                                 // one of the complications we have
                                 // to keep track of is that the graph
                                 // may be cyclic (i.e. header files
                                 // include each other mutually -- in
                                 // the real program, one of the
                                 // includes will be guarded by
                                 // preprocessor #ifdefs, but we don't
                                 // see them here), so we have to make
                                 // sure we strip elements from the
                                 // present front that we have already
                                 // visited before
                                 //
                                 // this function could presumably be
                                 // made more efficient in the
                                 // following way: when we have more
                                 // than one file for which we want to
                                 // compute dependencies, we presently
                                 // walk through the graph for each of
                                 // them. however, they will likely
                                 // have one or more includes in
                                 // common, so they will also have
                                 // parts of the dependency graph in
                                 // common. if we could precompute the
                                 // dependency graph for include files
                                 // in advance, we wouldn't have to
                                 // walk through _all_ the graph for
                                 // each file we consider, but could
                                 // just draw in blocks. the problem
                                 // with that is that to make this
                                 // efficient we cannot just compute
                                 // the whole set of dependencies for
                                 // _each_ file, but we have to do
                                 // this on the fly and to avoid again
                                 // problems with the cyclic nature we
                                 // have to keep track where we are
                                 // presently coming from. that's way
                                 // too complicated for now and I
                                 // leave it to times when dependency
                                 // generation becomes again a
                                 // noticable time hit
                                 //
                                 // one of the ideas to solve this
                                 // problem would be to start at
                                 // terminal nodes of the graph
                                 // (i.e. nodes that have no outgoing
                                 // edges) and fold these nodes into
                                 // nodes that have only outgoing
                                 // edges to terminal nodes. store the
                                 // dependencies of these
                                 // next-to-terminal nodes and remove
                                 // the terminal ones. then start over
                                 // again with the so generated
                                 // graph. if we consider the files
                                 // for which we want to compute
                                 // dependency information as top
                                 // level nodes (they will _only_ have
                                 // outgoing nodes), we could
                                 // presumable roll up the entire
                                 // graph from the bottom (terminal
                                 // nodes) and fold it one layer at a
                                 // time
                                 //
                                 // in any case, there is presently no
                                 // need for this since the actions of
                                 // this function are presently not
                                 // really time critical: parsing the
                                 // files in the function above is a
                                 // much bigger time-hit.
std::set<std::string>
get_all_includes (const std::string &name)
{
                                   // start with direct includes
  std::set<std::string> all_includes = direct_includes[name];

  std::set<std::string> next_level_includes = all_includes;
  while (true)
    {
                                       // traverse all next level
                                       // includes and get their
                                       // direct include files. the
                                       // set makes sure that
                                       // duplicates are removed
      std::set<std::string> second_next;
      for (std::set<std::string>::const_iterator
             next=next_level_includes.begin();
           next!=next_level_includes.end(); ++next)
        second_next.insert (direct_includes[*next].begin(),
                            direct_includes[*next].end());

                                       // for each of them, if it
                                       // hasn't been treated then add
                                       // it to the files of the next
                                       // level and later add it to
                                       // the all_includes
      next_level_includes.clear ();
      for (std::set<std::string>::const_iterator f=second_next.begin();
           f!=second_next.end(); ++f)
        if (all_includes.find(*f) == all_includes.end())
          next_level_includes.insert (*f);

                                       // if no new includes found no
                                       // more, then quit
      if (next_level_includes.size() == 0)
        return all_includes;

                                       // otherwise, copy over and
                                       // start over on the next level
                                       // of the tree
      all_includes.insert (next_level_includes.begin(),
                           next_level_includes.end());
    }
}



int main (int argc, char **argv)
{
  std::vector<std::string> filenames;

                                   // parse all arguments (except the
                                   // name of the executable itself)
  for (unsigned int c=1; c<static_cast<unsigned int>(argc); ++c)
    {
      const std::string arg = argv[c];

                                       // if string starts with -I,
                                       // take this as an include path
      if ((arg.length()>2) && (arg[0]=='-') && (arg[1]=='I'))
        {
          std::string dir (arg.begin()+2, arg.end());

                                           // append a slash if not
                                           // already there
          if (dir[dir.length()-1] != '/')
            dir += '/';

                                           // drop initial ./ if this
                                           // is there
          if ((dir[0]=='.') && (dir[1]=='/'))
            dir = std::string(dir.begin()+2, dir.end());

          include_directories.push_back (dir);
        }
                                       // if string starts with -B,
                                       // then this is the base name
                                       // for object files
      else if ((arg.length()>2) && (arg[0]=='-') && (arg[1]=='B'))
        {
          basepath = std::string(arg.begin()+2, arg.end());
          if (basepath[basepath.size()-1] != '/')
            basepath += '/';
        }
                                       // if string is -n,
                                       // then use new-style format
      else if ((arg.length()==2) && (arg[0]=='-') && (arg[1]=='n'))
	format = new_style;

                                       // otherwise assume that this
                                       // is one of the files for
                                       // input
      else
        {
          assert (arg.size()>=1);
          assert (arg[0] != '-');

          filenames.push_back (arg);
        }
    }

                                   // next iterate through all files
                                   // and figure out which other files
                                   // they include
  for (std::vector<std::string>::const_iterator file=filenames.begin();
       file != filenames.end(); ++file)
    determine_direct_includes (*file);


                                   // now we have all files that are
                                   // directly or indirectly included
                                   // into the files given on the
                                   // command lines. for each of them,
                                   // we have recorded which files
                                   // they include themselves. for
                                   // each file on the command line,
                                   // we can thus form a complete
                                   // include tree.
  for (std::vector<std::string>::const_iterator file=filenames.begin();
       file != filenames.end(); ++file)
    {
                                       // get base of filename by
                                       // chipping away .cc extension
                                       // as well as path
      std::string basename;
      if (file->find (".cc") != std::string::npos)
        basename = std::string (file->begin(),
                                file->begin()+file->find (".cc"));
      else if (file->find (".cpp") != std::string::npos)
        basename = std::string (file->begin(),
                                file->begin()+file->find (".cpp"));
      else
        basename = *file;

      if (basename.rfind ("/") != std::string::npos)
        basename = std::string (basename.begin()+basename.rfind("/")+1,
                                basename.end());

                                       // get all direct and indirect
                                       // includes for this file...
      const std::set<std::string> includes = get_all_includes (*file);

                                       // ...write the rule for the .o
                                       // file...
      if (format == old_style)
	std::cout << basepath << basename << ".o: \\"
		  << std::endl
		  << "\t\t" << *file;
      else
	std::cout << basepath << "optimized/" << basename << ".o: \\"
		  << std::endl
		  << "\t\t" << *file;
      for (std::set<std::string>::const_iterator i=includes.begin();
           i!=includes.end(); ++i)
        std::cout << "\\\n\t\t" << *i;
      std::cout << std::endl;

                                       // ...and a similar rule for
                                       // the .o file in debug mode
      if (format == old_style)
	std::cout << basepath << basename << ".g.o: \\"
		  << std::endl
		  << "\t\t" << *file;
      else
	std::cout << basepath << "debug/" << basename << ".o: \\"
		  << std::endl
		  << "\t\t" << *file;
      for (std::set<std::string>::const_iterator i=includes.begin();
           i!=includes.end(); ++i)
        std::cout << "\\\n\t\t" << *i;
      std::cout << std::endl;
    }
}

