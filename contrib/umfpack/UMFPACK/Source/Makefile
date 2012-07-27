#-------------------------------------------------------------------------------
# UMFPACK Makefile for compiling on Unix systems (for GNU Make)
#-------------------------------------------------------------------------------

include ../../../../common/Make.global_options

#-------------------------------------------------------------------------------
# source files
#-------------------------------------------------------------------------------

# non-user-callable umf_*.[ch] files:
UMFCH = umf_assemble umf_blas3_update umf_build_tuples umf_create_element \
	umf_dump umf_extend_front umf_garbage_collection umf_get_memory \
	umf_init_front umf_kernel umf_kernel_init umf_kernel_wrapup \
	umf_local_search umf_lsolve umf_ltsolve umf_mem_alloc_element \
	umf_mem_alloc_head_block umf_mem_alloc_tail_block \
	umf_mem_free_tail_block umf_mem_init_memoryspace \
	umf_report_vector umf_row_search umf_scale_column \
	umf_set_stats umf_solve umf_symbolic_usage umf_transpose \
	umf_tuple_lengths umf_usolve umf_utsolve umf_valid_numeric \
	umf_valid_symbolic umf_grow_front umf_start_front umf_2by2 \
	umf_store_lu umf_scale

# non-user-callable umf_*.[ch] files, int/long versions only (no real/complex):
UMFINT = umf_analyze umf_apply_order umf_colamd umf_free umf_fsize \
	umf_is_permutation umf_malloc umf_realloc umf_report_perm \
	umf_singletons

# non-user-callable, created from umf_ltsolve.c, umf_utsolve.c,
# umf_triplet.c, and umf_assemble.c , with int/long and real/complex versions:
UMF_CREATED = umf_lhsolve umf_uhsolve umf_triplet_map_nox \
	umf_triplet_nomap_x umf_triplet_nomap_nox umf_triplet_map_x \
	umf_assemble_fixq umf_store_lu_drop

# non-user-callable, int/long and real/complex versions:
UMF = $(UMF_CREATED) $(UMFCH)

# user-callable umfpack_*.[ch] files (int/long and real/complex):
UMFPACK = umfpack_col_to_triplet umfpack_defaults umfpack_free_numeric \
	umfpack_free_symbolic umfpack_get_numeric umfpack_get_lunz \
	umfpack_get_symbolic umfpack_numeric umfpack_qsymbolic \
	umfpack_report_control umfpack_report_info umfpack_report_matrix \
	umfpack_report_numeric umfpack_report_perm umfpack_report_status \
	umfpack_report_symbolic umfpack_report_triplet \
	umfpack_report_vector umfpack_solve umfpack_symbolic \
	umfpack_transpose umfpack_triplet_to_col umfpack_scale \
	umfpack_load_numeric umfpack_save_numeric \
	umfpack_load_symbolic umfpack_save_symbolic

# user-callable, created from umfpack_solve.c (umfpack_wsolve.h exists, though):
# with int/long and real/complex versions:
UMFPACKW = umfpack_wsolve

USER = $(UMFPACKW) $(UMFPACK)

# user-callable, only one version for int/long, real/complex, *.[ch] files:
GENERIC = umfpack_timer umfpack_tictoc

#-------------------------------------------------------------------------------
# include files:
#-------------------------------------------------------------------------------

INC = ../Include/umfpack.h \
	umf_config.h umf_version.h umf_internal.h umf_triplet.h \
	$(addsuffix .h,$(UMFCH)) \
	$(addsuffix .h,$(UMFINT)) \
	$(addprefix ../Include/, $(addsuffix .h,$(USER))) \
	$(addprefix ../Include/, $(addsuffix .h,$(GENERIC))) \
	../../AMD/Include/amd_internal.h ../../AMD/Include/amd.h

#-------------------------------------------------------------------------------
# object files for each version
#-------------------------------------------------------------------------------

DI = $(addprefix $D/lib/contrib/umfpack/, $(addsuffix .$(OBJEXT), $(subst umf_,umf_di_,$(UMF)) $(subst umfpack_,umfpack_di_,$(USER))))
DL = $(addprefix $D/lib/contrib/umfpack/, $(addsuffix .$(OBJEXT), $(subst umf_,umf_dl_,$(UMF)) $(subst umfpack_,umfpack_dl_,$(USER))))
I  = $(addprefix $D/lib/contrib/umfpack/, $(addsuffix .$(OBJEXT), $(subst umf_,umf_i_,$(UMFINT))))
L  = $(addprefix $D/lib/contrib/umfpack/, $(addsuffix .$(OBJEXT), $(subst umf_,umf_l_,$(UMFINT))))
GN = $(addprefix $D/lib/contrib/umfpack/, $(addsuffix .$(OBJEXT), $(subst umfpack_,umfpack_gn_,$(GENERIC))))

#-------------------------------------------------------------------------------
# compile each int and long routine (with no real/complex version)
#-------------------------------------------------------------------------------

$D/lib/contrib/umfpack/umf_i_%.$(OBJEXT): umf_%.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDINT -c $< -o $@

$D/lib/contrib/umfpack/umf_l_%.$(OBJEXT): umf_%.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDLONG -c $< -o $@

#-------------------------------------------------------------------------------
# compile each routine in the DI version
#-------------------------------------------------------------------------------

$D/lib/contrib/umfpack/umf_di_%.$(OBJEXT): umf_%.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDINT -c $< -o $@

$D/lib/contrib/umfpack/umf_di_%hsolve.$(OBJEXT): umf_%tsolve.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDINT -DCONJUGATE_SOLVE -c $< -o $@

$D/lib/contrib/umfpack/umf_di_triplet_map_x.$(OBJEXT): umf_triplet.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDINT -DDO_MAP -DDO_VALUES -c $< -o $@

$D/lib/contrib/umfpack/umf_di_triplet_map_nox.$(OBJEXT): umf_triplet.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDINT -DDO_MAP -c $< -o $@

$D/lib/contrib/umfpack/umf_di_triplet_nomap_x.$(OBJEXT): umf_triplet.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDINT -DDO_VALUES -c $< -o $@

$D/lib/contrib/umfpack/umf_di_triplet_nomap_nox.$(OBJEXT): umf_triplet.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDINT -c $< -o $@

$D/lib/contrib/umfpack/umf_di_assemble_fixq.$(OBJEXT): umf_assemble.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDINT -DFIXQ -c $< -o $@

$D/lib/contrib/umfpack/umf_di_store_lu_drop.$(OBJEXT): umf_store_lu.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDINT -DDROP -c $< -o $@

$D/lib/contrib/umfpack/umfpack_di_wsolve.$(OBJEXT): umfpack_solve.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDINT -DWSOLVE -c $< -o $@

$D/lib/contrib/umfpack/umfpack_di_%.$(OBJEXT): umfpack_%.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDINT -c $< -o $@

#-------------------------------------------------------------------------------
# compile each routine in the DL version
#-------------------------------------------------------------------------------

$D/lib/contrib/umfpack/umf_dl_%.$(OBJEXT): umf_%.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDLONG -c $< -o $@

$D/lib/contrib/umfpack/umf_dl_%hsolve.$(OBJEXT): umf_%tsolve.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDLONG -DCONJUGATE_SOLVE -c $< -o $@

$D/lib/contrib/umfpack/umf_dl_triplet_map_x.$(OBJEXT): umf_triplet.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDLONG -DDO_MAP -DDO_VALUES -c $< -o $@

$D/lib/contrib/umfpack/umf_dl_triplet_map_nox.$(OBJEXT): umf_triplet.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDLONG -DDO_MAP -c $< -o $@

$D/lib/contrib/umfpack/umf_dl_triplet_nomap_x.$(OBJEXT): umf_triplet.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDLONG -DDO_VALUES -c $< -o $@

$D/lib/contrib/umfpack/umf_dl_triplet_nomap_nox.$(OBJEXT): umf_triplet.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDLONG -c $< -o $@

$D/lib/contrib/umfpack/umf_dl_assemble_fixq.$(OBJEXT): umf_assemble.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDLONG -DFIXQ -c $< -o $@

$D/lib/contrib/umfpack/umf_dl_store_lu_drop.$(OBJEXT): umf_store_lu.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDLONG -DDROP -c $< -o $@

$D/lib/contrib/umfpack/umfpack_dl_wsolve.$(OBJEXT): umfpack_solve.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDLONG -DWSOLVE -c $< -o $@

$D/lib/contrib/umfpack/umfpack_dl_%.$(OBJEXT): umfpack_%.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -DDLONG -c $< -o $@

#-------------------------------------------------------------------------------
# Create the generic routines (GN) using a generic rule
#-------------------------------------------------------------------------------

$D/lib/contrib/umfpack/umfpack_gn_%.$(OBJEXT): umfpack_%.c $(INC)
	@echo =====umfpack==========optimized====== $(<F)
	@$(CC) $(CFLAGS.o) -I../Include -I../../AMD/Include -I../../AMD/Source -c $< -o $@

#-------------------------------------------------------------------------------
# Create the libumfpack.a library
#-------------------------------------------------------------------------------

all: $(I) $(L) $(GN) $(DI) $(DL)
