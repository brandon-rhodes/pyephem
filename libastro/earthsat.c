/* this file contains routines to support earth satellites.
 *
 * everything is based on the `orbit' program, as per the copyright therein as
 * follows:
 *   Copyright (c) 1986,1987,1988,1989,1990 Robert W. Berger N3EMO
 *   May be freely distributed, provided this notice remains intact. 
 * Any errors in utilizing the code from orbit are strictly my own.
 *
 * N.B. in spite of the orbit.doc comment, we assume that Range, RangeRate and
 *   Height are in km, not m.
 * define the following to get some intermediate results printed:
 * #define ESAT_TRACE
 */

#include <stdio.h>
#include <math.h>
#if defined(__STDC__)
#include <stdlib.h>
#endif

#include "P_.h"
#include "astro.h"
#include "circum.h"
#include "preferences.h"

typedef double MAT3x3[3][3];

#define ESAT_MAG        2       /* default satellite magnitude */

static void GetSatelliteParams P_((Obj *op));
static void GetSiteParams P_((Now *np));
static double Kepler P_((double MeanAnomaly, double Eccentricity));
static int esat_main P_((double CrntTime, Obj *op));
static void GetSubSatPoint P_((double SatX, double SatY, double SatZ,
    double T, double *Latitude, double *Longitude, double *Height));
static void GetPrecession P_((double SemiMajorAxis, double Eccentricity,
    double Inclination, double *RAANPrecession, double *PerigeePrecession));
static void GetSatPosition P_((double EpochTime, double EpochRAAN,
    double EpochArgPerigee, double SemiMajorAxis, double Inclination,
    double Eccentricity, double RAANPrecession, double PerigeePrecession,
    double T, double TrueAnomaly, double *X, double *Y, double *Z,
    double *Radius, double *VX, double *VY, double *VZ));
static void GetSitPosition P_((double SiteLat, double SiteLong,
    double SiteElevation, double CrntTime, double *SiteX, double *SiteY,
    double *SiteZ, double *SiteVX, double *SiteVY, MAT3x3 SiteMatrix));
static void GetRange P_((double SiteX, double SiteY, double SiteZ,
    double SiteVX, double SiteVY, double SatX, double SatY, double SatZ,
    double SatVX, double SatVY, double SatVZ, double *Range,
    double *RangeRate));
static void GetTopocentric P_((double SatX, double SatY, double SatZ,
    double SiteX, double SiteY, double SiteZ, MAT3x3 SiteMatrix, double *X,
    double *Y, double *Z));
static void GetBearings P_((double SatX, double SatY, double SatZ,
    double SiteX, double SiteY, double SiteZ, MAT3x3 SiteMatrix,
    double *Azimuth, double *Elevation));
static int Eclipsed P_((double SatX, double SatY, double SatZ,
    double SatRadius, double CrntTime));
static void InitOrbitRoutines P_((double EpochDay, int AtEod));



/* given a Now and an Obj with info about an earth satellite in the es_* fields
 * fill in the s_* sky fields describing the satellite.
 * as usual, we compute the geocentric ra/dec precessed to np->n_epoch and
 * compute topocentric altitude accounting for refraction.
 * return 0 if all ok, else -1.
 */
int
obj_earthsat (np, op)
Now *np;
Obj *op;
{
	double CrntTime;
	double ra, dec, alt;

	/* extract the xephem data forms into those used by orbit.
	 * (we reuse orbit's function names that read these from files.)
	 */
	GetSatelliteParams(op);
	GetSiteParams(np);
 
	/* xephem uses noon 12/31/1899 as 0; orbit uses midnight 1/1/1900.
	 * thus, xephem runs 12 hours, or 1/2 day, behind of what orbit wants.
	 */
	CrntTime = mjd + 0.5;

	/* do the work for the epoch of date */
	if (esat_main (CrntTime, op) < 0)
	    return (-1);

#ifdef ESAT_TRACE
	printf ("lat = %g\n", raddeg(lat));
	printf ("lng = %g\n", raddeg(lng));
	printf ("n_elev = %g\n", elev*ERAD/1000);

	printf ("az = %g\n", raddeg(op->s_az));
	printf ("alt = %g\n", raddeg(op->s_alt));
	printf ("range = %g\n", op->s_range);
	printf ("s_elev = %g\n", op->s_elev);
	printf ("sublat = %g\n", raddeg(op->s_sublat));
	printf ("sublng = %g\n", -raddeg(op->s_sublng));
	printf ("eclipsed = %d\n", op->s_eclipsed);
	fflush (stdout);
#endif

	/* correct altitude for refraction */
	alt = op->s_alt;
	refract (pressure, temp, alt, &alt);
	op->s_alt = alt;

	/* find s_ra/dec, depending on current options. */
	if (pref_get(PREF_EQUATORIAL) == PREF_TOPO) {
	    double ha, lst;
	    aa_hadec (lat, alt, (double)op->s_az, &ha, &dec);
	    now_lst (np, &lst);
	    ra = hrrad(lst) - ha;
	    range (&ra, 2*PI);
	} else {
	    ra = op->s_gaera;
	    dec = op->s_gaedec;
	}
	if (epoch != EOD)
	    precess (mjd, epoch, &ra, &dec);
	op->s_ra = ra;
	op->s_dec = dec;

	/* just make up a size and brightness */
	set_smag (op, ESAT_MAG);
	op->s_size = 0;

	return (0);
}


/* follows is right out of `orbit', sans the printing stuff, except as
 * noted by ECD.
 */
 
#ifndef PI
#define PI 3.14159265
#endif

#ifdef PI2
#undef PI2
#endif

#define PI2 (PI*2)

#define MinutesPerDay (24*60.0)
#define SecondsPerDay (60*MinutesPerDay)
#define HalfSecond (0.5/SecondsPerDay)
#define EarthRadius 6378.16             /* Kilometers           */
#define C 2.997925e5                    /* Kilometers/Second    */
#define RadiansPerDegree (PI/180)
#define ABS(x) ((x) < 0 ? (-(x)) : (x))
#define SQR(x) ((x)*(x))
 
#define EarthFlat (1/298.25)            /* Earth Flattening Coeff. */
#define SiderealSolar 1.0027379093
#define SidRate (PI2*SiderealSolar/SecondsPerDay)	/* radians/second */
#define GM 398600			/* Kilometers^3/seconds^2 */
 
#define Epsilon (RadiansPerDegree/3600)     /* 1 arc second */
#define SunRadius 695000		
#define SunSemiMajorAxis  149598845.0  	    /* Kilometers 		   */

/*
char VersionStr[] = "N3EMO Orbit Simulator  v3.9";
*/
 
/*  Keplerian Elements and misc. data for the satellite              */
static double  EpochDay;                   /* time of epoch                 */
static double EpochMeanAnomaly;            /* Mean Anomaly at epoch         */
static long EpochOrbitNum;                 /* Integer orbit # of epoch      */
static double EpochRAAN;                   /* RAAN at epoch                 */
static double epochMeanMotion;             /* Revolutions/day               */
static double OrbitalDecay;                /* Revolutions/day^2             */
static double EpochArgPerigee;             /* argument of perigee at epoch  */
static double Eccentricity;
static double Inclination;
 
/* Site Parameters */
static double SiteLat,SiteLong,SiteAltitude;


static double SidDay,SidReference;	/* Date and sidereal time	*/

/* Keplerian elements for the sun */
static double SunEpochTime,SunInclination,SunRAAN,SunEccentricity,
       SunArgPerigee,SunMeanAnomaly,SunMeanMotion;

/* values for shadow geometry */
static double SinPenumbra,CosPenumbra;
 

/* grab the xephem stuff from op and copy into orbit's globals.
 */
static void
GetSatelliteParams(op)
Obj *op;
{
	/* xephem uses noon 12/31/1899 as 0; orbit uses midnight 1/1/1900 as 1.
	 * thus, xephem runs 12 hours, or 1/2 day, behind of what orbit wants.
	 */
	EpochDay = op->es_epoch + 0.5;

	/* xephem stores inc in degrees; orbit wants rads */
	Inclination = degrad(op->es_inc);

	/* xephem stores RAAN in degrees; orbit wants rads */
	EpochRAAN = degrad(op->es_raan);

	Eccentricity = op->es_e;

	/* xephem stores arg of perigee in degrees; orbit wants rads */
	EpochArgPerigee = degrad(op->es_ap);

	/* xephem stores mean anomaly in degrees; orbit wants rads */
	EpochMeanAnomaly = degrad (op->es_M);

	epochMeanMotion = op->es_n;

	OrbitalDecay = op->es_decay;

	EpochOrbitNum = op->es_orbit;

#ifdef ESAT_TRACE
	printf ("\n");
	printf ("EpochDay = %.15g\n", EpochDay);
	printf ("Inclination = %.15g\n", Inclination);
	printf ("EpochRAAN = %.15g\n", EpochRAAN);
	printf ("Eccentricity = %.15g\n", Eccentricity);
	printf ("EpochArgPerigee = %.15g\n", EpochArgPerigee);
	printf ("EpochMeanAnomaly = %.15g\n", EpochMeanAnomaly);
	printf ("epochMeanMotion = %.15g\n", epochMeanMotion);
	printf ("OrbitalDecay = %.15g\n", OrbitalDecay);
	printf ("EpochOrbitNum = %ld\n", EpochOrbitNum);
	fflush (stdout);
#endif
}

 
static void
GetSiteParams(np)
Now *np;
{
	SiteLat = lat;
     
	/* xephem stores longitude as >0 east; orbit wants >0 west */
	SiteLong = 2.0*PI - lng;
     
	/* what orbit calls altitude xephem calls elevation and stores it from
	 * sea level in earth radii; orbit wants km
	 */
	SiteAltitude = elev*ERAD/1000.0;
     
	/* we don't implement a minimum horizon altitude cutoff
	SiteMinElev = 0;
	 */

#ifdef ESAT_TRACE
	printf ("SiteLat = %g\n", SiteLat);
	printf ("SiteLong = %g\n", SiteLong);
	printf ("SiteAltitude = %g\n", SiteAltitude);
	fflush (stdout);
#endif
}
 
/* compute geocentric gaera/dec @ eod, and topocentric alt/az _sans refraction_.
 * as well as satellites-specific fields.
 * return 0 if ok, else -1.
 */
static int
esat_main (CrntTime, op)
double CrntTime;
Obj *op;
{
	double ReferenceOrbit;      /* Floating point orbit # at epoch */
	double CurrentOrbit;
	double AverageMotion,       /* Corrected for drag              */
	    CurrentMotion;
	double MeanAnomaly,TrueAnomaly;
	double SemiMajorAxis;
	double Radius;              /* From geocenter                  */
	double SatX,SatY,SatZ;	    /* In Right Ascension based system */
	double SatVX,SatVY,SatVZ;   /* Kilometers/second	       */
	double SiteX,SiteY,SiteZ;
	double SiteVX,SiteVY;
	double SiteMatrix[3][3];
	double Height;
	double RAANPrecession,PerigeePrecession;
	double SSPLat,SSPLong;
	long OrbitNum;
	double Azimuth,Elevation,Range;
	double RangeRate;
	double dtmp;

	InitOrbitRoutines(CrntTime, 1);

	SemiMajorAxis = 331.25 * exp(2*log(MinutesPerDay/epochMeanMotion)/3);
	GetPrecession(SemiMajorAxis,Eccentricity,Inclination,&RAANPrecession,
			    &PerigeePrecession);

	ReferenceOrbit = EpochMeanAnomaly/PI2 + EpochOrbitNum;
     
	AverageMotion = epochMeanMotion + (CrntTime-EpochDay)*OrbitalDecay/2;
	CurrentMotion = epochMeanMotion + (CrntTime-EpochDay)*OrbitalDecay;

	/* ECD: range checking */
	if (CurrentMotion <= 0)
	    return (-1);

	SemiMajorAxis = 331.25 * exp(2*log(MinutesPerDay/CurrentMotion)/3);
     
	CurrentOrbit = ReferenceOrbit + (CrntTime-EpochDay)*AverageMotion;

#ifdef ESAT_TRACE
	printf ("CurrentOrbit=%.10g ReferenceOrbit=%.10g CrntTime=%.10g EpochDay=%.10g AverageMotion=%.10g\n",
	CurrentOrbit, ReferenceOrbit, CrntTime, EpochDay, AverageMotion);
#endif

	OrbitNum = CurrentOrbit;
     
	MeanAnomaly = (CurrentOrbit-OrbitNum)*PI2;
     
	TrueAnomaly = Kepler(MeanAnomaly,Eccentricity);

	GetSatPosition(EpochDay,EpochRAAN,EpochArgPerigee,SemiMajorAxis,
		Inclination,Eccentricity,RAANPrecession,PerigeePrecession,
		CrntTime,TrueAnomaly,&SatX,&SatY,&SatZ,&Radius,
		&SatVX,&SatVY,&SatVZ);

	/* ECD: insure 0..2PI ra */
	dtmp = atan2 (SatY, SatX);
	range (&dtmp, 2*PI);
	op->s_gaera = dtmp;
	op->s_gaedec = atan2 (SatZ, sqrt(SatX*SatX + SatY*SatY));

	GetSitPosition(SiteLat,SiteLong,SiteAltitude,CrntTime,
		    &SiteX,&SiteY,&SiteZ,&SiteVX,&SiteVY,SiteMatrix);


	GetBearings(SatX,SatY,SatZ,SiteX,SiteY,SiteZ,SiteMatrix,
		    &Azimuth,&Elevation);

	op->s_az = Azimuth;
	op->s_alt = Elevation;

	/* Range: line-of-site distance to satellite, m
	 * RangeRate: m/s
	 */
	GetRange(SiteX,SiteY,SiteZ,SiteVX,SiteVY,
	    SatX,SatY,SatZ,SatVX,SatVY,SatVZ,&Range,&RangeRate);

	op->s_range = Range*1000;	/* we want m */
	op->s_rangev = RangeRate*1000;	/* we want m/s */
 
	/* SSPLat: sub-satellite latitude, rads 
	 * SSPLong: sub-satellite longitude, >0 west, rads 
	 * Height: height of satellite above ground, m
	 */
	GetSubSatPoint(SatX,SatY,SatZ,CrntTime,
	    &SSPLat,&SSPLong,&Height);

	op->s_elev = Height*1000;	/* we want m */
	op->s_sublat = SSPLat;
	op->s_sublng = -SSPLong;	/* we want +E */

	op->s_eclipsed = Eclipsed(SatX,SatY,SatZ,Radius,CrntTime);

#ifdef ESAT_TRACE
	printf ("\n");
	printf ("CrntTime = %g\n", CrntTime);
	printf ("ReferenceOrbit = %g\n", ReferenceOrbit);
	printf ("CurrentOrbit = %g\n", CurrentOrbit);
	printf ("AverageMotion = %g\n", AverageMotion);
	printf ("CurrentMotion = %g\n", CurrentMotion);
	printf ("MeanAnomaly = %g\n", MeanAnomaly);
	printf ("TrueAnomaly = %g\n", TrueAnomaly);
	printf ("SemiMajorAxis = %g\n", SemiMajorAxis);
	printf ("Radius = %g\n", Radius);
	printf ("SatX = %g\n", SatX);
	printf ("SatY = %g\n", SatY);
	printf ("SatZ = %g\n", SatZ);
	printf ("SatVX = %g\n", SatVX);
	printf ("SatVY = %g\n", SatVY);
	printf ("SatVZ = %g\n", SatVZ);
	printf ("SiteX = %g\n", SiteX);
	printf ("SiteY = %g\n", SiteY);
	printf ("SiteZ = %g\n", SiteZ);
	printf ("SiteVX = %g\n", SiteVX);
	printf ("SiteVY = %g\n", SiteVY);
	printf ("Height = %g\n", Height);
	printf ("RAANPrecession = %g\n", RAANPrecession);
	printf ("PerigeePrecession = %g\n", PerigeePrecession);
	printf ("SSPLat = %g\n", SSPLat);
	printf ("SSPLong = %g\n", SSPLong);
	printf ("Azimuth = %g\n", Azimuth);
	printf ("Elevation = %g\n", Elevation);
	printf ("Range = %g\n", Range);
	printf ("RangeRate = %g\n", RangeRate);
	fflush (stdout);
#endif

	return (0);
}


 
/* Solve Kepler's equation                                      */
/* Inputs:                                                      */
/*      MeanAnomaly     Time Since last perigee, in radians.    */
/*                      PI2 = one complete orbit.               */
/*      Eccentricity    Eccentricity of orbit's ellipse.        */
/* Output:                                                      */
/*      TrueAnomaly     Angle between perigee, geocenter, and   */
/*                      current position.                       */
 
static
double Kepler(MeanAnomaly,Eccentricity)
register double MeanAnomaly,Eccentricity;
 
{
register double E;              /* Eccentric Anomaly                    */
register double Error;
register double TrueAnomaly;
 
    E = MeanAnomaly ;/*+ Eccentricity*sin(MeanAnomaly);  -- Initial guess */
    do
        {
        Error = (E - Eccentricity*sin(E) - MeanAnomaly)
                / (1 - Eccentricity*cos(E));
        E -= Error;
        }
   while (ABS(Error) >= Epsilon);

    if (ABS(E-PI) < Epsilon)
        TrueAnomaly = PI;
      else
        TrueAnomaly = 2*atan(sqrt((1+Eccentricity)/(1-Eccentricity))
                                *tan(E/2));
    if (TrueAnomaly < 0)
        TrueAnomaly += PI2;
 
    return TrueAnomaly;
}
 
static void
GetSubSatPoint(SatX,SatY,SatZ,T,Latitude,Longitude,Height)
double SatX,SatY,SatZ,T;
double *Latitude,*Longitude,*Height;
{
    double r;
    /* ECD: long i; */

    r = sqrt(SQR(SatX) + SQR(SatY) + SQR(SatZ));

    *Longitude = PI2*((T-SidDay)*SiderealSolar + SidReference)
		    - atan2(SatY,SatX);

    /* ECD:
     * want Longitude in range -PI to PI , +W
     */
    range (Longitude, 2*PI);
    if (*Longitude > PI)
	*Longitude -= 2*PI;

    *Latitude = atan(SatZ/sqrt(SQR(SatX) + SQR(SatY)));

#if SSPELLIPSE
#else
    *Height = r - EarthRadius;
#endif
}
 
 
static void
GetPrecession(SemiMajorAxis,Eccentricity,Inclination,
        RAANPrecession,PerigeePrecession)
double SemiMajorAxis,Eccentricity,Inclination;
double *RAANPrecession,*PerigeePrecession;
{
  *RAANPrecession = 9.95*pow(EarthRadius/SemiMajorAxis,3.5) * cos(Inclination)
                 / SQR(1-SQR(Eccentricity)) * RadiansPerDegree;
 
  *PerigeePrecession = 4.97*pow(EarthRadius/SemiMajorAxis,3.5)
         * (5*SQR(cos(Inclination))-1)
                 / SQR(1-SQR(Eccentricity)) * RadiansPerDegree;
}
 
/* Compute the satellite postion and velocity in the RA based coordinate
 * system.
 * ECD: take care not to let Radius get below EarthRadius.
 */

static void
GetSatPosition(EpochTime,EpochRAAN,EpochArgPerigee,SemiMajorAxis,
	Inclination,Eccentricity,RAANPrecession,PerigeePrecession,
	T,TrueAnomaly,X,Y,Z,Radius,VX,VY,VZ)
 
double EpochTime,EpochRAAN,EpochArgPerigee;
double SemiMajorAxis,Inclination,Eccentricity;
double RAANPrecession,PerigeePrecession,T, TrueAnomaly;
double *X,*Y,*Z,*Radius,*VX,*VY,*VZ;

{
    double RAAN,ArgPerigee;
 

    double Xw,Yw,VXw,VYw;	/* In orbital plane */
    double Tmp;
    double Px,Qx,Py,Qy,Pz,Qz;	/* Escobal transformation 31 */
    double CosArgPerigee,SinArgPerigee;
    double CosRAAN,SinRAAN,CoSinclination,SinInclination;

    *Radius = SemiMajorAxis*(1-SQR(Eccentricity))
                        / (1+Eccentricity*cos(TrueAnomaly));

    if (*Radius <= EarthRadius)
	*Radius = EarthRadius;


    Xw = *Radius * cos(TrueAnomaly);
    Yw = *Radius * sin(TrueAnomaly);
    
    Tmp = sqrt(GM/(SemiMajorAxis*(1-SQR(Eccentricity))));

    VXw = -Tmp*sin(TrueAnomaly);
    VYw = Tmp*(cos(TrueAnomaly) + Eccentricity);

    ArgPerigee = EpochArgPerigee + (T-EpochTime)*PerigeePrecession;
    RAAN = EpochRAAN - (T-EpochTime)*RAANPrecession;

    CosRAAN = cos(RAAN); SinRAAN = sin(RAAN);
    CosArgPerigee = cos(ArgPerigee); SinArgPerigee = sin(ArgPerigee);
    CoSinclination = cos(Inclination); SinInclination = sin(Inclination);
    
    Px = CosArgPerigee*CosRAAN - SinArgPerigee*SinRAAN*CoSinclination;
    Py = CosArgPerigee*SinRAAN + SinArgPerigee*CosRAAN*CoSinclination;
    Pz = SinArgPerigee*SinInclination;
    Qx = -SinArgPerigee*CosRAAN - CosArgPerigee*SinRAAN*CoSinclination;
    Qy = -SinArgPerigee*SinRAAN + CosArgPerigee*CosRAAN*CoSinclination;
    Qz = CosArgPerigee*SinInclination;

    *X = Px*Xw + Qx*Yw;		/* Escobal, transformation #31 */
    *Y = Py*Xw + Qy*Yw;
    *Z = Pz*Xw + Qz*Yw;

    *VX = Px*VXw + Qx*VYw;
    *VY = Py*VXw + Qy*VYw;
    *VZ = Pz*VXw + Qz*VYw;
}

/* Compute the site postion and velocity in the RA based coordinate
   system. SiteMatrix is set to a matrix which is used by GetTopoCentric
   to convert geocentric coordinates to topocentric (observer-centered)
    coordinates. */

static void
GetSitPosition(SiteLat,SiteLong,SiteElevation,CrntTime,
             SiteX,SiteY,SiteZ,SiteVX,SiteVY,SiteMatrix)

double SiteLat,SiteLong,SiteElevation,CrntTime;
double *SiteX,*SiteY,*SiteZ,*SiteVX,*SiteVY;
MAT3x3 SiteMatrix;

{
    static double G1,G2; /* Used to correct for flattening of the Earth */
    static double CosLat,SinLat;
    static double OldSiteLat = -100000;  /* Used to avoid unneccesary recomputation */
    static double OldSiteElevation = -100000;
    double Lat;
    double SiteRA;	/* Right Ascension of site			*/
    double CosRA,SinRA;

    if ((SiteLat != OldSiteLat) || (SiteElevation != OldSiteElevation))
	{
	OldSiteLat = SiteLat;
	OldSiteElevation = SiteElevation;
	Lat = atan(1/(1-SQR(EarthFlat))*tan(SiteLat));

	CosLat = cos(Lat);
	SinLat = sin(Lat);

	G1 = EarthRadius/(sqrt(1-(2*EarthFlat-SQR(EarthFlat))*SQR(SinLat)));
	G2 = G1*SQR(1-EarthFlat);
	G1 += SiteElevation;
	G2 += SiteElevation;
	}


    SiteRA = PI2*((CrntTime-SidDay)*SiderealSolar + SidReference)
	         - SiteLong;
    CosRA = cos(SiteRA);
    SinRA = sin(SiteRA);
    

    *SiteX = G1*CosLat*CosRA;
    *SiteY = G1*CosLat*SinRA;
    *SiteZ = G2*SinLat;
    *SiteVX = -SidRate * *SiteY;
    *SiteVY = SidRate * *SiteX;

    SiteMatrix[0][0] = SinLat*CosRA;
    SiteMatrix[0][1] = SinLat*SinRA;
    SiteMatrix[0][2] = -CosLat;
    SiteMatrix[1][0] = -SinRA;
    SiteMatrix[1][1] = CosRA;
    SiteMatrix[1][2] = 0.0;
    SiteMatrix[2][0] = CosRA*CosLat;
    SiteMatrix[2][1] = SinRA*CosLat;
    SiteMatrix[2][2] = SinLat;
}

static void
GetRange(SiteX,SiteY,SiteZ,SiteVX,SiteVY,
	SatX,SatY,SatZ,SatVX,SatVY,SatVZ,Range,RangeRate)

double SiteX,SiteY,SiteZ,SiteVX,SiteVY;
double SatX,SatY,SatZ,SatVX,SatVY,SatVZ;
double *Range,*RangeRate;
{
    double DX,DY,DZ;

    DX = SatX - SiteX; DY = SatY - SiteY; DZ = SatZ - SiteZ;

    *Range = sqrt(SQR(DX)+SQR(DY)+SQR(DZ));    

    *RangeRate = ((SatVX-SiteVX)*DX + (SatVY-SiteVY)*DY + SatVZ*DZ)
			/ *Range;
}

/* Convert from geocentric RA based coordinates to topocentric
   (observer centered) coordinates */

static void
GetTopocentric(SatX,SatY,SatZ,SiteX,SiteY,SiteZ,SiteMatrix,X,Y,Z)
double SatX,SatY,SatZ,SiteX,SiteY,SiteZ;
double *X,*Y,*Z;
MAT3x3 SiteMatrix;
{
    SatX -= SiteX;
    SatY -= SiteY;
    SatZ -= SiteZ;

    *X = SiteMatrix[0][0]*SatX + SiteMatrix[0][1]*SatY
	+ SiteMatrix[0][2]*SatZ; 
    *Y = SiteMatrix[1][0]*SatX + SiteMatrix[1][1]*SatY
	+ SiteMatrix[1][2]*SatZ; 
    *Z = SiteMatrix[2][0]*SatX + SiteMatrix[2][1]*SatY
	+ SiteMatrix[2][2]*SatZ; 
}

static void
GetBearings(SatX,SatY,SatZ,SiteX,SiteY,SiteZ,SiteMatrix,Azimuth,Elevation)
double SatX,SatY,SatZ,SiteX,SiteY,SiteZ;
MAT3x3 SiteMatrix;
double *Azimuth,*Elevation;
{
    double x,y,z;

    GetTopocentric(SatX,SatY,SatZ,SiteX,SiteY,SiteZ,SiteMatrix,&x,&y,&z);

    *Elevation = atan(z/sqrt(SQR(x) + SQR(y)));

    *Azimuth = PI - atan2(y,x);

    if (*Azimuth < 0)
	*Azimuth += PI;
}

static int
Eclipsed(SatX,SatY,SatZ,SatRadius,CrntTime)
double SatX,SatY,SatZ,SatRadius,CrntTime;
{
    double MeanAnomaly,TrueAnomaly;
    double SunX,SunY,SunZ,SunRad;
    double vx,vy,vz;
    double CosTheta;

    MeanAnomaly = SunMeanAnomaly+ (CrntTime-SunEpochTime)*SunMeanMotion*PI2;
    TrueAnomaly = Kepler(MeanAnomaly,SunEccentricity);

    GetSatPosition(SunEpochTime,SunRAAN,SunArgPerigee,SunSemiMajorAxis,
		SunInclination,SunEccentricity,0.0,0.0,CrntTime,
		TrueAnomaly,&SunX,&SunY,&SunZ,&SunRad,&vx,&vy,&vz);

    CosTheta = (SunX*SatX + SunY*SatY + SunZ*SatZ)/(SunRad*SatRadius)
		 *CosPenumbra + (SatRadius/EarthRadius)*SinPenumbra;

    if (CosTheta < 0)
        if (CosTheta < -sqrt(SQR(SatRadius)-SQR(EarthRadius))/SatRadius
	    		*CosPenumbra + (SatRadius/EarthRadius)*SinPenumbra)
	  
	    return 1;
    return 0;
}

/* Initialize the Sun's keplerian elements for a given epoch.
   Formulas are from "Explanatory Supplement to the Astronomical Ephemeris".
   Also init the sidereal reference				*/

static void
InitOrbitRoutines(EpochDay, AtEod)
double EpochDay;
int AtEod;
{
    double T,T2,T3,Omega;
    int n;
    double SunTrueAnomaly,SunDistance;

    T = (floor(EpochDay)-0.5)/36525;
    T2 = T*T;
    T3 = T2*T;

    SidDay = floor(EpochDay);

    SidReference = (6.6460656 + 2400.051262*T + 0.00002581*T2)/24;
    SidReference -= floor(SidReference);

    /* Omega is used to correct for the nutation and the abberation */
    Omega = AtEod ? (259.18 - 1934.142*T) * RadiansPerDegree : 0.0;
    n = Omega / PI2;
    Omega -= n*PI2;

    SunEpochTime = EpochDay;
    SunRAAN = 0;

    SunInclination = (23.452294 - 0.0130125*T - 0.00000164*T2
		    + 0.000000503*T3 +0.00256*cos(Omega)) * RadiansPerDegree;
    SunEccentricity = (0.01675104 - 0.00004180*T - 0.000000126*T2);
    SunArgPerigee = (281.220833 + 1.719175*T + 0.0004527*T2
			+ 0.0000033*T3) * RadiansPerDegree;
    SunMeanAnomaly = (358.475845 + 35999.04975*T - 0.00015*T2
			- 0.00000333333*T3) * RadiansPerDegree;
    n = SunMeanAnomaly / PI2;
    SunMeanAnomaly -= n*PI2;

    SunMeanMotion = 1/(365.24219879 - 0.00000614*T);

    SunTrueAnomaly = Kepler(SunMeanAnomaly,SunEccentricity);
    SunDistance = SunSemiMajorAxis*(1-SQR(SunEccentricity))
			/ (1+SunEccentricity*cos(SunTrueAnomaly));

    SinPenumbra = (SunRadius-EarthRadius)/SunDistance;
    CosPenumbra = sqrt(1-SQR(SinPenumbra));
}
