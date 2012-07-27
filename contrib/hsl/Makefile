# $Id: Makefile 22430 2010-10-23 13:41:09Z heister $
# Copyright (C) 2001, 2002, 2003, 2004, 2006, 2008, 2009, 2010 by Wolfgang Bangerth


D = ../..

include $D/common/Make.global_options

# get lists of files we need
f-files     = $(shell a="`echo source/*.f`"; if test "$$a" != "source/*.f" ; then echo $$a ; fi)
o-files     = $(f-files:source/%.f=$(LIBDIR)/contrib/hsl/%.$(OBJEXT))




# rules
lib: $(LIBDIR)/libhsl$(lib-suffix) $(LIBDIR)/bin/detached_ma27$(EXEEXT)



$(LIBDIR)/contrib/hsl/%.$(OBJEXT) :
	@echo =====hsl==============optimized==$(MT)== $(<F)
	@$(F77) $(F77FLAGS.o) -c $< -o $@


$(LIBDIR)/contrib/hsl/detached_ma27.$(OBJEXT) : source/detached_ma27.cc \
						include/hsl/hsl.h
	@echo =====hsl==============optimized==$(MT)== $(<F)
	@$(CXX) $(CXXFLAGS.g) -c $< -o $@


$(LIBDIR)/libhsl$(static-lib-suffix): $(forward-declarations) $(o-files)
	@echo =====hsl==============optimized==$(MT)== Linking library:   $(@F)
	@$(AR) ru $@ $(o-files)
	@$(RANLIB) $@

# link the object files to a shared library.
$(LIBDIR)/libhsl$(shared-lib-suffix): $(forward-declarations) $(o-files)
	@echo =====hsl==============optimized==$(MT)== Linking library:   $(@F)
	@$(SHLIBLD) $(LDFLAGS) -shared -o $(LIBDIR)/$(call DEAL_II_SHLIB_NAME,hsl) $(call DEAL_II_ADD_SONAME,hsl) $(o-files) $(F77LIBS)
	@ln -f -s $(call DEAL_II_SHLIB_NAME,hsl) $@



# rule to make the program that runs an MA27 solver detached from the
# main program, and communicates through a pipe. since this program
# has actually nothing much to do, compile it in debug mode
$(LIBDIR)/bin/detached_ma27$(EXEEXT):  \
				$(LIBDIR)/contrib/hsl/detached_ma27.$(OBJEXT) \
				$(LIBDIR)/libhsl$(lib-suffix)
	@echo =====hsl=========================$(MT)== Making   $(@F)
	@$(CXX) $(CXXFLAGS.g) $^ -o $@ $(F77LIBS) -lpthread $(LIBS)



clean:
	-rm -f $(LIBDIR)/contrib/hsl/*.$(OBJEXT) $(LIBDIR)/libhsl$(lib-suffix) \
		$(LIBDIR)/bin/detached_ma27$(EXEEXT) \
		Makefile.dep



# Rule to generate the dependency file. This file is
# automagically remade whenever needed, i.e. whenever
# one of the f-files changed. Make detects whether
# to remake this file upon inclusion at the bottom
# of this file.
#
# note that we take care if there are no f-files
#
# If the command fails, then remove Makefile.dep again and fail
Makefile.dep: $(f-files) Makefile
	@echo ============================ Remaking Makefile
	@(if test "x$(f-files)" != "x" ; then \
           for i in source/*.f ; do \
	     echo $$i | $(PERL) -pi -e 's#source/(.*)\.f#\$$(LIBDIR)/contrib/hsl/\1.$(OBJEXT):source/\1.f#;' ;\
	    done ; \
          fi) \
		> $@ \
	 || (rm -f $@ ; false)


# include all the dependencies
include Makefile.dep


.PHONY: clean
.PHONY: lib lib$(static-lib-suffix) lib.g$(static-lib-suffix)
