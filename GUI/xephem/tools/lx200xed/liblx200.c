/*
 * liblx200 v0.8.1
 * A library for controlling the Meade LX200 series scopes
 * 
 * Copyright (C) 1999, Mike Stute
 * 
 * Todo:
 *   V0.6 will make this a daemon controlled with the usual
 *        signals with all functions available
 *   V1.1 will add lx200_errno queue and be concurrent
 *        non-blocking or async I/O or alarm on telescope reads
 *   V1.2 will add the TDB (telescope data block) for
 *        faster data retrieving and less scope chatter
 *   V1.3 will add configuration templates (if we get that far)
 *   V1.5 Clean up, bug free (hah) and ready for GTCCS
 *   V2.0 will swallow the file descriptor and no longer let
 *        the application program have access to it. But
 *        since this is concurrent, we do synchronization
 *        not the app., this may or may not be a needed by the
 *        time I get there. This means all apps would need to
 *        be rewritten; hence, the major version change
 *   
 *   Throughout all versions I am sure functions will be added
 *   to increase the high-level functionality as I get
 *   feedback (hopefully).
 * 
 * A few notes:
 * This library was designed for my use in writing the 
 * GTCCS modules (Generic Telescope and Camera Control System)
 * I toyed with two main ideas when writing it and didn't
 * end up with the initial design (hindsight vs. foresight).
 * 
 * Originally I used a table look up with a set of tokens
 * to pull the command from the tokenized table and a set
 * of about 5 functions in the Base Library Functions 
 * with a function dispatcher to handle scope control.
 * The dispatcher was called with a token and a buffer.
 * This worked great, but did limit what the
 * application program could do because the intelligence
 * was built into the dispatcher and table. If an application
 * needed lower level services, they weren't there. This was
 * because my original design was high-level control only.
 * 
 * But I decided a library that could provide both low and
 * high level services would be more useful to more people
 * and rewrote it as a module block library, that is a library
 * of bottom-up functions where small low-level are combined
 * to create high-level calls, thus allowing the application
 * access to both high- and low-level services. This yielded
 * the Base Library Functions, the Main Library Functions,
 * and the Support Library Functions. This means
 * more stack frames and a little more overhead, but in 
 * addition to providing both kinds of services, it also
 * makes the code extremely easy to maintain and read.
 *
 * Two defines are called public and private. This is
 * just a indication to application program developers
 * on my original intentions. It's a free use library
 * but I suggest you don't call private functions directly.
 * I reserve the right to change then from version to version.
 * All scope functions can be accessed through the public
 * functions.
 * 
 * As always, comments and suggestions are welcome.
 *                               Mike Stute
 *                               Feb 1999
 *                               mrstute@attbi.com
 * 
 * Changes:
 *   Fixed lx200_fset_longitude(x,y) macro as
 *       pointed out by Jason Etherton
 */

#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "liblx200.h"

#define	TREADTO		10	/* telescope read timeout, secs */

/* Globals*/
private static struct termios lx200_oldtio,lx200_newtio;
private static char *lx200_planets[9]={"Mercury","Venus","","Mars","Jupiter","Saturn","Uranus","Neptune","Pluto"};
private static char fHardwareEmulate=FALSE;
private static char iHardwareMode=TRUE;
/*TDB structure goes here*/

#undef DEBUG
/**************************************************************
 *                     I/O Functions                          *
 * These are the low-level input and output routines defined  *
 * for the scope. There is no generic "read-a-char" because   *
 * the point of liblx200 is to provided a "black box" module  *
 * for talking to the scope. An application program shouldn't *
 * need to directly call any of the I/O routines, except      *
 * opening and closing the scope. However, currently (v1.0)   *
 * the application program is returned the file descriptor    *
 * so it is possible for an app. to handle all communications *
 * with the scope directly. In that case, why is this code    *
 * linked in anyway?                                          *
 * Exports:                                                   *
 * lx200_open-scope                                           *
 * lx200_close_scope                                          *
 **************************************************************/
public int
lx200_open_scope(char *szDevice)
{
   int fd;
   
   /*Return the fake port if emulating a scope*/   
   if(fHardwareEmulate)
     return(LX200_EMULATE_FD);

   /* Open serial device to telescope */
   fd = open(szDevice,O_RDWR|O_NOCTTY);
   if (fd<0) {
#ifdef DEBUG
      perror(szDevice);
#endif      
      return(LX200_FALSE); 
   }
   
   /* Set the terminal for 9600,8-N-1
    * Non-canonical input processing
    * no flow control
    */

   /*Clear the new term struct*/
   memset(&lx200_newtio,0,sizeof(lx200_newtio));
   tcgetattr(fd,&lx200_oldtio); /*save port settings*/
   
   /* Set for 9600 CS8=8n1n CLOCAL=local connection CREAD=enable reading */
   cfsetospeed(&lx200_newtio, (speed_t) B9600);
   cfsetispeed(&lx200_newtio, (speed_t) B9600);
   lx200_newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
   lx200_newtio.c_iflag = IGNPAR;
   lx200_newtio.c_oflag = 0;   /* Raw output */
   
   /* set input mode (non-canonical, no echo, ... */
   lx200_newtio.c_lflag = 0;
   
   lx200_newtio.c_cc[VTIME] = 0; /* inter-character timer unused, block instead */
   lx200_newtio.c_cc[VMIN] = 1;  /* read 1 character minimum */
   
   tcflush(fd, TCIFLUSH);             /* clear the channel */
   tcsetattr(fd,TCSANOW,&lx200_newtio);     /* set it */
   return(fd);
}

/* Simple for serial scopes, not so simple for parallel
 * or special dev scopes. 
 * Return saved terminal settings and close the device.
 */
public int
lx200_close_scope(int fd)
{
   if(!fHardwareEmulate) {
      tcsetattr(fd,TCSANOW,&lx200_oldtio);
      close(fd);
   }
   return(LX200_TRUE);
}

/* 
 * You would think Meade designed the scope with blocking I/O in mind
 * since they terminate everything with a TERMINATOR of #. But they
 * broke their own rule, when they return "0" pr "1" with no terminator.
 * This function reads and verifies an expected "0"
 * if it doesn't ever come back from the scope, we're toast
 * in the current version. But that indicates a problem with
 * either the scope or the serial link, so it may just stay that
 * way in the future. 
 * 
 * Perhaps ASYNC IO is in order for the scope channel
 */
private int
lx200_read_ok(int fd)
{
   char cIn;
   
     /*Fake a hardware response*/
   if(fHardwareEmulate) {
      switch(iHardwareMode) {
       case LX200_EMULATE_FALSE:
	 return(LX200_FALSE);
         break;
       case LX200_EMULATE_TRUE:
	 return(LX200_TRUE);
         break;
      }
   }
   
   if((cIn=lx200_read_one(fd))==LX200_FALSE)
     return(LX200_FALSE);
   if(cIn=='1')
     return(LX200_TRUE);
   return(LX200_FALSE);
}

/* 
 * Again Meade doesn't terminate with a '#' when returning
 * a 0, 1, or 2 for some calls
 * I rest my case
 * The scope uses these to indicate success or errors for a few different calls
 */
private char
lx200_read_one(int fd)
{
   struct timeval tv;
   fd_set rd;
   char szIn[2];
   int iBytes;
   
     /* Fake a hardware response*/
   if(fHardwareEmulate) {
      switch(iHardwareMode) {
       case LX200_EMULATE_FALSE:
	 return('0');
         break;
       case LX200_EMULATE_TRUE:
	 return('1');
         break;
      }
   } 

   /* only wait so long for a response */
   FD_ZERO (&rd);
   FD_SET (fd, &rd);
   tv.tv_sec = TREADTO;
   tv.tv_usec = 0;
   if (select (fd+1, &rd, NULL, NULL, &tv) != 1)
       return(LX200_FALSE);
   
   iBytes = read(fd,szIn,1);
   if(iBytes==1)
     return(szIn[0]);
   return(LX200_FALSE);
}

/* Read two from the scope and return them in a string*/
private int 
lx200_read_two(int fd, char *szIn)
{
   int iBytes;

   if(fHardwareEmulate) {
      switch(iHardwareMode) {
       case LX200_EMULATE_FALSE:
	 return(LX200_FALSE);
         break;
       case LX200_EMULATE_TRUE:
	 szIn[0]='1';
	 szIn[1]='2';	 
	 szIn[2]='\0';	 
	 return(LX200_TRUE);
         break;
      }
   }
   
   iBytes = lx200_read_one (fd);
   if (iBytes == LX200_FALSE)
       return(LX200_FALSE);
   szIn[0] = iBytes;
   iBytes = lx200_read_one (fd);
   if (iBytes == LX200_FALSE)
       return(LX200_FALSE);
   szIn[1] = iBytes;
   szIn[2]='\0';
   return(LX200_TRUE);
}

/*
 * Write a command to the scope
 * Prefix the INITIATOR
 * Suffix the TERMINATOR
 * write to the stream
 * ALL comunications to the scope must use this
 * except send ACK in get_scope_mode()
 * Gotta love Meade's consistentecy
 */
private int 
lx200_write_to_scope(int fd, char *szpCmd)
{
   char szCmd[17];
     
   strcpy(szCmd,LX200_INITIATOR);      /*Initiator*/
   strncat(szCmd,szpCmd,14);
   strcat(szCmd,LX200_TERMINATOR);     /*Terminator*/
#ifdef DEBUG
   printf("Out: %s on %d\n",szCmd, fd);
#endif

   if(fHardwareEmulate) {
      switch(iHardwareMode) {
       case LX200_EMULATE_FALSE:
	 return(LX200_FALSE);
         break;
       case LX200_EMULATE_TRUE:
	 return(strlen(szCmd));
         break;
      }
   }
   
   return(write(fd,szCmd,strlen(szCmd)));
}

/*
 * Send an ACK to the scope
 * This is only used by lx200_get_mode()
 */
private int
lx200_send_ACK(int fd)
{
   char szACK[1]={(char)0x06};

   if(fHardwareEmulate) {
      switch(iHardwareMode) {
       case LX200_EMULATE_FALSE:
	 return(0);
         break;
       case LX200_EMULATE_TRUE:
	 return(1);
         break;
      }
   }
   
   return(write(fd,szACK,1));
}

/*
 * Reads a terminated line from the scope
 * Returns the number of chars read
 */
private int 
lx200_read_from_scope(int fd, char *szpBuffer)
{
   unsigned char szIn[5], *szp=szpBuffer;
   int iChars = 0, iBytes;
   
   /* Emulation will be performed with an addition to lx200_get_generic()
    * of a parameter of what to return in each instance. But,
    * full hardware emulation is for later
    */
   if(fHardwareEmulate)
	 return(TRUE);
   
   while(1) {                      /* loop from input */
      iBytes = lx200_read_one (fd);
      if (iBytes == LX200_FALSE)
	return (LX200_FALSE);
      szIn[0] = iBytes;                 /* to debug later */
      if(szIn[0]==LX200_TERMINATOR_C) {
          *szp='\0';
#ifdef DEBUG
	 printf("Reply: %s\n",szpBuffer);
#endif
	 break;
      }
      *szp++=szIn[0];
      iChars++;
   }
   return iChars;
}   

/****************************************************************************
 *                       Base Library Functions                             *
 * These functions perform a single action by communicating with the scope  *
 * in a simple step. The output is performed and any input is returned.     *
 * All functions return 1 on success and -1 on failure.                     *
 * Any informational functions return values as parameters                  *
 * These simple functions are combined to perform more powerful functions   *
 * All functions should should make a single call to use, and await a       *
 * return value, indicating success or failure.                             *
 *                                                                          *
 * An application may choose to use some of these, so most are exported     *
 * Exports:                                                                 *
 ****************************************************************************/

/*
 * The generic get routine
 * Given a command, it returns the raw string form the scope
 * with appropriate error checking
 */
private int
lx200_get_generic(int fd, char *szCmd, char *buf, char *szEmulated)
{
 
   if(fHardwareEmulate) {
      if(iHardwareMode==TRUE) {
	 strcpy(buf,szEmulated);
	 return(LX200_TRUE); 
      }
      else {
	 return(LX200_FALSE);
      }
   }
       
   if(!lx200_write_to_scope(fd,szCmd))   /*Request RA*/
     return(LX200_FALSE);
   if(!lx200_read_from_scope(fd, buf))
     return(LX200_FALSE);
   return(LX200_TRUE);
}

/********************** Get Functions ****************************/
/* Most of these are implemented through a macro in liblx200.h
 * to call lx200_get_generic()
 * There really ins't a lot of code this way.
 * See lx200_get in liblx200.h
 */

/*
 * Request the alignment mode
 * returns on of the following:
 * LX200_OPT_MODE_ALTAZ   - Alt-Az mode
 * LX200_OPT_MODE_POLAR   - Polar aligned
 * LX200_OPT_MODE_LAND    - Land mode (no tracking)
 * LX200_OPT_MODE_GPOLAR  - German Polar
 * LX200_FALSE            - Smell the smoke?
 */
public int
lx200_get_mode(int fd)
{
   char cMode;

   if(fHardwareEmulate) {
      if(iHardwareMode==TRUE)
	return(LX200_MODE_POLAR);
      else
	return(LX200_FALSE);
   }
   
   if(lx200_send_ACK(fd)==LX200_FALSE)
     return(LX200_FALSE);
   cMode = lx200_read_one(fd);
   switch(cMode) {
    case 'A':
      return(LX200_MODE_ALTAZ);
    case 'L':
      return(LX200_MODE_LAND);
    case 'P':
      return(LX200_MODE_POLAR);
    case 'G':
      return(LX200_MODE_GPOLAR);
    default:
      return(LX200_FALSE);
   } 
}

/* Macro defined functions
 * Request the Alt and return a string in format sDD:MM:SS
 * lx200_get_alt()

 * Request the Az and return a string in format DDD:MM:SS
 * lx200_get_az()

 * Request the sidereal time and return a string in format HH:MM:SS
 * lx200_get_sidereal()

 * Request the local time and return a string in format DDD:MM:SS
 * lx200_get_local12()

 * Request the local time and return a string in format DDD:MM:SS
 * lx200_get_local24()

 * Request the date and return a string in format MM/DD/YY
 * lx200_get_date()

 * Request the site latitude and return a string in format sDD*MM
 * lx200_get_latitude()

 * Request the site longitude and return a string in format DDD*MM
 * lx200_get_longitude()

 * Request the offset from GMT and return a string in format sHH
 * lx200_get_GMT_offset()

 * Request the object RA and return a string in format HH:MM:SS
 * lx200_get_obj_RA()

 * Request the object dec and return a string in format sDD*MM:SS
 * lx200_get_obj_dec()

 * Request the filter type string and return a string containing the types
 * Types can be looked up with lx200_filter_type_map()
 * A type string can be broken into an array of types with lx200_filter_string_map()
 * lx200_get_filter_type()

 * Request the filter quality and return a string with the quality code
 * The quality code can be looked up with lx200_filter_quality_map()
 * lx200_get_filter_quality()

 * Request the filter horizon limit and return a string in format DD*
 * lx200_get_filter_horizon()

 * Request the filter minimum magnitude and return a string in format sMM.M
 * lx200_get_filter_minmag()

 * Request the filter maximum magnitude and return a string in format sMM.M
 * lx200_get_filter_maxmag()

 * Request the filter minimum size and return a string in format NNN'
 * lx200_get_filter_minsize()

 * Request the filter maximum size and return a string in format NNN'
 * lx200_get_filter_maxsize()

 * Request the field radius and return a string in format NNN'
 * lx200_get_field_radius()

 * Request the field information and return it in a string
 * The string contains the number of objects in the field
 * and the object closest to the center
 * use lx200_field_info_map to crack it
 * lx200_get_field_info()

 * Request the object information
 * use lx200_obj_info_map() to build an lx200_obj structure
 * lx200_get_obj_field()

 * Request the track frequency in the format TT.T
 * use lx200_freq_map() to map to a double
 * lx200_get_track()
 
 * Request the status bars on a GOTO
 * use lx200_status_map() to map to an integer indicating
 * how far the GOTO is along
 * lx200_get_status()
 
 */

/*
 * Request a site name (0-4), this is a three letter code
 * The code is virtually worthless, but it probably
 * means something to the user
 */
public int
lx200_get_site_name(int fd, char *buf, int iSite)
{
   char szCmd[3]="G";
   
   switch(iSite) {
    case 1:
      szCmd[1]='M';
      break;
    case 2:
      szCmd[1]='N';
      break;
    case 3:
      szCmd[1]='O';
      break;
    case 4:
      szCmd[1]='P';
      break;
    default:
      return(LX200_FALSE);
   }
   szCmd[2]='\0';  /*Don't forget the Terminator*/   
   return(lx200_get_generic(fd, szCmd, buf,"PBY"));
}

/*
 * Request the status of the clock
 * returns LX200_OPT_CLOCK24 if it's set for 24 hours
 * returns LX200_OPT_CLOCK12 if it's set for 12 hours
 */
public int
lx200_get_clock_format(int fd) 
{
   char szIn[3];
   
   if(fHardwareEmulate) {
      if(iHardwareMode==TRUE)
	return(LX200_OPT_CLOCK12);
      else
	return(LX200_FALSE);
   }
   
   if(lx200_read_two(fd, szIn)==LX200_FALSE)
     return(LX200_FALSE);
   if(szIn[0]=='1' && szIn[1]=='2')
     return(LX200_OPT_CLOCK12);
   if(szIn[0]=='2' && szIn[1]=='4')
     return(LX200_OPT_CLOCK24);
   return(LX200_FALSE);
}

/******************* Telescope Movement ************************/
/* Starts the scope slewing to set object */
public int
lx200_goto(int fd)
{
   char response,szReturn[201];

   if(fHardwareEmulate) {
      if(iHardwareMode==TRUE)
	return(LX200_TRUE);
      else
	return(LX200_FALSE);
   }
   
   if(lx200_write_to_scope(fd,"MS")==LX200_FALSE)
     return(LX200_FALSE);
   response=lx200_read_one(fd);
   if(response=='0')
      return(LX200_TRUE);
   lx200_read_from_scope(fd,szReturn);
   return(LX200_FALSE);
}

/************************ Set Commands **************************/
/* All _fset_ expect a preformatted string
 * Use the lx200_format commands to format the strings properly
 * Here was the first decision I could go either way with.
 * 
 * All these functions except a string, ie, they have been
 * preformatted. The intention being an app. first calls
 * a formatter, then the set command. But this means the
 * app. has to make two calls, one to format then one
 * to send. Some apps may not need to format, others
 * might, so I came up with two sets of calls.
 * 
 * The unformatted calls in which each function accepts the
 * proper number of parameters, calls the formatter, and
 * sends the command. Each function has a varying number of
 * parameters depending on it's function. This means lots
 * of referencing the library documentation to find out
 * what accepts what. Ugly (especially when, if you're
 * reading this, you're reading the documentation for now).
 * 
 * The formatted set expects to to be previously
 * formatted and sends it. This makes the unformatted calls 
 * wrappers to the formatted calls. Then we use about 10 functions
 * to create the proper format. They implement the unformatted
 * calls as macros (most anyway) to call the proper formatter and the 
 * asscoiated formatted call. This means less reference work, 
 * but more complex library code. I'd rather have the library 
 * complicated then the application.
 * 
 * With this in place, anyone gets whatever they need, though
 * most will probably end up using the macros in the header 
 * file. If the application performs verification and formatting
 * the _fset_ calls are used
 */

/*
 * Send out a set command
 * All sets are followed by the return of "Ok"
 */
private int
lx200_set_generic(int fd, char *szCmd, char *szValue)
{
   char szFCmd[255];
   
   snprintf(szFCmd,254,"%s%s",szCmd,szValue);
   if(lx200_write_to_scope(fd,szFCmd)==LX200_FALSE)
     return(LX200_FALSE);
   return(lx200_read_ok(fd));
}

/* This sends a command that the scope will do or not do
 * as it sees fit, because it doesn't tell us anything.
 * Therefore we always return the value of the write
 * operation the scope is actually does it
 * 
 * Used primarily for macro expansion
 */
private int
lx200_send_command(int fd, char *szCmd)
{
   return((lx200_write_to_scope(fd,szCmd)==LX200_FALSE));
}

/***** The formated calls*******
 * Keep in mind that using these means the application is
 * responsible for ALL data verifcation. If you ask a 
 * user and supply that input to a _fset_, make sure
 * you validate it before passing it to the library.
 * 
 * Use the _set_ functions with raw data to get proper
 * validation
 */

/*
 * lx200_toggle_format()
 * 
 * The scope can be set for long format using this call
 * Scopes with version 3.30 or above can use this
 * Note with liblx200 v1.0 the short format is
 * NOT supported (too inaccurate) and probably never 
 * will be.
 * 
 * There is no way to determine the current state of the scope,
 * so this function can only toggle. Use lx200_set_format()
 * to specify a particular format.
 * 
 * The formatted ( _fset_ ) functions are marked private
 * but feel free to call them. You won't be invalidating
 * the library.
 * 
 * (MACRO)
 */


/* The macro functions*/
/*
 * Set sidereal time
 * Format is HH:MM:SS (24 hour clock)
 * lx200_fset_sidereal()

 * Set target local time
 * Format is HH:MM:SS
 * lx200_fset_local24()

 * Set target date
 * Format is MM/DD/YY
 * lx200_fset_date()

 * Set site latitude
 * Format is sDD*MM
 * lx200_fset_latitude()
 
 * Set site longitude
 * Format is sDD*MM
 * lx200_fset_longitude()
 
 * Set GMT offset
 * Format is sHH
 * lx200_fset_GMT_offset()
 
 * Set object RA
 * Format is HH:MM:SS
 * lx200_fset_obj_RA()
 
 * Set object declination
 * Format is sDD*MM:SS
 * lx200_fset_obj_dec()
  
 * Set the filter type string
 * Format is special
 * lx200_fset_filter_type()
 
 * Set the filter horizon 
 * Format is DD
 * lx200_fset_filter_horizon()
 
 * Set the filter minimum magnitude
 * Format is sMM.M
 * lx200_fset_filter_minmag()
 
 * Set the filter maximum magnitude
 * Format is sMM.M
 * lx200_fset_filter_maxmag()
 
 * Set the filter minimum size
 * Format is NNN (in arc minutes)
 * lx200_fset_filter_minsize()
 
 * Set the filter maximum size
 * Format is NNN (in arc minutes)
 * lx200_fset_filter_maxsize()

 * Set the field radius
 * Format is NNN
 * lx200_fset_field_radius

 * Set the catalog to use for stars
 *   format is N

 * Set target to a star
 * Format is NNNN
 * This selects within the current catalog
 * Use lx200_set_star_catalog() to choose a catalog
 * lx200_goto_star() does both
 */


/*
 * Set the calendar date
 * Could have used a macro but
 * the scope returns "Updating planetary data"
 * first followed by a string of blanks.
 * The only questions is, "Why?"
 */
public int
lx200_fset_date(int fd, char *sDate)
{
   char szReturn[51],szCmd[11];
   
   snprintf(szCmd,10,"SC%8s",sDate);
   if(lx200_write_to_scope(fd,szCmd)==LX200_FALSE)
     return(LX200_FALSE);
   if(lx200_read_from_scope(fd,szReturn))
     return(LX200_FALSE);
   if(lx200_read_from_scope(fd,szReturn))
     return(LX200_FALSE);
   return(LX200_TRUE);   
}

/*
 * Set the reticle brightness to a level 1-3
 * Mode is 
 * LX200_OPT_CONTINOUS  reticle on all the time
 * LX200_OPT_FLASH50    reticle on 50%
 * LX200_OPT_FLASH25    reticle on 25%
 * LX200_OPT_FLASH10    reticle on 10%
 */
public int
lx200_set_reticle_flash(int fd, int iMode)
{
   char szCmd[4]="B";
   
   if(iMode<LX200_OPT_CONTINOUS || iMode>LX200_OPT_FLASH10)
     return(LX200_FALSE);
   
   szCmd[1]=iMode - LX200_OPT_CONTINOUS;
   szCmd[2]='\0';
   return(lx200_send_command(fd,szCmd));
   
}

/*
 * Sync on current object
 */
public int
lx200_obj_sync(int fd, char *szpObj)
{
   if(lx200_send_command(fd,"CM")==FALSE)
     return(LX200_FALSE);
   if(lx200_read_from_scope(fd,szpObj)==LX200_FALSE)
     return(LX200_FALSE);
   return(LX200_TRUE);
}

/*
 * Set a site (1-4) name to a
 * three letter name
 */
public int
lx200_set_site_name(int fd, char *buf, int iSite)
{
   char szCmd[3]="S",szSite[4];
   
   if(iSite<1 || iSite>4)  /*Error condition invalid site*/
     return(LX200_FALSE);
   if(strlen(buf)>3 || buf[0]=='\0')      /*Error condition invalid name*/
     return(LX200_FALSE);  /*Probably ought to verify all characters are acceptable*/
   szCmd[1]='L' + iSite;
   szCmd[2]='\0';
   snprintf(szSite,6,"%3s",buf);
   return(lx200_set_generic(fd,szCmd,szSite));   
}

/************************************************************************
 *                        Main Library Functions                        *
 * These functions are the ones the most applications programs will     *
 * call. They combine the base library functions together to perform    *
 * large actions, such as selecting and slewing to a target object,     *
 * returning scope information, or performing telescope finds.          *
 *                                                                      *
 * These are consider the high-level routines of the library.           *
 ************************************************************************/
/*
 * Given a star number and catalog, this performs a goto
 */
/*
public int
lx200_goto_star(int fd, int iStar, int iCatalog)
{
   
#ifdef DEBUG
   printf("Called lx200_goto_star for %d\n",iStar,iCatalog);  fflush(stdout);
#endif
   if(lx200_set_star_catalog(fd,iCatalog)==LX200_FALSE)
     return(LX200_FALSE);
   if(lx200_set_star(fd, iStar)==LX200_FALSE)
     return(LX200_FALSE);
   return(lx200_goto(fd));
}
*/

/*
 * Goto a given Dec and RA
 */
public int
lx200_goto_RADec(int fd, char *szpRa, char *szpDec)
{
   if(lx200_fset_obj_RA(fd, szpRa)==LX200_FALSE)
     return(LX200_FALSE);
   if(lx200_fset_obj_dec(fd, szpDec)==FALSE)
     return(LX200_FALSE);
   return(lx200_goto(fd));
}

/*
 * Goto to an extended object
 * Need catalog which is one of the following
 * LX200_MESSIER_CATLOG
 * LX200_NGC_CATALOG
 * LX200_IC_CATALOG
 * LX200_UGC_CATALOG
 *  and the object number
 */
public int
lx200_goto_ext(int fd, int iObj, int iCatalog)
{
   char szObj[6];
    
       /*Check catalog. Is it valid?*/
   if(iCatalog<LX200_MESSIER_CATALOG || iCatalog > LX200_UGC_CATALOG)
     return(LX200_FALSE);
   if(iObj<1)   /*Object less then one*/
     return(LX200_FALSE);
   
   switch(iCatalog) {
    case LX200_MESSIER_CATALOG:
      if(iObj>101)
	return(LX200_FALSE);
      if(lx200_format_messier(iObj,szObj))
	return(LX200_FALSE);
      if(lx200_fset_messier(fd,szObj))
	return(LX200_FALSE);
      break;
    case LX200_NGC_CATALOG:
      if(iObj>7840)
	return(LX200_FALSE);
          /*I can cheat, it's my library!*/
      if(lx200_set_ext_ngc(fd)==LX200_FALSE)
	return(LX200_FALSE);
      if(lx200_format_ngc(iObj,szObj))
	return(LX200_FALSE);
      if(lx200_fset_ext(fd,szObj)==LX200_FALSE)
	return(LX200_FALSE);
      break;
    case LX200_IC_CATALOG:
      if(iObj>5386)
	return(LX200_FALSE);
      if(lx200_set_ext_ic(fd)==LX200_FALSE)
	return(LX200_FALSE);      
      if(lx200_format_ic(iObj,szObj))
	return(LX200_FALSE);
      return(lx200_fset_ext(fd,szObj));
      break;
    case LX200_UGC_CATALOG:
      if(iObj>12921)
	return(LX200_FALSE);
      if(lx200_set_ext_ugc(fd)==LX200_FALSE)
	return(LX200_FALSE);
      if(lx200_format_ugc(iObj,szObj))
	return(LX200_FALSE);
      if(lx200_fset_ext(fd,szObj)==LX200_FALSE)
	return(LX200_FALSE);
      break;      
    default:
      return(LX200_FALSE);
   }
   return(lx200_goto(fd));
}

/* Set the format to long or short
 * All other liblx200 functions
 * assume the format is long, so the application
 * currently needs to call lx200_set_format
 * passing in LX200_OPT_LONG_FORMAT
 * if anything involing degrees
 * hours minutes and seconds
 * Old format     New format
 * HH:MM.T        HH:MM:SS
 * sDD*MM         sDD:MM:SS
 * DDD*MM         SSS*MM:SS
 */
public int
lx200_set_format(int fd, int iSetState)
{
   char szBuf[15];
   int iCurState;

   /* Get the current state.*/
   if(lx200_get_dec(fd, szBuf)==LX200_FALSE)
     return(LX200_FALSE);
#ifdef DEBUG
   printf("lx200lib %s\n",szBuf);
#endif
   if(strlen(szBuf)<8)
     iCurState=LX200_OPT_SHORT_FORMAT;
   else
     iCurState=LX200_OPT_LONG_FORMAT;
   
   if(iSetState!=iCurState)
     return(lx200_toggle_format(fd));
   return(LX200_TRUE);
}

/*
 * Set filter type
 * Type is a bitmap of the following
 * LX200_TYPE_GALAXIES
 * LX200_TYPE_PLANETARY
 * LX200_TYPE_DIFFUSE
 * LX200_TYPE_GLOBULAR
 * LX200_TYPE_OPEN
 */
private int
lx200_set_filter_type(int fd, int iType)
{
   char szType[5]="";
   
   /*Build type string*/
   if(iType & LX200_TYPE_GALAXIES)
     strcat(szType,"G");
   if(iType & LX200_TYPE_PLANETARY)
     strcat(szType,"P");
   if(iType & LX200_TYPE_DIFFUSE)
     strcat(szType,"D");
   if(iType & LX200_TYPE_GLOBULAR)
     strcat(szType,"C");
   if(iType & LX200_TYPE_OPEN)
     strcat(szType,"O");
   
   return(lx200_fset_filter_type(fd,szType));    
}

/* Set site number 1 - 4
 * see lx200_get_site_name() and lx200_set_site_name()
 */
public int
lx200_set_site_number(int fd, int iSite)
{
   char szCmd[5]="W";
   
   if(iSite<1 || iSite>4)
     return(LX200_FALSE);
   szCmd[1]='0' + iSite;
   return(lx200_send_command(fd,szCmd));
}

/*************************************************************************
 *                   Support Library Functions                           *
 * These functions are support functions that don't actually perform     *
 * telescope functions. Instead they perform such actions as formatting  *
 * data, performing table lookups, and handling data conversion.         *
 *************************************************************************/

/* 
 * A very fast way to break LX200 RA string into integers
 * And no, I won't stop using atoi() in lieu of strtol().
 * Some things just don't need to be fixed. You can have 
 * it when you pry the keyboard from my cold, dead
 * fingers.
 */
public int
lx200_convert_RA(char *szpRA, int *RH, int *RM, int *RS)
{
   char s[3];
   
   s[2] = 0;
   /*Get R HOUR*/
   s[0] = szpRA[0];
   s[1] = szpRA[1];
   *RH=atoi(s);
   
   /*Get R Minute*/
   s[0] = szpRA[3];
   s[1] = szpRA[4];
   *RM=atoi(s);

   /*Get R Seond*/
   s[0] = szpRA[6];
   s[1] = szpRA[7];
   *RS=atoi(s);
   
   return(LX200_TRUE);
}

/* 
 * A very fast way to break LX200 Dec string into integers
 */
public int
lx200_convert_Dec(char *szpRA, int *DD, int *DM, int *DS)
{
   char s[4];
   
   s[3] = 0;
   
   /*Get Dec Degrees*/
   s[0] = szpRA[0];
   s[1] = szpRA[1];
   s[2] = szpRA[2];
   *DD=atoi(s);
   
   /*Get Dec Minute*/
   s[0] = szpRA[4];
   s[1] = szpRA[5];
   s[2] = 0;
   *DM=atoi(s);

   /*Get D Second*/
   s[0] = szpRA[7];
   s[1] = szpRA[8];
   *DS=atoi(s);
   
   return(LX200_TRUE);
}

/* 
 * Given a planet name, this function returns the star number
 * if the planet string isn't real, return -1 */
public int
lx200_map_planet_id(char *szName)
{
   int i;
   
   for(i=0;i<9;i++)
     if(!strcmp(szName,lx200_planets[i]))
       return(900 + i);
   return(LX200_FALSE);
}

/************************ Format functions ******************************
 * These perform generic format routines
 * and are called by macro in almost all
 * cases
 * Once again, an application can call them
 * directly, but the macros make more sense.
 * the _format_ macros are put together from
 * the are _fmt_ functions
 */

/*An integer iDigits in length*/
private int
lx200_fmt_number(int iNumber, int iDigits, char cEnd, char *szp)
{
   char szFormat[8],szEnd[2];
   
   sprintf(szFormat,"%c%ds",'%',iDigits);
   sprintf(szp,szFormat,iNumber);
   if(cEnd!='\0') {
      szEnd[0]=cEnd;
      szEnd[1]='\0';
      strcat(szp,szEnd);
   }
     
   return(LX200_TRUE);
}

/* NN<ifs>NN<ifs>NN for
 * HH:MM:SS
 * MM/DD/YY
 */
private int
lx200_fmt_time(int HH, int MM, int SS, char cIFS1, char cIFS2, int iSigned, char *szp)
{
   if(iSigned)
     snprintf(szp,9,"%c%2d%c%2d%c%2d",HH < 0 ? '-' : '+',HH,cIFS1,MM,cIFS2,SS);
   else
     snprintf(szp,8,"%2d%c%2d%c%2d",HH,cIFS1,MM,cIFS2,SS);
   return(LX200_TRUE);
}

/*DDD<ifs>MM<ifs>SS*/
private int
lx200_fmt_coord(int DDD, int MM, int SS, char cIFS1, char cIFS2, char *szp)
{
   snprintf(szp,9,"%3d%c%2d%c%2d",DDD,cIFS1,MM,cIFS2,SS);
   return(LX200_TRUE);
}

/*sHH or DD or DD(*) */
private int
lx200_fmt_hour(int HH, int iSigned, char cEnd, char *szp)
{
   char szEnd[2];
   
   if(iSigned)
     snprintf(szp,3,"%c%2d",HH < 0 ? '-' : '+',HH);
   else
     snprintf(szp,2,"%2d",HH);
   if(cEnd!='\0') {
      szEnd[0]=cEnd;
      szEnd[1]='\0';
      strcat(szp,szEnd);
   }
   return(LX200_TRUE);
}

/*sMM.M or TT.T*/
private int
lx200_fmt_magnitude(double d, int iSigned, char *szp)
{
   if(iSigned)
     snprintf(szp,5,"%c%3.1f",(char)d < 0 ? '-' : '+',d);
   else
     snprintf(szp,4,"%3.1f",d);
   return(LX200_TRUE);
}


/******************* MISC *********************/

/*
 * Return the version of liblx200 in a string
 */
public int
lx200_get_lib_version(char *sz)
{
   strcpy (sz, "$Revision: 1.8 $");
   return(LX200_TRUE);
}

/* 
 * Control hardware emulate mode
 */
public int
lx200_set_lib_emulate(int fEmulate, int iMode)
{
   if(iMode!=LX200_EMULATE_FALSE && iMode!=LX200_EMULATE_TRUE)
     return(LX200_FALSE);
   if(fEmulate==LX200_TRUE) {
      fHardwareEmulate=LX200_TRUE;
      iHardwareMode=iMode;
   }
   else
     if(fEmulate==LX200_FALSE)
       fHardwareEmulate=LX200_FALSE;
   else
     return(LX200_FALSE);
   return(LX200_TRUE);
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: liblx200.c,v $ $Date: 2002/01/04 17:36:52 $ $Revision: 1.8 $ $Name:  $"};
