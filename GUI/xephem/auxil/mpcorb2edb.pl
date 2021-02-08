#!/usr/bin/perl
# convert MPCORB.DAT to 2 .edb files.
# Usage: [-f] <base>
#   if -f then use ftp to get the script from harvard, else read it from stdin.
#   <base> is a prefix used in naming the generated .edb files.
# Two files are created:
#   <base>.edb contains only those asteroids which might ever be brighter
#      than $dimmag (set below);
#   <base>_dim.edb contains the remaining asteroids.
#
# mpcorb.dat is a service of the Minor Planet Center,
# http://cfa-www.harvard.edu/cfa/ps/mpc.html.
#
# Copyright (c) 2000 Elwood Downey
# 16 Mar 1999: first draft
# 17 Mar 1999: change output filename
#  4 Apr 2000: update arg handling and support new MPC file format.
#  6 Oct 2000: add -f
# 30 Jan 2003: add XCN.
# 24 Sep 2004: only remove files if downloaded fresh
#  1 Nov 2012: change from ftp to curl for better error handling.
#  4 Jul 2014: change download site

# grab RCS version
my $ver = '$Revision: 1.3 $';
$ver =~ s/\$//g;
my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst, $me);
my $XCN=<<EOF;
# ================================================================================
#                       The Minor Planet Center Orbit Database
# ================================================================================
# XEphem Catalog Notes. (XCN)
# ---------------------------
# MPC Orbit Database
# * The MPC Orbit Database (MPCORB) is available for downloading from the anon-ftp
#   link ftp://cfa-ftp.harvard.edu/pub/MPCORB/.
#   Download either the complete file or daily updates.
# * Note that the security features used in  this  ftp  server require  that  your
#   machine be found  in  a  "reverse DNS lookup"  in order to access this site. A
#   number  of  ISPs  do not properly DNS-register their IP addresses: attempts at
#   access from such ISPs will  fail.  You  should  complain  to  your ISP if your
#   machine cannot access this ftp server.
# * You must  also  ensure  that  you have set your anonymous ftp password in your
#   browser. This password must  contain the "@" character. The default values for
#   this setting in  browsers  such  as  Netscape  and Mozilla will NOT work. (New
#   requirement 2003 Aug. 27)
#
#    License and Acknowledgements.
#    ----------------------------
#    This file contains published orbital elements for  all  numbered  and
#    unnumbered multi-opposition minor planets for which it is possible to
#    make reasonable predictions.  It also includes published elements for
#    recent one-opposition  minor planets  and is  intended to be complete
#    through the last issued  Daily Orbit  Update  MPEC.  As  such  it  is
#    intended to be of interest primarily to astrometric observers.
#
#    Software programs may include this datafile amongst their datasets,as
#    long as this header is included (it is acceptable if it is  contained
#    in a file separate from the actual data) and that proper  attribution
#    to the Minor Planet Center is given.  Credit to the  individual orbit
#    computers is implicit by the inclusion of a reference and the name of
#    the orbit computer on each orbit record. Information on how to obtain
#    updated copies of the datafile must also be included.
#
#    The work of the individual astrometric observers,without whom none of
#    the work of the Minor Planet Center would be possible, is  gratefully
#    acknowledged.
#
#    New versions of this file, updated on a daily basis,will be available
#    at: ftp://cfa-ftp.harvard.edu/pub/MPCORB/MPCORB.DAT
#    These  files  are  rebuilt  each  night,  generally between 02:00 and
#    03:00 EST  (07:00  and 08:00 UT), except during the short period each
#    month  when  the  next  batch  of   Minor  Planet Circulars are being
#    prepared.
# ================================================================================
EOF

# setup cutoff mag
my $dimmag = 13;			# dimmest mag to be saved in "bright" file
# set site and file in case of -f
my $MPCSITE = "http://www.minorplanetcenter.net";
my $MPCFTPDIR = "/iau/MPCORB";
my $MPCFILE = "MPCORB.DAT";
my $MPCZIPFILE = "MPCORB.DAT.gz";
my $MPCOUTFILE = "AstMPC";
# immediate output
$| = 1;

# crack args.
# when thru here $fnbase is prefix name and $srcfd is handle to $MPCFILE.
if (@ARGV == 2) {
    &usage() unless $ARGV[0] eq "-f"; # Changed @ for $
    &fetch();
    open SRCFD, $MPCFILE or die "$MPCFILE: $?\n";
    $ARGV[1] = $MPCOUTFILE if $ARGV[1] =~ /$MPCZIPFILE/; # Correct the output name if gzipped.
    $srcfd = SRCFD;
    $fnbase = $ARGV[1];
} elsif (@ARGV != 1) {
    &usage();
} else {
    &usage() if $ARGV[0] =~ /^-/; # Changed @ for $
    &fetch() if $ARGV[0] =~ /$MPCZIPFILE/; # Check is local file is gzipped.
    open SRCFD, $MPCFILE or die "$MPCFILE: $?\n"; # Otherwise go to process.
    $ARGV[0] = $MPCOUTFILE if $ARGV[0] =~ /$MPCZIPFILE/; # Correct the output name if gzipped.
    $srcfd = SRCFD;
    $ARGV[0] = $MPCOUTFILE if $ARGV[0] =~ /$MPCFILE/; # Correct the output name if gzipped.
    $fnbase = $ARGV[0];
}

# create output files prefixed with $fnbase
$brtfn = "$fnbase.edb";		# name of file for bright asteroids
open BRT, ">$brtfn" or die "Can not create $brtfn\n";
$dimfn = "$fnbase"."_dim.edb";# name of file for dim asteroids
open DIM, ">$dimfn" or die "Can not create $dimfn\n";
print "Creating $brtfn and $dimfn..\n";

# build some common boilerplate
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime;
$year += 1900;
$mon += 1;
$from = "# Data is from ftp://cfa-ftp.harvard.edu/pub/MPCORB/MPCORB.DAT\n";
$what = "# Generated by mpcorb2edb.pl $ver, (c) 2000 Elwood Downey\n";
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
# process each mpcorb.dat entry
while (<$srcfd>) {
    chomp();
    if (/^-----------/) {
	$sawstart = 1;
	next;
    }
    next unless ($sawstart);

    # build the name
    $name = &s(167, &min(length(),194));
    $name =~ s/[\(\)]//g;
    $name =~ s/^ *//;
    $name =~ s/ *$//;
    next if ($name eq "");

    # gather the orbital params
    $i = &s(60,68) +  0;
    $O = &s(49,57) +  0;
    $o = &s(38,46) +  0;
    $a = &s(93,103) + 0;
    $e = &s(71,79) +  0;
    $M = &s(27,35) +  0;
    $H = &s(9,13) +   0;
    $H = 100 if ($H == 0);	# beware blank field
    $G = &s(15,19) +  0;

    $cent = &s(21,21);
    $TY = &s(22,23) + 0;
    $TY += 1800 if ($cent =~ /I/i);
    $TY += 1900 if ($cent =~ /J/i);
    $TY += 2000 if ($cent =~ /K/i);
    $TM = &mpcdecode (&s(24,24)) + 0;
    $TD = &mpcdecode (&s(25,25)) + 0;

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
    print $fd "$name";
    print $fd ",e,$i,$O,$o,$a,0,$e,$M,$TM/$TD/$TY,2000.0,$H,$G\n";
}

# remove fetched files, if new
if (defined($fetchok)) {
    unlink $MPCFILE;
    unlink $MPCZIPFILE;
}

print "Done\n";
exit 0;

# like substr($_,first,last), but one-based.
sub s
{
    substr ($_, $_[0]-1, $_[1]-$_[0]+1);
}

# return log base 10
sub log10
{
    .43429*log($_[0]);
}

# return absolute value
sub absv
{
    $_[0] < 0 ? -$_[0] : $_[0];
}

# return decoded value
sub mpcdecode
{
    my $x = $_[0];
    $x =~ /\d/ ? $x : sprintf "%d", 10 + ord ($x) - ord ("A");
}

# return min of two values
sub min
{
    $_[0] < $_[1] ? $_[0] : $_[1];
}

# print usage message then die
sub usage
{
    my $base = $0;
    $base =~ s#.*/##;
    print "Usage: $base [-f] <base>\n";
    print "$ver\n";
    print "Purpose: convert $MPCFILE to 2 .edb files.\n";
    print "Options:\n";
    print "  -f: first $MPCFILE from $MPCSITE, else read from stdin\n";
    print "Creates two files:\n";
    print "  <base>.edb:     all asteroids ever brighter than $dimmag\n";
    print "  <base>_dim.edb: all asteroids never brighter than $dimmag\n";

    exit 1;
}

# get and unzip the data
sub fetch
{
    # transfer
    print "Getting $MPCFTPDIR/$MPCZIPFILE from $MPCSITE...\n";
    $cmd = "curl -connect-timeout 10 -s -u 'anonymous:xephem\@clearskyinstitute.com' $MPCSITE/$MPCFTPDIR/$MPCZIPFILE > $MPCZIPFILE";
    print "$cmd\n";
    !system "$cmd" or exit(1);

    # extract into current dir
    print "Decompressing $MPCZIPFILE...\n";
    unlink $MPCFILE;			# avoid unzip asking OK to overwrite
    !system "gunzip $MPCZIPFILE" or die "$MPCZIPFILE: gunzip failed\n";
    -s $MPCFILE or die "$MPCFILE: failed to create from unzip $MPCZIPFILE\n";

    # flag
    $fetchok = 1;
}

# For RCS Only -- Do Not Edit
# @(#) $RCSfile: mpcorb2edb.pl,v $ $Date: 2014/07/11 02:46:32 $ $Revision: 1.3 $ $Name:  $
