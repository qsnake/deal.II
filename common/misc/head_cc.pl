$/='this should not appear in any of these stupid files';

$fn = $ARGV;

$head = << 'EOT'
//----------------------------  XXX  ---------------------------
//    $Id: head_cc.pl 5351 2002-01-07 09:48:43Z wolf $
//    Version: $Name$
//
//    Copyright (C) 2002 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  XXX  ---------------------------


EOT
    ;

$head =~ s/XXX/$fn/g;

s/^[^\#]+/$head\n\n\n/s;
s/\/\*-+\s+$fn\s+-+\*\/\n//g;
s/\/\*\s*end of.*H\s*\*\///g;
s/\n[ \t]*\n[ \t]*\n\s*\n*/\n\n\n/g;
