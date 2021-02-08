#!/usr/bin/perl -w
# tle2edb.pl: perl script to convert NASA "2-line" geocentric orbital elements
# to XEphem .edb. we crack everything that looks reasonable. we allow the TLE
# to be embedded in other text, just so long as it stands on three successive
# lines of their own. the idea is to keep rolling successive lines into l1,
# l2 and l3 and crack whenever they all look correct.
#
# (c) 1993,1995,1998,2000 Elwood Charles Downey
#
# v2.2  9/22/00 add drag term
# v2.1  12/8/98 change to perl. actually use the checksum. support Y2K.
# v2:   6/26/95 allow names up to 22 chars long (official format change).
# v1.3 10/26/93 looks even harder.
# v1.2  9/12/93 fixes for NORAD-format (by bon@LTE.E-TECHNIK.uni-erlangen.de)
# v1.1   9/8/93 change type code to E
# v1.0  8/10/93 initial cut
#
# usage:
#	tle2edb.pl [file]
#
# Data for each satellite consists of three lines in the following format:
#
# AAAAAAAAAAAAAAAAAAAAAA
# 1 NNNNNU NNNNNAAA NNNNN.NNNNNNNN +.NNNNNNNN +NNNNN-N +NNNNN-N N NNNNN
# 2 NNNNN NNN.NNNN NNN.NNNN NNNNNNN NNN.NNNN NNN.NNNN NN.NNNNNNNNNNNNNN
#
# Line 0 is a 22-character name.
#
# Lines 1 and 2 are the standard Two-Line Orbital Element Set Format identical
# to that used by NORAD and NASA.  The format description is:
#
# Line 1
# Column     Description
#  01-01     Line Number of Element Data
#  03-07     Satellite Number
#  10-11     International Designator (Last two digits of launch year)
#  12-14     International Designator (Launch number of the year)
#  15-17     International Designator (Piece of launch)
#  19-20     Epoch Year (Last two digits of year). 2000+ if < 57.
#  21-32     Epoch (Julian Day and fractional portion of the day)
#  34-43     First Time Derivative of the Mean Motion
#         or Ballistic Coefficient (Depending on ephemeris type)
#  45-52     Second Time Derivative of Mean Motion (decimal point assumed;
#            blank if N/A)
#  54-61     BSTAR drag term if GP4 general perturbation theory was used.
#            Otherwise, radiation pressure coefficient.  (Decimal point assumed)
#  63-63     Ephemeris type
#  65-68     Element number
#  69-69     Check Sum (Modulo 10)
#            (Letters, blanks, periods, plus signs = 0; minus signs = 1)
#
# Line 2
# Column     Description
#  01-01     Line Number of Element Data
#  03-07     Satellite Number
#  09-16     Inclination [Degrees]
#  18-25     Right Ascension of the Ascending Node [Degrees]
#  27-33     Eccentricity (decimal point assumed)
#  35-42     Argument of Perigee [Degrees]
#  44-51     Mean Anomaly [Degrees]
#  53-63     Mean Motion [Revs per day]
#  64-68     Revolution number at epoch [Revs]
#  69-69     Check Sum (Modulo 10)
#
# All other columns are blank or fixed.
#
# Example:
#
# NOAA 6
# 1 11416U          86 50.28438588 0.00000140           67960-4 0  5293
# 2 11416  98.5105  69.3305 0012788  63.2828 296.9658 14.24899292346978
# 
# shield yield:
#
# NOAA 6-529,E,1/ 50.28438588/1986, 98.5105, 69.3305,0.0012788, 63.2828,296.9658,14.24899292,0.00000140,34697
#

$[ = 1;			# set array base to 1
$l1 = "";
$l2 = "";
$l3 = "";

while (<>) {
    chop;	# discard newline
    s/^[\s]*//;	# strip leading whitespace
    s/[\s]*$//;	# strip trailing whitespace
    $l1 = $l2;	# roll into place
    $l2 = $l3;
    $l3 = $_;

    # sanity checks
    next if (length($l1)>22 or $l2 !~ /^1 / or $l3 !~ /^2 /);
    next if (!&chksum ($l2));
    next if (!&chksum ($l3));

    # $l1 is just the satellite name
    $name = $l1;

    # pick out the goodies from l2, "line 1" of the TLE
    $year = substr($l2, 19, 2);
    if ($year >= 57) {$year += 1900;} else {$year += 2000;}
    $dayno = substr($l2, 21, 12);

    $decay = substr($l2, 34, 10);
    $drag = substr($l2,54,1) .
			substr($l2, 55, 5) * (10 ** substr($l2, 60, 2)) * 1e-5;
    $set = substr($l2, 65, 4);

    # pick out the goodies from l3, "line 2" of the TLE
    $inc = substr($l3, 9, 8);
    $ra = substr($l3, 18, 8);
    $e = substr($l3, 27, 7) * 1e-7;
    $ap = substr($l3, 35, 8);
    $anom = substr($l3, 44, 8);
    $n = substr($l3, 53, 11);
    $rev = substr($l3, 64, 5);

    # print in xephem format.
    printf "%s-%d,E,1/%s/%d,%s,%s,%s,%s,%s,%s,%s,%d,%s\n", $name, $set, $dayno,
		      $year, $inc, $ra, $e, $ap, $anom, $n, $decay, $rev, $drag;
}

sub chksum
{
    my $line = $_[0];
    my $len = length($line);
    my ($sum, $i, $c);

    $sum = 0;
    for ($i = 1; $i < $len; $i++) {
	$c = substr($line,$i,1);
	$sum += $c if ($c =~ /[\d]/);
	$sum += 1 if ($c eq "-");
    }
    $c = substr($line,$len,1);
    return (($sum % 10) == $c);
}

# For RCS Only -- Do Not Edit
# @(#) $RCSfile: tle2edb.pl,v $ $Date: 2000/09/26 23:33:09 $ $Revision: 1.3 $ $Name:  $
