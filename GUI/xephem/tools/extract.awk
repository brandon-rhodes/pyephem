#!/bin/sh
#
# (l) 1996 by Pawel T. Jochym <ptj@pkpf.ifj.edu.pl>
#
# $Header: /home/ecdowney/telescope/GUI/xephem/tools/RCS/extract.awk,v 1.1 1996/08/26 19:40:53 ecdowney Exp $
#
# Shell (bash)/awk (gawk) script for extracting
# Comets orbital elements from various internet postings.
# It creates on stdout records in XEphem edb format from
# analysis of files enumerated in command line.
#
# Synopsis
#   extr [file1 file2 ...]
# 
# History:
#
# $Log: extract.awk,v $
# Revision 1.1  1996/08/26 19:40:53  ecdowney
# Initial revision
#
# Revision 1.2  1996/06/02  15:49:32  jochym
# First full implementation of Don Yeomas format decoder.
#
# Revision 1.1  1996/06/02  11:11:20  jochym
# Initial revision
#
#
# Yes, this is a free software (GPL).
# 
# You can always send comments, additions, 
# bug raports, money and beautiful woman 
# to the above address.
#

SUFF=$$
TMPDIR="/tmp"

# Create file names

AWKENCKE=$TMPDIR/extract.encke.$SUFF
AWKYEOMAS=$TMPDIR/extract.yeomas.$SUFF
AWKSKYMAP=$TMPDIR/extract.skymap.$SUFF
AWKCOMET=$TMPDIR/extract.comet.$SUFF
AWKREC=$TMPDIR/extract.type.$SUFF
AWKTYPEOUT=$TMPDIR/extract.out.$SUFF

# Create awk scripts

# Type of file recognizer

cat >$AWKREC <<"EOF"
# AWK script for recognizing file type
# Returns (prints to stdout) name of file type
# ID's are:
# Yeomas
# Encke
# Comet
# SkyMap
# Unrecognized

# Initialization

BEGIN{
	result="Unrecognized";
	yeomas=0;
	encke=0;
	comet=0;
	skymap=0;
}

END{
	print result
}

# Don Yeomas format

/Don/ && /Yeomas/{
	result="Yeomas"
	if (++yeomas>3)	exit;	
}

/Planetary/ && /Ephemeris/{
	result="Yeomas"
	if (++yeomas>3)	exit;
}

/\(with/ && /perturbations\)/{
	result="Yeomas"
	if (++yeomas>3)	exit;
}

/Moon-Earth-Object/{
	result="Yeomas"
	if (++yeomas>3)	exit;
}

/Corrected/ && /Elements/ && /Solution/{
	result="Yeomas"
	if (++yeomas>3)	exit;
}
 
# Encke

/TWO/ && /BODY/ && /EPHEMERIS/ && /GENERATOR/{
	result="Encke";
	exit;
}

# Comet page format


/Latitude/ && /Longitude/ && /Magnetic/{
	result="Comet"
	exit;
}


# Sky Map page (html) format

/SkyMap/ && /Comet/ && /Orbital/{
	result="SkyMap"
	exit;
}

/Refer/ && /bottom/ && /page/ && /explanation/{
	result="SkyMap"
	exit;
}

EOF

#Result line generator
cat >$AWKTYPEOUT <<"EOF"
#Type out the result 
# Get params in the following order:
# T e i o O q g k name
{
#	print "#  Got: "$0
	if ( substr($1,1,1) == "#" ){
		print "# " $0 ;
	}else{
    	T=$1;
    	e=$2+0.0;
    	i=$3+0.0;
    	o=$4+0.0;
    	O=$5+0.0;
    	q=$6+0.0;
	epoch=$7+0.0;
    	g=$8+0.0;
    	k=$9+0.0;
    	name=$10
    	for( l=11; l<=NF; l++ ) name=name " " $(l);
    	design=sprintf("%.13s",$10" "$11);
	design=sprintf("%s",$10" "$11);
	
    	# print name, design
    
    	if (e < 1) {
        	# elliptical
        	a = q/(1-e);
			printf "%-.13s,e,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,%s,%.12g,g%g,%g #%s\n", \
	    		design, i, O, o, a, 0, e, 0, T, epoch, g, k, name ;
    	} else if (e > 1) {
        	# hyperbolic
			printf "%-.13s,h,%s,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,g%g,%g #%s\n", \
	    		design, T, i, O, o, e, q, epoch, g, k, name ;
    	} else {
        	# parabolic
			printf "%-.13s,p,%s,%.12g,%.12g,%.12g,%.12g,%.12g,g%g,%g #%s\n", \
	    		design, T, i, o, q, O, epoch, g, k, name ;
    	}
	}
}

EOF

#SkyMap posting extractor

cat >$AWKSKYMAP <<"EOF"
# AWK script for extracting orbital elements
# from SkyMap postings

BEGIN{
#	print "# Sky Map decoder not implemented"

	state=0;
	CONVFMT="%.12g"
	OFMT="%.12g"

	mon["Jan"]=1;
	mon["Feb"]=2;
	mon["Mar"]=3;
	mon["Apr"]=4;
	mon["May"]=5;
	mon["Jun"]=6;
	mon["Jul"]=7;
	mon["Aug"]=8;
	mon["Sep"]=9;
	mon["Oct"]=10;
	mon["Nov"]=11;
	mon["Dec"]=12;

	epoch=2000.0;
	g=5;
	k=4;
}

/\<\/PRE\>/{
	if ( state==6 || state==8 ) {
		printf ("%s %.12g %.12g %.12g %.12g %.12g %.12g %.12g %.12g %s\n", \
			T, e, i, o, O, q, epoch, g, k, name);
		epoch=2000.0;
		g=5;
		k=4;
		state=0;
	}	
}

/\<HR\>/{ 
	if ( state==0 ) state=1;
	
}

/\<H4\>/ && /\<\/H4\>/{
	if ( state==1 ) {
		state=2;
		name=substr($0,10);
		name=substr(name,2,length(name)-6);
		#	print "#"name
#		if ( substr(name,2,1) == "/" ) {
#			# tmpname=substr(name,6);
#			name=substr(name,3,4)"/"substr(name,1,1)substr(name,7);
#			#	print "#"name
#		}
	}
}

/\<PRE\>/{
	if ( state==2 ) {
		state=3 ;
	}
}

/Epoch/{
	if (state==3) {
		#This is anly an approximation !
		epoch=$3+(mon[$4]-1)/12+($5/365);
	}
}

/T:/ && /w:/ {
	if ( state==3 ) {
		state=4;
		o=$7+0.0;
		T=sprintf("%d/%g/%d", mon[$3]+0, $4+0.0, $2+0);
	}
}

/e:/ && /W:/ {
	if ( state==4 ) {
		state=5;
		O=$4+0.0;
		e=$2+0.0;
	}
}

/q:/ && /i:/ {
	if ( state==5 ) {
		state=6;	#exit state
		q=$2+0.0;
		i=$4+0.0;
	}
}

/Magnitude/ && /parameters/{
	if (state==6) {
		state=7;
	}
}

/H\ =/ && /G\ =/{
	if (state==7) {
		state=8;	#exit state
		g=$3+0.0;
		k=$6+0.0;
	}
}


EOF



#Comets page posting extractor

cat >$AWKCOMET <<"EOF"
# AWK script for extracting orbital elements
# from Comets page postings

BEGIN{
}


EOF

#Don Yeomas posting extractor

cat >$AWKYEOMAS <<"EOF"
# AWK script for extracting orbital elements
# from Don Yeomas postings

BEGIN{
#	print "# Don Yeomas decoder"

	mon["Jan"]=1;
	mon["Feb"]=2;
	mon["Mar"]=3;
	mon["Apr"]=4;
	mon["May"]=5;
	mon["Jun"]=6;
	mon["Jul"]=7;
	mon["Aug"]=8;
	mon["Sep"]=9;
	mon["Oct"]=10;
	mon["Nov"]=11;
	mon["Dec"]=12;

	epoch=2000.0;
	object = ""
	state = 0
	nr_elems = 0;
}


END{
	if (nr_elems>0) {
		for( l=0; l<nr_elems; l++ ) { 
			time=epochs[l]
			print "#  Elements for "object" at "time
			typeout(elems[time,"T"],\
				elems[time,"e"],\
				elems[time,"i"],\
				elems[time,"o"],\
				elems[time,"O"],\
				elems[time,"q"],\
				elems[time,"epoch"],\
				elems[time,"g"],\
				elems[time,"k"],\
				elems[time,"com"]\
			)
		}
	}
}

function addelem( time, T, e, i, o, O, q, epoch, comment )
{
	epochs[nr_elems++]=time
	elems[time,"T"]=T
	elems[time,"e"]=e
	elems[time,"i"]=i
	elems[time,"o"]=o
	elems[time,"O"]=O
	elems[time,"q"]=q
	elems[time,"epoch"]=epoch
#	nr_of_fields=split(comment,arr);
#	comment=""
#	arr[2]=arr[2]"_"nr_elems;
#	for( l=1; l<=nr_of_fields; l++ ) comment=comment" "arr[l];
	elems[time,"com"]=comment
}

function updategk( g, k )
{
	for( l=0; l<nr_elems; l++ ) {
		elems[epochs[l],"g"]=g
		elems[epochs[l],"k"]=k
	}
}

function typeout( T, e, i, o, O, q, epoch, g, k, comment )
{
    printf "%s %.12g %.12g %.12g %.12g %.12g %.12g %.12g %.12g %s\n", T, e, i, o, O, q, epoch, g, k, comment 
}

# Pattern based recognizers implementing state machine

/Object:/ && /Comet/{
	for(l=3;l<=NF;l++) {object=object" "$l;}
#	print "# Object is: "object
}

/Corrected/ && /Elements/ && /Solution/{
	if (state!=0) {print "#  WHAT ?! state="state" in "; exit} else state++;
}

/Epoch/ && /=/{
	if (state==1) {
		etime=$6"/"$5"/"$4
	};
}

/Post-Fit/ && /Std.Dev./{
	if (state!=1) {print "#  WHAT ?!"; exit} else state++;
}


/Elements/ && /at/ && /other/ && /epochs/{
	if (state==9) {
		state++
#		print "# Other epochs block"
	} else {
		print "#  WHAT ?! state="state" in "$0
	}
}

/Epoch/ && /e/ && /q/ && /Node/ && /w/ && /i/ && /Tp/{
	if (state==10) {
		state++
#		print "# Header line"
	} else {
		print "#  WHAT ?! state="state" in "$0
	}
}

/log/ && /Delta/ && /log/{
	if (state==12) {
		state++
	} else {
		print "#  WHAT ?! state="state" in "$0
	}
}


{
	if ((state>1) && (state<8)) { # We are in the main elements block - collect the data
		if ($1=="e") { e=$2+0.0 ; state++ }
		else if ($1=="q") { q=$2+0.0 ; state++ }
		else if ($1=="Tp") { T=mon[$5]"/"$6"/"$4 ; state++ }
		else if ($1=="Node") { O=$2+0.0 ;  state++ }
		else if ($1=="w") { w=$2+0.0 ; state++ }
		else if ($1=="i") { i=$2+0.0 ; state++ }
		else if ($1=="Post-Fit") ; 
		else print "# WHAT ?! state="state" in "$0;
	}else if (state == 8) { # We are past main block - add the result and search for additionals
#		print "#  Elements for epoch: "etime
		addelem( etime, T, e, i, w, O, q, 2000.0, object );
		state++;
	}else if ((state == 11) && (NF==11)) { # We are at other epochs lines collect the data
		etime=$3"/"$2"/"$1
#		print "#  Elements for epoch: "etime
		T=mon[$10]"/"$11"/"$9
		addelem( etime, T, $4+0.0, $8+0.0, $7+0.0, $6+0.0, $5+0.0, 2000.0, object );
	} else if ((state == 11) && (NF==8)) { # We are at other epochs lines header - do nothing
		;
	} else if ((state == 11)) { # End of other epochs block - search for magnitude model
#		print "# End of other epochs, NF="NF" state="state
		state++
	} else if (state==13) { # Magnitude line found - analyse it
		state++;
		# find position of first plus sign first
		g_base=-1
		k_base=-1
		for(l=1;l<=NF;l++) {
			if (index($l,"+")!=0) {
				if (g_base==-1) {
					g_base=l;
				} else if (k_base==-1) { 
					k_base=l;
				}
			}
		}
#		print "#  g_base="g_base" k_base="k_base
		g=$(g_base-1)+0.0
		k=$(k_base+1)
		gsub("*log(r)","",k);
		k=k/2.5
#		print "#  TMag line g="g"  k="k" - "$0
		updategk( g, k )
	} else if (state==14) { # all done - type the result
		state++;
	}	 
}

EOF

#Encke posting extractor

cat >$AWKENCKE <<"EOF"
# AWK script for extracting orbital elements
# from encke postings

# Initialize
BEGIN{
    FS="[=*+ \(\)]+"
    state=0;
    CONVFMT="%.12g"
    OFMT="%.12g"
}

#Type out the result 
END{
    printf "%s %.12g %.12g %.12g %.12g %.12g %.12g %.12g %.12g %s\n", T, e, i, o, O, q, 2000.0, g, k, comment 
}

# Pattern based recognizers

#Total magnitude line 
/TOTAL/ && /MAGNITUDE/ {
#    print "g=", $5," k=", $9 ;
#    print $1, $2, $3, $4, $5, $6, $7, $8, $9, $10 ;
    g=$5+0.0;
    k=($9+0.0)/2.5;
}

#Name line
/EPHEMERIS/ && /FOR/ {
#    print "Comet: " $8, $9, $10 ;
	if ( $8=="Comet" ) {
		pos=9;
	} 	
	else {
		pos=8;
    };
    name = $(pos)$(pos+1);
    comment="";
    for (j=pos; j<NF; j++) {
		if (j==pos) {
			sub("/P","P",$(j));
			sub("P/","P ",$(j));
		}else{
			 sub("P/","P ",$(j));
		}
		comment=comment" "$(j);
#      	if ( index($(j),"C/")==1 ){
#			comment = comment" "substr($(j),3);
#		}else{
#			comment = comment" "$(j);
#		}
	}
}

#Pre-elements lines
/THE/ && /ABOVE/ && /COMET/ && /EPHEMERIS/ && /IS/ && /BASED/{
#    print "here 1:",$0 ;
    if (state==0){
	state=1 ;
#       print 1 ;
    }
}

/YR/ && /MN/ && /DY/ && /HR/{
#    print "here 2" ;
    if (state==1){ 
	state=2 ;
#        print 2 ;
    }
}


{
    # Main parameter colector
    
#    print ":"$1":"$2":"$3":" $4":" $5":";

    if (state==3){
        # print "analyse" 
        # inclination 
	i = $11 + 0 ;

        # long of asc node
	O = $10 + 0 ;

        # arg of peri
	o = $9 + 0 ;

	# eccentricity
	e = $8 + 0 ;

	# epoch of peri
	T = sprintf ("%g/%g/%g", $3+0, $4+$5/24.0, $2+0) ;

        # Year of peryhelion
	y = $2 + 0 ;

#        print y, $2 ;

	# peri distance
	q = $7 + 0 ;
        
        # I have got elements
	state = 4 ;
    }
    if (state==2){
	state=3 ;
#        print 3 ;
    }
}
EOF

# Run the script

for infile in $*; do
    echo "# Elements from file: $infile ..."
	case `awk -f $AWKREC < $infile` in
		"Encke") awk -f $AWKENCKE < $infile ;;
		"SkyMap") awk -f $AWKSKYMAP < $infile ;;
		"Comet") awk -f $AWKCOMET < $infile ;;
		"Yeomas") awk -f $AWKYEOMAS < $infile ;;
		"Unrecognized")	echo \# Unrecognized file format ;;
		*) echo "# What ?!" ;;
	esac
done | awk -f $AWKTYPEOUT

# Clean up

#less $AWKENCKE

rm $AWKYEOMAS
rm $AWKCOMET
rm $AWKENCKE
rm $AWKSKYMAP
rm $AWKREC
rm $AWKTYPEOUT

















# For RCS Only -- Do Not Edit
# @(#) $RCSfile: extract.awk,v $ $Date: 1996/08/26 19:40:53 $ $Revision: 1.1 $ $Name:  $
