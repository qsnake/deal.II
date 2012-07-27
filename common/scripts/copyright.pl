######################################################################
# $Id: copyright.pl 10291 2005-03-29 16:27:05Z guido $
#
# Copyright
#
######################################################################
# Check the copyright line of a file against cvs log
#
# Call: perl copyright.pl <filename>
######################################################################

use strict;

my $file = $ARGV[0];

my @log = `cvs log $file`;
my %years;

foreach (@log)
{
    next unless (m/^date: (\d\d\d\d)/);
    my $year = $1;
    $years{$year} = 1;
}

my $copystring;

foreach (sort keys %years)
{
    if ($copystring)
    {
	$copystring .= ", $_";
    } else {
	$copystring = "$_";
    }
}

my $copyreg = "Copyright \\(C\\) $copystring by the deal.II authors";
$copystring = "Copyright (C) $copystring by the deal.II authors";

my $found = 0;
my $qpl = 0;
my $ok = 0;

while(<>)
{
    next unless (m/(Copyright.*authors)/);
    my $copyfile = $1;
    $found = 1;
    if (m/$copyreg/)
    {
	$ok = 1;
    } else {
	$copyfile =~ s/\(/\\(/;
	$copyfile =~ s/\)/\\)/;
	print "perl -pi~ -e 's{$copyfile}{$copystring};' $file\n";
    }
}

# print "# OK: $copystring\n" if ($ok);

print "# $file: no copyright found\n" unless ($found);

