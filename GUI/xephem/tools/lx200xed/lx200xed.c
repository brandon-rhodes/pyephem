/*
 * liblx200 v0.8
 * A library for controlling the Meade LX200 series scopes
 * 
 * Copyright (C) 2000, Mike Stute
*/
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/*Stuff needed by libastro*/
#include "astro.h"

#include "liblx200.h"

#define _POSIX_SOURCE 1

#define GOTOFIFO_NAME   "/usr/local/xephem/fifos/xephem_loc_fifo"
#define MARKFIFO_NAME	"/usr/local/xephem/fifos/xephem_in_fifo"

#define	TEMPERATURE	10.		/* air temp, C */
#define	PRESSURE	1010.		/* air pressure, mB */
#define	SUNDIP		degrad(18.)	/* dusk, rads below horizon */
#define	ELEVATION	0.		/* elevation, m */
#define	MINMOV		60		/* secs between moving objs updates */
#define	UPDDEF		500		/* default graphical update period, ms*/

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define tracing  config.iTrace

typedef int BOOLEAN;

/***** Predefines ****/
void usage(void);
void parse_opts(int, char **);
void die(char *, char *);
void initNow (int fd);
void replace(char, char, char *);
void telescope_goto(int, int, char);
int send_goto(int fd);
int open_fifo(char *, int, BOOLEAN);
void time_fromsys(Now *);
int tz_fromsys(Now *);
void trace(int, char *, char *);

/******* Globals *******/

static Obj op;               /* Object for libastro, here due to laziness*/
static Now now;		     /* Now for libastro, here due to same laziness */
static BOOLEAN moving;       /* TRUE if current object is not fixed*/
static char szMsg[101];      /* For trace*/
static int nohw;	     /* set by -e */
static int updper = UPDDEF;  /* update period, ms */

static struct {
   BOOLEAN fGoto;            /*If TRUE then gotos okay, else don't do them*/
   BOOLEAN fMarker;          /*If TRUE then marker enabled*/
   BOOLEAN fMoving;          /*If TRUE then following moving objects*/
   double dTemp;             /*Temperature to use in C*/
   double dDip;              /*Sun dip (degress above horizon) in radians */
   double dPressure;         /*Pressure in mBars*/
   double dElev;	     /*Elevation in meters*/
   int iTrace;               /*Trace level*/
   char szFifo_goto_name[256];/*FIFO for telescope control (from xephem) */
   char szFifo_mark_name[256];/*FIFO fir telescope marker (to xephem) */
   char szTelDev[256];       /*Telescope device name*/
   int minimum_move_time;    /*How often goto is resent for moving object, sec*/
} config;

/*
 * minimum_move_time determines how often to recenter the scope
 * for a moving object. I have picked once a minute, but
 * once every 3 or 5 is probably good too. The program pulses 3 times
 * a second roughly and there's a little overhead due to scope 
 * communications for the marker, so I picked 57 seconds
 * for the default
 */

int
main(int argc, char **argv)
{
   int fd,fd_marker_fifo,fd_goto_fifo;   /*File descriptors*/
   unsigned char buf[101];
   double ra,tmp;
   int ra_hour, ra_min, ra_sec;
   double dec;
   int dec_deg, dec_min, dec_sec;
   char szIn[5];
   int iBytes;
   long last_send_time = 0;
   double ep;

   /*Setup default config*/
   strncpy(config.szFifo_goto_name, GOTOFIFO_NAME,255);
   strncpy(config.szFifo_mark_name, MARKFIFO_NAME,255);
   strncpy(config.szTelDev,TELESCOPE,255);
   config.dTemp=TEMPERATURE;
   config.dDip=SUNDIP;
   config.dPressure=PRESSURE;
   config.dElev=ELEVATION;
   config.fMarker=TRUE;
   config.fGoto=TRUE;
   config.fMoving=TRUE;
   config.minimum_move_time = MINMOV;
   
   parse_opts(argc, argv);

   /**************** Open out fifo *******************/
   /*      This is used for the telescope marker     */
   fd_marker_fifo = open_fifo(config.szFifo_mark_name,O_RDWR,TRUE);
   
   /**************** Open in fifo ********************/
   /*    This is used to read for telescope gotos    */
   fd_goto_fifo = open_fifo(config.szFifo_goto_name,O_RDWR,FALSE);      
   
   /*************** Open the scope *******************/
   /*          This is, well, the telescope          */
   /**************************************************/
   fd = lx200_open_scope(config.szTelDev);
   if(fd==-1)
     die(config.szTelDev, strerror(errno));

   /*Set scope for long format*/
   
   if(lx200_set_format(fd,LX200_OPT_LONG_FORMAT)==LX200_FALSE)
     die("The scope is not responding", NULL);

   /* set up global "now" for EOD */
   initNow(fd);
   
   while(1) {     
      /* Check for a goto */
      
      if(config.fGoto) {
	 iBytes=0;
	 iBytes = read(fd_goto_fifo,szIn,1);
	 if(iBytes==1) {
	    if(tracing) {
	       szMsg[0]=szIn[0];
	       szMsg[1]='\0';
	    }
	    trace(1,"Reading GOTO %s\n",szMsg);
	    telescope_goto(fd_goto_fifo,fd,szIn[0]);
	    last_send_time = time(NULL);
	 }
	 
	 if(moving && time(NULL) > last_send_time + config.minimum_move_time) {
	   send_goto(fd);   /*Recenter a moving object*/
	   last_send_time = time(NULL);
	 } else {
	    usleep(updper*1000);
	 }
      }
      else
	usleep(updper*1000);

      if(config.fMarker) {
	 trace(7,"Sending get ra\n",NULL);
	 lx200_get_ra(fd,buf);
	 lx200_convert_RA(buf,&ra_hour,&ra_min,&ra_sec); /*Parse it*/
	 if(tracing) {
	    snprintf(szMsg,100,"%s RH: %d RM: %d RS: %d\n",buf,ra_hour,ra_min,ra_sec);
	    trace(8,szMsg,NULL);
	 }
	 /*Scan and convert it to radians*/      
	 f_scansexa(buf,&tmp);
	 ra = nohw ? op.s_ra : hrrad(tmp);

	      
	 lx200_get_dec(fd,buf);	 
	 lx200_convert_Dec(buf,&dec_deg,&dec_min,&dec_sec); /*Parse it*/
	 buf[3]=':';	 
	 if(tracing) {
	    snprintf(szMsg,100,"%s %d %d %d\n",buf,dec_deg,dec_min,dec_sec);
	    trace(8," %s\n",szMsg);
	 }
	 /*Scan it and convert to radians*/
	 f_scansexa(buf,&tmp);
	 dec = nohw ? op.s_dec : degrad(tmp);

	 time_fromsys( &now);
	 mjd_year( now.n_mjd, &ep);
	 snprintf(buf,sizeof(buf),"RA:%9.6f Dec:%9.6f Epoch:%13.8f\n",ra,dec,ep);
	 trace(7,"To XEphem: %s\n", buf);
	 write(fd_marker_fifo, buf, strlen(buf));
      }
   }   
     /*A clean exit*/
   die(NULL,NULL);
}

/*
 * Standard usage
 */
void
usage(void)
{
   printf("Usage: lx200xed [options]\n");
   printf("Purpose: allow XEphem to control a Meade LX200 telescope\n");
   printf("Options:\n");
   printf(" -e        Turn on hardware emulation\n");
   printf(" -m path   alternate FIFO to XEphem for telescope marker\n");
   printf("           default %s\n", MARKFIFO_NAME);
   printf(" -g path   alternate FIFO from XEphem for telescope GOTOs\n");
   printf("           default %s\n", GOTOFIFO_NAME);
   printf(" -t path   device for telescope; default %s\n", TELESCOPE);
   printf(" -p number air pressure, mbars; default %g\n", PRESSURE);
   printf(" -c temp   air temperature, Celius; default %g\n", TEMPERATURE);
   printf(" -d dip    sun angle down at end of twilight; default %g\n",
							    raddeg(SUNDIP));
   printf(" -h elev   elevation in meters; default %g\n", ELEVATION);
   printf(" -l lat    use this latitude, not scope's, D:M:S, +N\n");
   printf(" -L lng    use this longitude, not scope's, D:M:S, +E\n");
   printf(" -N        turn off GOTO control\n");
   printf(" -M        turn off telescope marker\n");
   printf(" -A        turn off automatic follow of moving objects\n");
   printf(" -i secs   interval betweens moves for automatic follow, secs; default %d\n", MINMOV);
   printf(" -u ms     graphical update period, ms; default %d\n", UPDDEF);
   printf(" -x level  set debug level to level\n");
   printf(" -v        print version and exit\n");   
   printf(" -<other>  this schpeel\n");
   die(NULL,NULL);
}

static void
prVersion(void)
{
    char buf[128];

    lx200_get_lib_version(buf);
    printf("lx200xed $Revision: 1.18 $\n");
    printf("liblx200 %s\n", buf);
}

/*
 * Handle command line options
 */
void
parse_opts(int argc, char **argv)
{
   char cOption;
   double tmp;

   trace(6,"In parse_opts...\n",NULL);
   while (1) {
      cOption=getopt(argc,argv,"l:L:ep:d:m:g:t:NMAx:u:i:h:vc:");
      if(cOption==EOF)
	break;
      switch(cOption) {
       case 'v':
	 prVersion();
	 die(NULL,NULL);
	 break;
       case 'e':   /*set liblx200 to emulate hardware*/
	 lx200_set_lib_emulate(LX200_TRUE,LX200_EMULATE_TRUE);
	 nohw = 1;
	 break;

       case 'm':   /*Override the default marker FIFO*/
	 strncpy(config.szFifo_mark_name,optarg,255);
	 break;
       case 'l':   /* new latitude D:M:S +N */
         f_scansexa(optarg,&tmp);
         now.n_lat = degrad(tmp);
         break;
       case 'L':   /* new longitude D:M:S +W */
         f_scansexa(optarg,&tmp);
         now.n_lng = degrad(-tmp);
         break;
       case 'g':   /*Override the default goto FIFO*/
	 strncpy(config.szFifo_goto_name,optarg,255);
	 break;
       case 't':   /*Override the default telescope device*/
	 strncpy(config.szTelDev,optarg,255);
	 break;
       case 'p':   /*Set the pressure*/
	 config.dPressure=strtod(optarg,NULL);
	 break;
       case 'd':   /*Set the dip*/
	 config.dDip=strtod(optarg,NULL);
	 if(!errno)
	   config.dDip=degrad(config.dDip);
	 break;
       case 'c':   /*Set the temperature*/
	 config.dTemp=strtod(optarg,NULL);
	 break;
       case 'h':   /*Set the elevation*/
	 config.dElev=strtod(optarg,NULL);
	 break;
       case 'N':   /*No goto control*/
	 config.fGoto=FALSE;
	 break;
       case 'M':   /*No marker control*/
	 config.fMarker=FALSE;
	 break;
       case 'A':   /*No automatic following of moving objects*/
	 config.fMoving=FALSE;
	 break;
       case 'i':
	 config.minimum_move_time=atoi(optarg);
	 if(errno) {
	    printf("Invalid number of seconds for interval, setting to %d\n",
								    MINMOV);
	    config.minimum_move_time= MINMOV;
	 }
	 break;
       case 'x':   /*Debug level*/
	 config.iTrace=(int)strtol(optarg,NULL,10);
	 if(errno) {
	    printf("Invalid trace level, setting to no trace\n");
	    config.iTrace=0;
	 }
	 break;
       case 'u':
	updper = strtol (optarg, NULL, 10);
	break;
       default:
	 usage();
	 break;
      }  
   }
}

/*
 * die cleanly, possibly with a message and argument
 */
void
die(char *szMsg, char *szArg)
{
   trace(6,"In die...\n",NULL);

   /*
   if(fd!=-1) {
      lx200_close_scope(fd);  
      close(fd_goto_fifo);
      close(fd_marker_fifo);
   }*/
   
   if(szMsg!=NULL && szArg!=NULL)
     printf("%s: %s\n",szMsg, szArg);
   else
     if(szMsg!=NULL)
       printf("%s\n",szMsg);
   trace(1,"lx200xed exiting\n",NULL);
   exit(0);
}

/* init now -- all but mjd, that gets set as needed.
 * Get the latitude and longitude from scope if not already known.
 */
void
initNow (int fd)
{
   if (!now.n_lng && !now.n_lat) {
       char szLat[21],szLong[21];
       double tmp;

       lx200_get_latitude(fd,szLat);
       if(tracing) trace(4,"Lat from scope: '%s' ",szLat);
       szLat[3]  = ':';
       strcat(szLat,":00");
       if(tracing) trace(4,"becomes '%s'\n",szLat);

       lx200_get_longitude(fd,szLong);
       if(tracing) trace(4,"Lng from scope: '%s' ",szLong);
       szLong[3] = ':';
       strcat(szLong,":00");
       if(tracing) trace(4,"becomes '%s'\n",szLong);

       f_scansexa(szLong,&tmp);
       now.n_lng = degrad(-tmp);
       f_scansexa(szLat,&tmp);
       now.n_lat = degrad(tmp);
    }

   now.n_elev=config.dElev/ERAD;
   now.n_dip=config.dDip;
   now.n_epoch=J2000;
   now.n_temp=config.dTemp;
   now.n_pressure=config.dPressure;
}

/*
 * Replace one character in a string with another
 */
void
replace(char cFrom, char cTo, char *szp)
{
   char *c=szp;
   
   trace(6,"In replace...",NULL);
   while(*c++!='\0')
     if(*c==cFrom)
       *c=cTo;
}

/* 
 * Telescope GOTO
 * Reads the in fifo to get the object information
 * Calls db_crack_line from libastro
 * sets the moving flag and then calls send_goto
 * to calculate the position and move the scope
 */
void
telescope_goto(int fd_in, int fd_out, char c)
{
   int iBytes,iRead=1;
   char sz[121],szIn[2],szMsg[100];
   
   trace(6,"In telescope_goto...",NULL);
   sz[0]=c;
   trace(1,"Telescope goto\n",NULL);

   while(1) {
      trace(5,"Reading a byte\n",NULL);
      iBytes = read(fd_in,szIn,1);
      if(tracing) {
	 szMsg[0]=c;
	 szMsg[1]='\0';
	 trace(6,"Read c=%.1s\n",szIn);
      }
      if(iBytes==-1) {
	 sz[iRead]='\0';
	 trace(5,"Received: %s\n",sz);
	 break;
      }
      if(iRead<120)
        sz[iRead++]=szIn[0];
      
   }

   trace(4,"Cracking line:%s\n", sz);
   if(db_crack_line(sz,&op,NULL,0,szMsg)<0) {
      trace(4,"Couldn't crack edb info:%s\n", szMsg);
      return;
   }
   if(op.o_type ==FIXED)
     moving=FALSE;
   else
     moving=config.fMoving;
   
   if(tracing) {
      trace(4,"Cracked edb line for %s",op.o_name);
      trace(4," %s\n",moving ? "Moving" : "Fixed" );
   }
   
   send_goto(fd_out);  
}

/*
 * send_goto
 * This sends a goto by calling obj_cir
 * from libastro to get current information,
 * and then sending the goto to the scope.
 * 
 * op should contain a valid object before this call.
 */
int
send_goto(int fd)
{
   char szRa[20], szDec[20];

   trace(6,"In send_goto...",NULL);
   
   if(tracing) {
      char szEp[10];
      double tmp;
      mjd_year (op.f_epoch, &tmp);
      sprintf (szEp, "%8.3f", tmp);
      fs_sexa(szRa,radhr(op.f_RA),2,3600);  
      fs_sexa(szDec,raddeg(op.f_dec),3,3600);
      trace(5,"From XEphem RA=%s",szRa);
      trace(5," Dec=%s",szDec);
      trace(5," Ep=%s\n", szEp);
   }
   
   /*And finally calculate the emphererid for the object*/
   trace(3,"Calling obj_cir\n",NULL);
   time_fromsys(&now);
   if(obj_cir(&now,&op)<0) {  
      trace(4,"obj_cir failed\n",NULL);
      return LX200_FALSE; 
   }
   trace(3,"Formatting scope goto values\n",NULL);
   fs_sexa(szRa,radhr(op.s_ra),2,3600);  

   /* this gives always create 4 chars left of first :
    * no zero pad, no explicit +
    */
   fs_sexa(szDec,raddeg(op.s_dec),4,3600);
   
   if(tracing) {
      trace(5,"EOD RA=%s",szRa);
      trace(5," Dec=%s\n",szDec);
   }
   /*Leading zeros are required by the LX200*/
   /*RA is easy*/
   if(szRa[0]==' ')
     szRa[0]='0';
 
   /*Deal with sign, but liblx200 accepts : or 223 for degree sign*/
   /*However, this is a little harder.
    * FS requires exactly "Sd SDD:MM:SS", we build " SDD:MM:SS" portion here.
    */
   /* always leave szDec[0] a blank */
   if (szDec[2] == ' ' || szDec[2] == '-') {
      /* zero-pad degrees to 2 places */
      szDec[1] = szDec[2];
      szDec[2] = '0';
   }
   if (szDec[1] != '-') {
      /* explicit + before positive deg */
      szDec[1] = '+';
    }

   if(tracing) {
      trace(5,"Real: RA %s ,",szRa);
      trace(5," Dec: %s\n",szDec);
   }
   return(lx200_goto_RADec(fd,szRa,szDec));
}

/*
 * Open a FIFO using mode "mode"
 * If blocking is false, set the stream 
 * to block
 */
int
open_fifo(char *szName, int mode, BOOLEAN blocking)
{
   struct stat fbuf;
   int fd;

   trace(6,"In send_goto...",NULL);
   
   if (stat(szName, &fbuf) == -1) { 
      die(szName,strerror(errno));
   }
   else if (!S_ISFIFO(fbuf.st_mode))
     {
	die(szName, "not a fifo");
     }
   
   trace(4,"Open the FIFO\n",NULL);

   if (!blocking)
      mode |= O_NONBLOCK;
   fd = open(szName,mode);
   if(fd<0) {
     die(szName, strerror(errno));
   }
   return(fd);
}

/*
 * Shamelessly ripped from E.C. Downey's XEphem
 * Gets time from system
 */
void
time_fromsys (Now *np)
{   
#if defined(__STDC__)
   time_t t;
#else
   long t;
#endif

   trace(6,"In time_fromsys...\n",NULL);
   
   t = time(NULL);
   
   /* t is seconds since 00:00:00 1/1/1970 UTC on UNIX systems;
    * mjd was 25567.5 then.
    */
#if defined(VMS) && (__VMS_VER < 70000000)
   /* VMS returns t in seconds since 00:00:00 1/1/1970 Local Time
    * so we need to add the timezone offset to get UTC.
    * Don't need to worry about 'set_t0' and 'inc_mjd' because
    * they only deal in relative times.
    * this change courtesy Stephen Hirsch <oahirsch@southpower.co.nz>
    * - OpenVMS V7.0 finally has gmt support! so use standard time
    * - algorithm Vance Haemmerle <vance@toyvax.Tucson.AZ.US>
    */
   mjd = (25567.5 + t/3600.0/24.0) + (tz/24.0);
#else
   mjd = 25567.5 + t/3600.0/24.0;
#endif
   snprintf(szMsg,100,"TFS: %f\n",mjd);
   trace(5,szMsg,NULL);
   
   (void) tz_fromsys(np);
   
   if(tracing) {
      snprintf(szMsg,100,"TFS2 MJD: %f TZ=%f TZN=%s\n",mjd,tz,tznm);
      trace(5,szMsg,NULL);
   }
}

/* given the mjd within np, try to figure the timezone from the os.
 * return 0 if it looks like it worked, else -1.
 * 
 * Shamelessly ripped from E.C. Downey's XEphem
 */
int
tz_fromsys (Now *np)
{
   struct tm *gtmp;
   time_t t;
   
   trace(6,"In tz_fromsys...",NULL);
   t = (time_t)((mjd - 25567.5) * (3600.0*24.0) + 0.5);
   
   /* try to find out timezone by comparing local with UTC time.
    * GNU doesn't have difftime() so we do time math with doubles.
    */
   gtmp = gmtime (&t);
   if (gtmp) {
      double gmkt, lmkt;
      struct tm *ltmp;
      
      gtmp->tm_isdst = 0;	/* _should_ always be 0 already */
      gmkt = (double) mktime (gtmp);
      
      ltmp = localtime (&t);
      ltmp->tm_isdst = 0;	/* let mktime() figure out zone */
      lmkt = (double) mktime (ltmp);
      
      tz = (gmkt - lmkt) / 3600.0;
      (void) strftime (tznm, sizeof(tznm)-1, "%Z", ltmp);
      return (0);
   } else
     return (-1);
}

/*
 * General debug printer
 * The higher iLevel the more information
 * you get
 */
void
trace(int iLevel, char *szpFormat, char *szpArg)
{
   if(iLevel<=config.iTrace) {
      if(szpArg==NULL)
	printf(szpFormat);
      else
	printf(szpFormat,szpArg);
   }
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: lx200xed.c,v $ $Date: 2004/04/20 03:58:44 $ $Revision: 1.18 $ $Name:  $"};
