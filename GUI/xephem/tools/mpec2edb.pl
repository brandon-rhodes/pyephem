#!/usr/bin/perl
# convert Minor Planet Electronic Circular, MPEC, to edb format

# Sample:
# Orbital elements:
# 2003 UB313
# Epoch 2005 Aug. 18.0 TT = JDT 2453600.5                 MPC
# M 197.53790              (2000.0)            P               Q
# n   0.00176902     Peri.  151.31153     -0.91258509     -0.02028701
# a  67.7091000      Node    35.87500     -0.34877687     -0.48266077
# e   0.4416129      Incl.   44.17700     +0.21340843     -0.87557240
# P 557              H   -1.1           G   0.15           U   5

while (<>) {
    if (/Orbital elements/) {
	chomp ($name = <>);
	($e,$y,$m,$d) = split (/ +/, <>);
	next unless ($e =~ /Epoch/);
	($x,$M,$ep) = split (/[ ()]+/, <>);
	($x,$n,$x,$P) = split (/ +/, <>);
	($x,$a,$x,$N) = split (/ +/, <>);
	($x,$e,$x,$i) = split (/ +/, <>);
	($x,$x,$x,$H,$x,$G) = split (/ +/, <>);

	print "$name,e,$i,$N,$P,$a,$n,$e,$M,8/$d/$y,$ep,H$H,$G\n",
    }
}

