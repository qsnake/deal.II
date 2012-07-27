include ../../common/Make.global_options

deplibs = 

# for cygwin, lapack and other libraries are necessary to link with
ifeq ($(findstring cygwin,$(TARGET)),cygwin)
  deplibs = $(LIBS)
endif


# rules
lib: AMD UMFPACK


AMD:
	cd AMD/Source ; $(MAKE) all

UMFPACK:
	cd UMFPACK/Source ; $(MAKE) all

clean:


.PHONY: AMD UMFPACK clean lib
