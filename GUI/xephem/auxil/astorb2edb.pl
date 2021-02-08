#!/usr/bin/perl
# convert  astorb.txt to 2 .edb files.
# Usage: [-f] <base>
#   if -f then use ftp to get the script from lowell, else read it from stdin.
#   <base> is a prefix used in naming the generated .edb files.
# Two files are created:
#   <base>.edb contains only those asteroids which might ever be brighter
#      than $dimmag (set below);
#   <base>_dim.edb contains the remaining asteroids.
#
# astorb is a set of elements for 30k+ asteroids maintained by Lowell
# Observatory. See ftp://ftp.lowell.edu/pub/elgb/astorb.html. From the
# Acknowledgments section of same:
#   The research and computing needed to generate astorb.dat were funded
#   principally by NASA grant NAGW-1470, and in part by the Lowell Observatory
#   endowment. astorb.dat may be freely used, copied, and transmitted provided
#   attribution to Dr. Edward Bowell and the aforementioned funding sources is
#   made. 
#
# grab RCS version
my $ver = '$Revision: 1.1 $';
$ver =~ s/\$//g;
my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst, $me);
my $XCN=<<EOF;
# ================================================================================
#                 The Lowell Asteroid Orbital Elements Database
# ================================================================================
# Introduction
#       astorb.dat is an ASCII file of high-precision osculating orbital elements,
# ephemeris uncertainties, and some additional data for all the numbered asteroids
# and the vast majority of  unnumbered  asteroids (multi-apparition and single-ap-
# parition) for which it is possible to make reasonably determinate computations.
#       It is currently about 1.7 Mb in size in its compressed form(astorb.dat.gz)
# 6.3 Mb in  size when  decompressed  (astorb.dat),  and  contains  237215  orbits
# computed by me (Edward Bowell).  Each  orbit,  based on astrometric observations
# downloaded from the Minor Planet Center, occupies one 266-column record.
#
#                          Special Features of astorb.dat
#       There are three primary differences between our  database and conventional
# asteroid orbit files.
#       First, we update the database daily. Thus,  observations in each new batch
# of Minor Planet Circulars will be used in new  orbits on a  monthly  basis,  and
# those in the Minor Planet Electronic Circulars shortly after they are published.
# Other changes, such as the addition of orbits resulting from our own astrometric
# observations  and the computation of current ephemeris uncertainties, are  being
# made  on  a  quasi-daily basis. Automatic daily updates commence at 8 hr UT. The
# updating  process  is  generally  completed  by 10:00 UT. On the UT date of full
# Moon, the updating process is not completed until 14 hr UT or later.
#
#       Second,  all  the  orbits  in a given version of the file have an epoch of
# osculation near the  present.  Consequently,  the ephemerides of most non-Earth-
# -approaching asteroids can be computed to  arcsec accuracy or better within ± 50
# days of the epoch using a 2-body ephemeris program.
#
#       Third, current and future ephemeris uncertainties are given.Observers will
# readily be able to  estimate  whether  asteroids are likely  to  be within their
# telescopes' fields  of  view,  and  they  will  better  be  able  to  prioritize
# astrometric targets.
#
# ================================================================================
# XEphem Catalog Notes. (XCN)
# ---------------------------
#
# AstLowell.edb
# astorb is a set of elements for 30k+ asteroids maintained by Lowell
# Observatory. See ftp://ftp.lowell.edu/pub/elgb/astorb.html. From the
# Acknowledgments section of same:
#   The research and computing needed to generate astorb.dat were funded
#   principally by NASA grant NAGW-1470, and in part by the Lowell Observatory
#   endowment. astorb.dat may be freely used, copied, and transmitted provided
#   attribution to Dr. Edward Bowell and the aforementioned funding sources is
#   made.
# ================================================================================
EOF
# setup cutoff mag
$dimmag = 13;			# dimmest mag to be saved in "bright" file

# set site and file in case of -f
my $ORBSITE = "ftp.lowell.edu";
my $ORBFTPDIR = "/pub/elgb";
my $ORBFILE = "astorb.dat";
my $ORBGZFILE = "astorb.dat.gz";
my $ORBOUTFILE = "AstLowell";

# immediate output
$| = 1;

# crack args.
# when thru here $fnbase is prefix name and $srcfd is handle to $ORBFILE.
if (@ARGV == 2) {
    &usage() unless $ARGV[0] eq "-f"; # Changed @ for $
    &fetch();
    open SRCFD, $ORBFILE or die "$ORBFILE: $?\n";
    $ARGV[1] = $ORBOUTFILE if $ARGV[1] =~ /$ORBGZFILE/; # Correct the output name if gzipped.
    $srcfd = SRCFD;
    $fnbase = $ARGV[1];
} elsif (@ARGV != 1) {
    &usage();
} else {
    &usage() if $ARGV[0] =~ /^-/; # Changed @ for $
    &fetch() if $ARGV[0] =~ /$ORBGZFILE/; # Check is local file is gzipped.
    open SRCFD, $ORBFILE or die "$ORBFILE: $?\n"; # Otherwise go to process.
    $ARGV[0] = $ORBOUTFILE if $ARGV[0] =~ /$ORBGZFILE/; # Correct the output name if gzipped.
    $srcfd = SRCFD;
    $fnbase = $ARGV[0];

}

# create output files prefixed with $fnbase
my $brtfn = "$fnbase.edb";		# name of file for bright asteroids
open BRT, ">$brtfn" or die "Can not create $brtfn\n";
my $dimfn = "$fnbase"."_dim.edb";	# name of file for dim asteroids
open DIM, ">$dimfn" or die "Can not create $dimfn\n";
print "Creating $brtfn and $dimfn..\n";

# build some common boilerplate
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime;
$year += 1900;
$mon += 1;
$from = "# Data From ftp://ftp.lowell.edu/pub/elgb/astorb.dat.gz\n";
$what = "# Generated by astorb2edb.pl $ver, (c) 2000 Elwood Downey\n";
$when = "# Processed $year-$mon-$mday $hour:$min:$sec UTC\n";

# add boilerplate to each file
print BRT "# Asteroids ever brighter than $dimmag.\n";
print BRT $from;
print BRT $what;
print BRT $when;
print BRT $XCN;
print DIM "# Asteroids never brighter than $dimmag.\n";
print DIM $from;
print DIM $what;
print DIM $when;
print DIM $XCN;

# process each astorb.dat entry. see astorb.txt from same site for format.
while (<$srcfd>) {
    # build the name
my  $nmbr = &s(1,6);
    $nmbr =~ s/^s+//g;
    $nmbr =~ s/\s+$//g;
    $nmbr =~ s/\s+//g;
    $nmbr = $nmbr+0;

    $name = &s(8,25);
    $name =~ s/^ *//;
    $name =~ s/ *$//;

    # gather the orbital params
    $i = &s(149,157) + 0;
    $O = &s(138,147) + 0;
    $o = &s(127,136) + 0;
    $a = &s(170,181) + 0;
    $e = &s(159,168) + 0;
    $M = &s(116,125) + 0;
    $H = &s(43,47) + 0;
    $G = &s(49,53) + 0;
    $TM = &s(111,112) + 0;
    $TD = &s(113,114) + 0;
    $TY = &s(107,110) + 0;

    # skip obviously bogus entries
    if ($a == 0) {
	next;
    }

    # decide whether it's ever bright
    $per = $a*(1 - $e);
    $aph = $a*(1 + $e);
    if ($per < 1.1 && $aph > .9) {
	$fd = BRT;	# might be in the back yard some day :-)
    } else {
	$maxmag = $H + 5*&log10($per*&absv($per-1));
	$fd = $maxmag > $dimmag ? DIM : BRT;
    }

    # print
    print $fd "$nmbr " if ($nmbr != 0);
    print $fd "$name";
    print $fd ",e,$i,$O,$o,$a,0,$e,$M,$TM/$TD/$TY,2000.0,$H,$G\n";
}

# remove fetched files, if new
if (defined($fetchok)) {
    unlink $ORBFILE;
    unlink $ORBGZFILE;
}

print "Done\n";
exit 0;

# like substr($_,first,last), but one-based.
sub s {
my $str=();
$str = $_;
# just to reduce the noise from -w.
    substr ($str, $_[0]-1, $_[1] - $_[0]+1);
}

# return log base 10

sub log10 {
    .43429*log($_[0]);
}

# return absolute value
sub absv {
    $_[0] < 0 ? -$_[0] : $_[0];
}

# print usage message then die
sub usage
{
    my $base = $0;
    $base =~ s#.*/##;
    print "Usage: $base [-f] <base>\n";
    print "$ver\n";
    print "Purpose: convert $ORBFILE to 2 .edb files.\n";
    print "Options:\n";
    print "  -f: first ftp $ORBFILE from $ORBSITE, else read from stdin\n";
    print "Creates two files:\n";
    print "  <base>.edb:     all asteroids ever brighter than $dimmag\n";
    print "  <base>_dim.edb: all asteroids never brighter than $dimmag\n";
    print "Note: If you downloaded the file astorb.dat or astorb.dat.gz,\n";
    print "      run this script with the file name as the only command.\n";
    print "      line option.\n";

    exit 1;
}

# get and explode the data
sub fetch
{
    # transfer
    print "Getting $ORBFTPDIR/$ORBGZFILE from $ORBSITE...\n";
    $cmd_v1 = "curl -s --connect-timeout 10 -u 'anonymous:xephem\@clearskyinstitute.com' ftp://$ORBSITE/$ORBFTPDIR/$ORBGZFILE > $ORBGZFILE";
    $cmd_v2 = "curl -s --disable-epsv --connect-timeout 10 -u 'anonymous:xephem\@clearskyinstitute.com' ftp://$ORBSITE/$ORBFTPDIR/$ORBGZFILE > $ORBGZFILE";
    print "trying $cmd_v1\n";
    $curl_return = system "$cmd_v1";
    $curl_return = $curl_return >> 8;
    if ($curl_return != 0) {
        print "trying $cmd_v2\n";
        $curl_return = system "$cmd_v2";
        $curl_return = $curl_return >> 8;
    }
    if ($curl_return != 0) {
        die "curl failed\n";
    }

    # explode
    print "Decompressing $ORBGZFILE ...\n";
    $gunzip = "gunzip";
    !system "$gunzip < $ORBGZFILE > $ORBFILE" or die "$gunzip failed\n";
    -s $ORBFILE or die "$ORBFILE: failed to create from $gunzip $ORBGZFILE\n";

    # flag
    $fetchok = 1;
}

# For RCS Only -- Do Not Edit
# @(#) $RCSfile: astorb2edb.pl,v $ $Date: 2014/07/07 01:45:10 $ $Revision: 1.1 $ $Name:  $
