#!/usr/bin/perl
# convert elements appearing in http://encke.jpl.nasa.gov/eph to xephem format.
# we need to scan the entire ephemeris so we can dig out the various values
#   which are sprinkled near the beginning and end of the file.
# (C) 1996 Elwood Charles Downey
# Feb 6, 1996. v1.0
# Feb 7, 1996. v1.1 incorporated nice perl tweaks from dbrooks@x.org.
# Feb 17,1996  v1.2 work harder to pull out a name.

# read the file and pick out the good stuff.
while (<>) {
    # Capture the name from a title line -- example:
    # TWO BODY EPHEMERIS (J2000.0 EQUINOX) FOR C/1995 Y1 (HYAKUTAKE)
    if (/TWO BODY EPHEMERIS \([BJ0-9.]+ EQUINOX\) FOR(.*)/) {
	$_ = $1;
	chop();
	s/^ *//;
	s/ *$//;
	s/ ?Comet ?//;
	s/[()]//g;
	s/[CP]\///;
	$name = $_;
	next;
    }

    # Capture the epoch from the title line before the elements -- example:
    # THE ABOVE COMET EPHEMERIS IS BASED UPON THE FOLLOWING ECLIPTIC ORBITAL ELEMENTS(J2000.0
    if (/THE ABOVE COMET EPHEMERIS IS BASED UPON THE FOLLOWING ECLIPTIC ORBITAL ELEMENTS\([BJ]([\d.]+)\)/) {
	$epoch = $1;
	next;
    }

    # Capture the orbital elements on line following their headings.
    if (/YR  MN DY  HR       J.D.          Q\(AU\)       E      SOMEGA\(DEG\)  LOMEGA\(DEG\)   I\(DEG\)/) {
	warn "Warning: multiple data sets found -- using last.\n"
								if defined($YR);
	$_ = <>;
	($YR,$MN,$DY,$HR,$JD,$Q,$E,$SOMEGA,$LOMEGA,$I) = split(' ');
	next;
    }

    # Capture magnitude model from line clear at the bottom -- example:
    # TMAG = TOTAL MAGNITUDE   =  7.5 +  5.00*LOG10(DELTA) + 10.00*LOG10(R).
    if (/TMAG = TOTAL MAGNITUDE[\s=]+([\d.]+)[\s+]+5\.00\*LOG10\(DELTA\)[\s+]+([\d.]+)\*LOG10\(R\)/) {
	$g = $1;
	$k = $2/2.5;
	next;
    }
}

# did we find everything necessary?
length($name) > 0 || die "No name found.\n";
$epoch > 1900 || die "No epoch found.\n";
defined($YR) || die "No orbital elements found.\n";
$g > 0 || $k > 0 || warn "No magnitude model found.\n"; # not especially fatal.

# format output depending on orbital shape.
if ($E < 1) {
    # elliptical
    $i = $I;		# inclination
    $O = $LOMEGA;	# long of asc node
    $o = $SOMEGA;	# arg of peri
    $a = $Q/(1-$E);	# mean distance
    $n = 0;		# mean daily motion (derived)
    $e = $E;		# eccentricity
    $M = 0;		# mean anomaly
    $D = $epoch;	# date of i/O/o

    printf "%s,e,%.8g,%.8g,%.8g,%.8g,%.8g,%.8g,%.8g,%g/%.8g/%g,%g,g%g,%g\n",
	$name, $i, $O, $o, $a, $n, $e, $M, $MN, $DY+$HR/24, $YR, $D, $g, $k;
} elsif ($E > 1) {
    # hyperbolic
    $i = $I;		# inclination
    $O = $LOMEGA;       # long of asc node
    $o = $SOMEGA;       # arg of peri
    $e = $E;            # eccentricity
    $q = $Q;		# peri distance
    $D = $epoch;	# date of i/O/o

    printf "%s,h,%g/%.8g/%g,%.8g,%.8g,%.8g,%.8g,%.8g,%.8g,%g,%g\n",
	$name, $MN, $DY+$HR/24, $YR, $i, $O, $o, $e, $q, $D, $g, $k;
} else {
    # parabolic
    $i = $I;		# inclination
    $o = $SOMEGA;       # arg of peri
    $q = $Q;		# peri distance
    $O = $LOMEGA;       # long of asc node
    $D = $epoch;	# date of i/O/o

    printf "%s,p,%g/%.8g/%g,%.8g,%.8g,%.8g,%.8g,%.8g,%g,%g\n",
	$name, $MN, $DY+$HR/24, $YR, $i, $o, $q, $O, $D, $g, $k;
}

# For RCS Only -- Do Not Edit
# @(#) $RCSfile: encke2edb.pl,v $ $Date: 1996/10/11 14:47:17 $ $Revision: 1.1 $ $Name:  $
