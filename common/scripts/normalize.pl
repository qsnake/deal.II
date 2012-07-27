######################################################################
# $Id: normalize.pl 22430 2010-10-23 13:41:09Z heister $
#
# Copyright (C) 2001, 2003, 2005, 2010, the deal.II authors
#
# Remove insignificant volatile data from output files of tests
#
# Data affected:
#  JobID line (containing date)
#  line number of exceptions
#  start and final residual in iterations
#  small doubles
######################################################################

# Remove absolute path names
$D = $0;
$D =~ s!common/scripts/normalize.pl!!;
s!$D!DEAL_II_PATH/!g;

# Remove JobID

s/JobId.*//;

# Several date and time strings

s/%%Creation Date:.*//;
s/\"created\".*//;
s/# Time =.*//;
s/# Date =.*//; 
s/^\s+Time =.*//;
s/^\s+Date =.*//;
s/Time tag:.*//g;
s/by the deal.II library on.*//;

# Exceptions

s/line <\d+> of file <.*\//file </;

# Make small exponentials zero

#s/-?\d?\.\d+e-[123456789]\d+/0.00/g;

# Zeroes are zero

s/-0\.00/0.00/g;

# Residual values

#s/value.*//;
#s/with residual.*//;


# remove deal.II debug output
s/^DEAL.*::_.*\n//g;
