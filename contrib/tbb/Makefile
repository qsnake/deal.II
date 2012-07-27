# bild the tbb libraries. they end up in directories with weird names that contain
# the system and compiler names etc. rename them to something that's a bit more
# predictable for the downstream make files.
tbb:
	@cd tbb30_104oss ; $(MAKE) tbb tbb_build_dir=$D/lib/contrib/tbb
	@rm -f $D/lib/libtbb*
	@for i in $D/lib/contrib/tbb/*_debug/libtbb_debug* ; do \
	    ln -s -f $$i $D/lib/`echo $$i | $(PERL) -pi -e 's#.*/##g;'` ; done
	@for i in $D/lib/contrib/tbb/*_release/libtbb* ; do \
	    ln -s -f $$i $D/lib/`echo $$i | $(PERL) -pi -e 's#.*/##g;'` ; done


# nothing to do here: tbb builds everything in $D/lib/contrib/tbb
clean:

.PHONY: tbb clean

include ../../common/Make.global_options