# $Id: Makefile 18849 2009-05-15 17:59:46Z bangerth $
# Copyright (C) 2005, 2008 by Wolfgang Bangerth


D = ../..

include $D/common/Make.global_options

# rules
$(LIBDIR)/contrib/functionparser/fparser.$(OBJEXT): fparser.cc fparser.h
	@echo =====function parser==optimized==$(MT)== $(<F)
	@$(CXX) $(CXXFLAGS.o) -c fparser.cc -o $@



clean:
	-rm -f *~ 


.PHONY: clean
