#!/usr/bin/perl
# start one instance of xephemdbd

# full path to catalogs
$dbdir = '/usr/local/xephem/catalogs';

# define helpers
$lkfile = 'xephemdbd.pid';
$logfn = "xephemdbd.log";
$infifo = 'xephemdbd.in';

# create a fifo unique to this instance
system "mkfifo $infifo" if (! -e $infifo);

# start xephemdbd with this fifo
system "./xephemdbd -vtlic 0 $lkfile $infifo $dbdir >> $logfn 2>&1";
