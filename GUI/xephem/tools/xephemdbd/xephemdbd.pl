#!/usr/bin/perl
# xephemdbd.pl: digest a GET form request and process using xephemdbd.
# (c) 1998 Elwood Charles Downey. All rights reserved.
# 24 Feb 98: begin
# 11 Mar 98: final tweaks

# generally no need to change anything from here on.
# ==================================================

# set $trace to 1 to see steps in $tracefile
$tracefile = 'xephemdbd.trace';
$trace = 1;

# set to lock file location
$lkfile = 'xephemdbd.pid';

# set to common input fifo location
$infifo = 'xephemdbd.in';

# set to unique output fifo location
$outfifo = "xephemdbd.$$";

# set to xephemdbd log file
$logfn = "xephemdbd.log";

# certainly no need to change anything from here on.
# ==================================================

# version number
$ver = 3;

# get some handy utility functions.
use Time::Local;
require "cgi-lib.pl";

# autoflush output
&afon (STDOUT);

# set up log file
open (LOG, ">>$logfn") || !print "<p>$logfn $!\n" || goto wrapup;
&afon (LOG);
printf LOG "xephemdbd.pl $$ started %s UTC from $ENV{'REMOTE_ADDR'}\n", gmtime()."";

# set up trace file, if enabled
!$trace || open(TR,">>$tracefile") || !print "<p>$tracefile $!\n" || goto wrapup;
!$trace || &afon (TR);
!$trace || printf TR "xephemdbd.pl $$ started %s UTC from $ENV{'REMOTE_ADDR'}\n", gmtime()."";

# Read in all the variables set by the form so we can access as $input{'name'}
&ReadParse(*input);

# Print the HTML header once -- match the default xephem colors.
print "Content-type: text/html\n\n";
print "<html>\n";
print "<head><title>xephemdbd results</title></head>\n";
print "<body bgcolor=\"#f3e3b2\" text=\"#1010f0\">\n";
print "<H2>XEphemdbd Results</H2>\n";

# check versions match.
$tmp = $input{'VERSION'};
!$trace || print TR "  checking version match";
if ($tmp != $ver) {
    print "<p>Wrong version: $ver <i>vs.</i> $tmp\n";
    goto wrapup;
}

# make sure we have the fifo to the xephemdbd
!$trace || print TR "  checking $infifo\n";
if (! -p $infifo && system ("mkfifo $infifo")) {
    print "<p>Can not create fifo $infifo: $!\n";
    goto wrapup;
}

# start xephemdbd -- it knows to exit if already running.
!$trace || print TR "  checking for xephemdbd\n";
if (system ("./start-xephemdbd.pl")) {
    print "<p>Can not start xephemdbd\n";
    goto wrapup;
}

# start building up the command to xephemdbd in $ecmd in the following format:
# >file,outputmode,types,year,RA,Dec,FOV,Mag[,lat,long,elev]
!$trace || print TR "  cracking FORM args\n";
$ecmd = ">$outfifo";

# gather the basic options 
$ops = 0;
$ops += 1 if $input{'FMT'} eq 'TXT';
$ops += 2 if $input{'CENTRIC'} eq 'TOPO';
$ops += 4 if $input{'PRECES'} eq 'APP';
$ops += 8 if $input{'FMT'} eq 'TXT';
$ecmd .= ",$ops";

# gather the desired object types
$obj = 0;
$obj +=   1 if $input{'PL'};
$obj +=  14 if $input{'SS'};
$obj +=  16 if $input{'BS'};
$obj += 224 if $input{'DS'};
$obj += 256 if $input{'PP'};
$obj += 512 if $input{'GS'};
$ecmd .= ",$obj";

# gather the date and time as a decimal year
$tmp = $input{'DATE'};
if (!$tmp) {
    print "<p>Date is required\n";
    goto wrapup;
}
if ($tmp =~ /now/i) {
    $tm = time();
} else {
    ($month, $day, $year) = split (/\D/, $tmp);
    $tmp = $input{'TIME'};
    if (!$tmp) {
	print "<p>Time is required\n";
	goto wrapup;
    }
    ($hr, $min, $sec) = split (/\D/, $tmp);
    $tm = timegm ($sec, $min, $hr, $day, $month-1, $year-1900);
}
($sec, $min, $hr, $day, $month, $year) = gmtime ($tm);
$tmyr0 = timegm (0, 0, 0, 1, 0, $year);
$tmyr1 = timegm (0, 0, 0, 1, 0, $year+1);
$decyear = 1900 + $year + ($tm - $tmyr0)/($tmyr1 - $tmyr0);
$ecmd .= ",$decyear";

# get the RA in rads
$tmp = $input{'RA'};
if (!$tmp) {
    print "<p>RA is required\n";
    goto wrapup;
}
$tmp = &sextorads ($tmp)*15;
$ecmd .= ",$tmp";

# get Dec in rads
$tmp = $input{'DEC'};
if (!$tmp) {
    print "<p>DEC is required\n";
    goto wrapup;
}
$tmp = &sextorads ($tmp);
$ecmd .= ",$tmp";

# get FOV in rads, limit to 15 degrees
$tmp = $input{'FOV'};
if (!$tmp) {
    print "<p>FOV is required\n";
    goto wrapup;
}
if ($tmp > 15) {
    print "<p>FOV is limited to 15 degrees\n";
    goto wrapup;
}
$tmp *= 0.0174533;
$ecmd .= ",$tmp";

# get limiting magnitude
$tmp = $input{'MAG'};
if (!$tmp) {
    print "<p>Limiting magnitude is required\n";
    goto wrapup;
}
$ecmd .= ",$tmp";

if ($ops & 6) {
    # if computing topocentric or apparent position, need location too
    $tmp = $input{'LAT'};
    if (!$tmp) {
	print "<p>Latitude is required\n";
	goto wrapup;
    }
    $lat = &sextorads ($tmp);
    $ecmd .= ",$lat";

    $tmp = $input{'LONG'};
    if (!$tmp) {
	print "<p>Longitude is required\n";
	goto wrapup;
    }
    $lng = &sextorads ($tmp);
    $ecmd .= ",$lng";

    $elev = $input{'ELEV'};
    $ecmd .= ",$elev";
}

!$trace || print TR "  command = `$ecmd'\n";

# give command to xephemdbd to use outfifo and echo results
!$trace || print TR "  creating $outfifo\n";
if (!system ("mkfifo -m 0666 $outfifo")) {
    !$trace || print TR "  opening $infifo\n";
    if (open (EIN, ">>$infifo")) {
	&afon(EIN);
	!$trace || print TR "  sending command\n";
	if (print EIN "$ecmd\n") {
	    !$trace || print TR "  opening $outfifo\n";
	    if (open (EOUT, "<$outfifo")) {
		!$trace || print TR "  reading $outfifo\n";
		# finally! copy result back to client
		print "<pre>\n";
		while (<EOUT>) {
		    print "$_";
		}
		print "</pre>\n";
		!$trace || print TR "  closing $outfifo\n";
		close (EOUT);
	    } else {
		!$trace || print TR "  opening $outfifo failed\n";
		print "<p>Opening $outfifo failed: $!\n";
		goto wrapup;
	    }
	} else {
	    !$trace || print TR "  printing to $infifo failed\n";
	    print "<p>Print to $infifo failed: $!\n";
	    goto wrapup;
	}
	close (EIN);
    } else {
	!$trace || print TR "  opening $infifo failed\n";
	print "<p>Opening $infifo failed: $!\n";
	goto wrapup;
    }
} else {
    !$trace || print TR "  creating $outfifo failed\n";
    print "<p>Creating $outfifo failed: $!\n";
    goto wrapup;
}

!$trace || print TR "  finished successfully\n";

# final boilerplate
wrapup:
!$trace || print TR "  wrapup\n";

# remove temp response fifo
!$trace || print TR "  unlinking $outfifo\n";
unlink ($outfifo);

print <<ENDOFTEXT;
<p>Use your browser's \"Go back\" feature to modify the same request.
</body></html>

<P>
&#169; 1998 Elwood Charles Downey. All rights reserved.
<P>
For more astronomy software and services, see <A HREF="http://www.ClearSkyInstitute.com"> www.ClearSkyInstitute.com </A>
ENDOFTEXT

exit (0);

# call with a sexigesimal string in @_[0] and return in rads
sub sextorads {
    local ($v) = @_[0];
    $v =~ s/\+//;
    $v =~ s/(-)//;
    local ($sign) = $1 eq "-" ? -1 : 1;
    local ($d, $m, $s) = split (/\D/, $v);
    local ($a) = ($s/3600 + $m/60 + $d)*.0174533;
    return ($a * $sign);
}

# turn on autoflush for the filehandle passed in @_[0]
sub afon {
    # "bizarre and obscure", Programming Perl, page 211
    select ((select(@_[0]), $| = 1)[0]);
}

# For RCS Only -- Do Not Edit
# @(#) $RCSfile: xephemdbd.pl,v $ $Date: 2003/12/05 06:27:38 $ $Revision: 1.5 $ $Name:  $
