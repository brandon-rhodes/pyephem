/* DeltaT = Ephemeris Time - Universal Time
 *
 * original code by Stephen Moshier <moshier@world.std.com>,
 * adapted to xephem by Michael Sternberg <sternberg@physik.tu-chemnitz.de>
 * smoothed transitions and updated by Neal McBurnett <nealmcb@bell-labs.com>
 *
 **********************************************************************
 *
 * The tabulated values of deltaT, in hundredths of a second,
 * were taken from The Astronomical Almanac, page K8.  The program
 * adjusts for a value of secular tidal acceleration ndot = -25.8
 * arcsec per century squared, the value used in JPL's DE403 ephemeris.
 * ELP2000 (and DE200) used the value -23.8946.
 *
 * The tabulated range is 1620.0 through 1998.0.  Bessel's interpolation
 * formula is implemented to obtain fourth order interpolated values at
 * intermediate times.
 *
 * For dates earlier than the tabulated range, the program
 * calculates approximate formulae of Stephenson and Morrison
 * or K. M. Borkowski.  These approximations have an estimated
 * error of 15 minutes at 1500 B.C.  They are not adjusted for small
 * improvements in the current estimate of ndot because the formulas
 * were derived from studies of ancient eclipses and other historical
 * information, whose interpretation depends only partly on ndot.
 *
 * For future vaues of deltaT, the function smoothly transitions with
 * a linear segment back to Stephenson & Morrison's quadratic formula
 * in 2130.
 *
 * Input is mj (modified julian date from MJD0 on). [stern]
 *  Note that xephem uses a different epoch for this "mj" than the
 *  normal value of JD=240000.5.
 * See AA page B4.
 *
 * Output double deltat(mj) is ET-UT1 in seconds.
 *
 *
 * References:
 *
 * Stephenson, F. R., and L. V. Morrison, "Long-term changes
 * in the rotation of the Earth: 700 B.C. to A.D. 1980,"
 * Philosophical Transactions of the Royal Society of London
 * Series A 313, 47-70 (1984)
 *
 * Borkowski, K. M., "ELP2000-85 and the Dynamical Time
 * - Universal Time relation," Astronomy and Astrophysics
 * 205, L8-L10 (1988)
 * Borkowski's formula is derived from eclipses going back to 2137 BC
 * and uses lunar position based on tidal coefficient of -23.9 arcsec/cy^2.
 *
 * Chapront-Touze, Michelle, and Jean Chapront, _Lunar Tables
 * and Programs from 4000 B.C. to A.D. 8000_, Willmann-Bell 1991
 * Their table agrees with the one here, but the entries are
 * rounded to the nearest whole second.
 *
 * Stephenson, F. R., and M. A. Houlden, _Atlas of Historical
 * Eclipse Maps_, Cambridge U. Press (1986)
 *
 * from obsolete extrapolation code [stern]:
 * Morrison, L. V. and F. R. Stephenson, "Sun and Planetary System"
 * vol 96,73 eds. W. Fricke, G. Teleki, Reidel, Dordrecht (1982)
 *
 **********************************************************************
 *
 * changes by stern:
 *   - adopted #include's for xephem
 *   - made dt[] static
 *   - made mj the time argument [was: year Y].
 *   - updated observed and extrapolated data from tables at
 *	ftp://maia.usno.navy.mil/ser7/ -- data deviated by up to 0.8 s
 *   - removed references to "extern double dtgiven"
 *   - removed DEMO #define and its references
 *   - replaced treatment after TABEND by linear extrapolation instead
 *	of second order version
 *   - installed lastmj cache (made ans static)
 *
 *   - no changes to table interpolation scheme and past extrapolations */

#include <math.h>

#include "astro.h"

#define TABSTART 1620.0
#define TABEND 2006.0
#define TABSIZ 387

/* Note, Stephenson and Morrison's table starts at the year 1630.
 * The Chapronts' table does not agree with the Almanac prior to 1630.
 * The actual accuracy decreases rapidly prior to 1780.
 */
static short dt[TABSIZ] = {
    /* 1620.0 thru 1659.0 */
    12400, 11900, 11500, 11000, 10600, 10200, 9800, 9500, 9100, 8800,
    8500, 8200, 7900, 7700, 7400, 7200, 7000, 6700, 6500, 6300,
    6200, 6000, 5800, 5700, 5500, 5400, 5300, 5100, 5000, 4900,
    4800, 4700, 4600, 4500, 4400, 4300, 4200, 4100, 4000, 3800,
    /* 1660.0 thru 1699.0 */
    3700, 3600, 3500, 3400, 3300, 3200, 3100, 3000, 2800, 2700,
    2600, 2500, 2400, 2300, 2200, 2100, 2000, 1900, 1800, 1700,
    1600, 1500, 1400, 1400, 1300, 1200, 1200, 1100, 1100, 1000,
    1000, 1000, 900, 900, 900, 900, 900, 900, 900, 900,
    /* 1700.0 thru 1739.0 */
    900, 900, 900, 900, 900, 900, 900, 900, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1100, 1100, 1100,
    1100, 1100, 1100, 1100, 1100, 1100, 1100, 1100, 1100, 1100,
    1100, 1100, 1100, 1100, 1200, 1200, 1200, 1200, 1200, 1200,
    /* 1740.0 thru 1779.0 */
    1200, 1200, 1200, 1200, 1300, 1300, 1300, 1300, 1300, 1300,
    1300, 1400, 1400, 1400, 1400, 1400, 1400, 1400, 1500, 1500,
    1500, 1500, 1500, 1500, 1500, 1600, 1600, 1600, 1600, 1600,
    1600, 1600, 1600, 1600, 1600, 1700, 1700, 1700, 1700, 1700,
    /* 1780.0 thru 1799.0 */
    1700, 1700, 1700, 1700, 1700, 1700, 1700, 1700, 1700, 1700,
    1700, 1700, 1600, 1600, 1600, 1600, 1500, 1500, 1400, 1400,
    /* 1800.0 thru 1819.0 */
    1370, 1340, 1310, 1290, 1270, 1260, 1250, 1250, 1250, 1250,
    1250, 1250, 1250, 1250, 1250, 1250, 1250, 1240, 1230, 1220,
    /* 1820.0 thru 1859.0 */
    1200, 1170, 1140, 1110, 1060, 1020, 960, 910, 860, 800,
    750, 700, 660, 630, 600, 580, 570, 560, 560, 560,
    570, 580, 590, 610, 620, 630, 650, 660, 680, 690,
    710, 720, 730, 740, 750, 760, 770, 770, 780, 780,
    /* 1860.0 thru 1899.0 */
    788, 782, 754, 697, 640, 602, 541, 410, 292, 182,
    161, 10, -102, -128, -269, -324, -364, -454, -471, -511,
    -540, -542, -520, -546, -546, -579, -563, -564, -580, -566,
    -587, -601, -619, -664, -644, -647, -609, -576, -466, -374,
    /* 1900.0 thru 1939.0 */
    -272, -154, -2, 124, 264, 386, 537, 614, 775, 913,
    1046, 1153, 1336, 1465, 1601, 1720, 1824, 1906, 2025, 2095,
    2116, 2225, 2241, 2303, 2349, 2362, 2386, 2449, 2434, 2408,
    2402, 2400, 2387, 2395, 2386, 2393, 2373, 2392, 2396, 2402,
    /* 1940.0 thru 1979.0 */
     2433, 2483, 2530, 2570, 2624, 2677, 2728, 2778, 2825, 2871,
     2915, 2957, 2997, 3036, 3072, 3107, 3135, 3168, 3218, 3268,
     3315, 3359, 3400, 3447, 3503, 3573, 3654, 3743, 3829, 3920,
     4018, 4117, 4223, 4337, 4449, 4548, 4646, 4752, 4853, 4959,
    /* 1980.0 thru 1995.0 */
     5054, 5138, 5217, 5296, 5379, 5434, 5487, 5532, 5582, 5630,
     5686, 5757, 5831, 5912, 5998, 6078,
    /* new USNO data (stern) */
     6163, 6230,
    /* 1999 USNO data 1998.0 thru 2000.0 (McBurnett) */
     6297, 6347, 6383, 
    /* 1999 extrapolation (McBurnett), 2001.0 thru 2006.0 */
     /* 6440, 6510, 6600, 6750, 6900, 7060 */
     6409, 6430, 6447, 6507, 6578, 6610	/* ECD */

    /* original 1997 USNO extrapolation (stern), 1998.0 thru 2004.0
     6296, 6420,
     6510, 6600, 6700, 6800, 6900   */ /* 7000, 7100, 7200, 7300, 7400, */

    /* Extrapolated values (USNO) (original Moshier) [1996.0 thru 2005.0]
     6183, 6280, 6377, 6475,
     6572, 6667, 6765, 6861, 6957
     */
};

/* calculate  DeltaT = ET - UT1 in seconds.  Describes the irregularities
 * of the Earth rotation rate in the ET time scale.
 */
double deltat(double mj)
{
	double Y;
	double p, B;
	int d[6];
	int i, iy, k;
	static double ans;
	static double lastmj = -10000;

	if (mj == lastmj) {
	    return(ans);
	}
	lastmj = mj;

	Y = 2000.0 + (mj - J2000)/365.25;

	if( Y > TABEND) {
	    /* linear interpolation from table end; stern */
	    B = Y - TABEND;
	    ans = dt[TABSIZ-1] + B * (dt[TABSIZ-1]  - dt[TABSIZ-11])/10;
	    ans *= 0.01;
	    return(ans);
	}

	if( Y < TABSTART) {
	    if( Y >= 948.0 - 15.0 ) {
		/* Stephenson and Morrison, stated domain is 948 to 1600:
		 * 25.5(centuries from 1800)^2 - 1.9159(centuries from 1955)^2
		 * Here we offset by -15 y to minimize the discontinuity,
		 * thus we use it from 933.0 to 1620.0,
		 * and from the end of the table to 2130.0.
		 * f(1620.0) = 60.955200, slope -0.079 s/y
		 * f(2004.0) = 105.649728, slope 1.02 s/y
		 * f(2048.0) = 155.176, slope 1.23 s/y
		 * f(2084.0) = 202.49, slope 1.4 s/y
		 * f(2130.0) = 272, slope .1616
	         * f(2150.0) = 305, slope .17
		 */
		B = 0.01*(Y - 2000.0);
		ans = (23.58 * B + 100.3)*B + 101.6;
	    } else {
		/* Borkowski */
	        /* f(2004.0) = 542.7435, slope 2.65 s/y */
		B = 0.01*(Y - 2000.0)  +  3.75;
		ans = 35.0 * B * B  +  40.;
	    }
	    return(ans);
	}

	/* Besselian interpolation from tabulated values.
	 * See AA page K11.
	 */

	/* value for 1620.1 is 121.96 or so, not 124.0 */

	/* Index into the table.
	 */
	p = floor(Y);
	iy = (int)(p - TABSTART);
	/* Zeroth order estimate is value at start of year
	 */
	ans = dt[iy];
	k = iy + 1;
	if( k >= TABSIZ )
	    goto done; /* No data, can't go on. */

	/* The fraction of tabulation interval
	 */
	p = Y - p;

	/* First order interpolated value
	 */
	ans += p*(dt[k] - dt[iy]);
	if( (iy-1 < 0) || (iy+2 >= TABSIZ) )
	    goto done; /* can't do second differences */

	/* Make table of first differences
	 */
	k = iy - 2;
	for( i=0; i<5; i++ ) {
	    if( (k < 0) || (k+1 >= TABSIZ) )
		d[i] = 0;
	    else d[i] = dt[k+1] - dt[k];
		k += 1;
	}

	/* Compute second differences
	 */
	for( i=0; i<4; i++ )
	    d[i] = d[i+1] - d[i];
	B = 0.25*p*(p-1.0);
	ans += B*(d[1] + d[2]);
	if( iy+2 >= TABSIZ )
	    goto done;

	/* Compute third differences
	 */
	for( i=0; i<3; i++ )
	    d[i] = d[i+1] - d[i];
	B = 2.0*B/3.0;
	ans += (p-0.5)*B*d[1];
	if( (iy-2 < 0) || (iy+3 > TABSIZ) )
	    goto done;

	/* Compute fourth differences
	 */
	for( i=0; i<2; i++ )
	    d[i] = d[i+1] - d[i];
	B = 0.125*B*(p+1.0)*(p-2.0);
	ans += B*(d[0] + d[1]);

	done:
	/* Astronomical Almanac table is corrected by adding the expression
	 *     -0.000091 (ndot + 26)(year-1955)^2  seconds
	 * to entries prior to 1955 (AA page K8), where ndot is the secular
	 * tidal term in the mean motion of the Moon.
	 *
	 * Entries after 1955 are referred to atomic time standards and
	 * are not affected by errors in Lunar or planetary theory.
	 */
	ans *= 0.01;
	if( Y < 1955.0 ) {
	    B = (Y - 1955.0);
	    ans += -0.000091 * (-25.8 + 26.0) * B * B;
	}
	return( ans );
}


#ifdef TEST_DT
main()
{
	double ans, y;

	while (scanf("%lf", &y) == 1) {
		ans = deltat((y - 2000.0)*365.25 + J2000);
		printf("%.4lf %.4lf\n", y, ans);
	}
}
#endif

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: deltat.c,v $ $Date: 2006/02/25 03:24:09 $ $Revision: 1.9 $ $Name:  $"};
