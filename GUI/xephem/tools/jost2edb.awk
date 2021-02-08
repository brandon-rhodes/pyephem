#!/bin/awk -f
# convert from Jost Jahn postings format to {x}ephem format
# we don't try to select out non-entries.
# v2 11/24/94

{
	# pick out the name; put year designation first if find it.
	name = substr ($0, 1, 17)
	gsub ("[()]", "|", name)
	nn = split (name, desig, "|")
	if (nn == 3) {
	    # looks like name is something like "XXXX (1992x)"
	    name = sprintf ("%s %s", desig[2], desig[1])
	} else
	    name = desig[1]
	sub ("[ ]+$", "", name)

	# inclination
	i = substr ($0, 64, 7) + 0

	# long of asc node
	O = substr ($0, 56, 7) + 0

	# arg of peri
	o = substr ($0, 48, 7) + 0

	# eccentricity
	e = substr ($0, 40, 7) + 0

	# epoch of peri
	# T = substr($0,22,2)+0 "/" substr($0,25,6)+0 "/19" substr($0,19,2)+0
	T = sprintf ("%g/%g/19%g", substr($0,22,2)+0, substr($0,25,6)+0,
							substr($0,19,2)+0)

	# peri distance
	q = substr($0,32,7) + 0

	# mag model -- says H/G but works with my g/k
	g = substr($0,72,4)
	k = substr($0,77,3)

	if (e < 1) {
	    # elliptical
	    a = q/(1-e)
	    printf "%s,e,%g,%g,%g,%g,%g,%g,%g,%s,2000,g%g,%g\n", \
		    name, i, O, o, a, 0, e, 0, T, g, k
	} else if (e > 1) {
	    # hyperbolic
	    printf "%s,h,%s,%g,%g,%g,%g,%g,2000,%g,%g\n", \
		    name, T, i, O, o, e, q, g, k
	} else {
	    # parabolic
	    printf "%s,p,%s,%g,%g,%g,%g,2000,%g,%g\n", \
		    name, T, i, o, q, O, g, k
	}
}

# For RCS Only -- Do Not Edit
# @(#) $RCSfile: jost2edb.awk,v $ $Date: 1998/04/30 02:19:21 $ $Revision: 1.1 $ $Name:  $
