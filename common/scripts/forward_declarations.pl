# call this script with a list of files it shall process for
# class and struct declarations.


#fill list of files to be processed
while ($ARGV[0]) {
    @input_files = (@input_files, shift);
}


# generate a guard for the file to disallow multiple inclusion
$guard = `pwd`;
# in case the string gets longer than the number of significant
# positions for the preprocessor is, it is safer to use the
# last parts of the string fist. therefore, split the string
# at the slashes and assemble it again backwards
$guard = join("/", reverse(split(/\//, $guard)));
$guard =~ s![^\w]!_!g;
$guard .= "__guard";
print "#ifndef $guard\n";
print "#define $guard\n";
print "\n";

print "/* this file is automatically generated from the Makefile; do not\n";
print " * change it by hand.                                               */\n";
print "\n\n";

foreach $file (@input_files) {
    parse_class_declarations ($file);
};

print "\n\n";
print "#endif // $guard\n";




sub parse_class_declarations {
    local ($filename) = $_[0];
    
    open (FILE, $filename);
    while (<FILE>) {
	# if this is continued line, then strip the backslash and join
	# it with the following one as well
	while ( /\\$/ ) {
	    $_ =~ s/\\$//;
	    $_ = $_ . " " . <FILE>;
        }

	# if the lines contains a "template" at the 
	# beginning and no semicolon at the end: join it
	# with the next line.
	if ( /^\s*template/ && !/;\s*$/ ) {
	    # if this is a multiline template, then join following lines as well
	    # find out by looking at the last char of the line
	    while ( ! />\s*$/ ) {
		# more concise check: find out whether the number of '<' is
		# not equal to the number of '>'
		my ( $tmp1, $tmp2 ) = ($_, $_);
		$tmp1 =~ s/[^<]//g;
		$tmp2 =~ s/[^>]//g;
		if ( length ($tmp1) ==  length ($tmp2) ) {
		    last;
		}
		else
		{
		    s/\n//;
		    $_ = $_ . " " . <FILE>;
		    s/ \s+/ /g;
		}
	    }
	    s/\n//;
	    s/ \s+/ /g;
	    $_ = $_ . " " . <FILE>;
	}

	if ( /^\s*((template\s*<
                     (
                        ([-\w,_=:\s]   |
                         <([-\w_,=:\s])+>  )*
                     )>\s*)?
                   (class|struct))(.*)/x ) {
	    # this is the declaration of a class, possibly a template
	    $basepart = $1;
	    $rest     = $7;

	    # test whether it is a forward declaration or something else.
	    # if it is a forward declaration, then skip it, as it must 
	    # be declared somewhere else properly
	    # (note that $rest contains the name of the class and what 
	    # comes after that)
	    if ( $rest =~ /;\s*$/ ) {
		next;
	    }

	    # first extract the name of the class
	    $rest =~ /([\w_]+(\s*<(([-\w,_\s]|<([-\w,\s])+>)+)>)?)(.*)/;

	    $name = $1;
	    $rest = $6;

	    # we look for declarations, where after the name comes a colon
	    # or an open-brace, maybe also a semicolon or just nothing
	    #
	    # the colon might be of an inheritance list, but we do not
	    # want it to come from a nested class declaration
	    if (($rest =~ /^\s*([\{;]|:[^:])/) || ($rest =~ /^\s*$/)) {
		$declaration = $basepart . " " . $name;
		# strip double spaces etc.
		$declaration =~ s/\s\s+/ /g;
		$declaration =~ s/^\s+//g;
		$declaration =~ s/\s+$//g;

		# strip default template parameters
		while ( $declaration =~ s/<(.*)=\s*[-\w,_:\s]+(<[^.]*>)?(.*)>/<$1 $3 > / ) {
		}

		# impose a negativ-list of names we do not want to
		# have in this file. this is due to a compiler bug in
		# egcs 1.1.2, which does not gracefully handle local
		# template classes if their name is globally defined,
		# for example
		#
		# template <int dim> struct Y;
		# struct X {template <int dim> struct Y{}; };
		# X::Y<1> y;
		#
		# does not work. So we filter out these classes at 
		# present. A better way would certainly be to only
		# allow those classes into the forward-declarations.h
		# file that are global, but that would mean parsing
		# the include files instead of just searching for
		# small bits...
		if ( ! ($declaration =~ /EpsFlags|Patch|Mem_Fun_Data|Fun_Data/ ))
		{
		    print $declaration, ";\n";
		}
	    }
	}
    }
}