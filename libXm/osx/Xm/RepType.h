/* 
 *  @OPENGROUP_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  Copyright (c) 1996, 1997, 1998, 1999, 2000 The Open Group
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
 *  
 *  This software is subject to an open license. It may only be
 *  used on, with or for operating systems which are themselves open
 *  source systems. You must contact The Open Group for a license
 *  allowing distribution and sublicensing of this software on, with,
 *  or for operating systems which are not Open Source programs.
 *  
 *  See http://www.opengroup.org/openmotif/license for full
 *  details of the license agreement. Any use, reproduction, or
 *  distribution of the program constitutes recipient's acceptance of
 *  this agreement.
 *  
 *  EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, THE PROGRAM IS
 *  PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 *  WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY
 *  OR FITNESS FOR A PARTICULAR PURPOSE
 *  
 *  EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, NEITHER RECIPIENT
 *  NOR ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED
 *  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE PROGRAM OR THE
 *  EXERCISE OF ANY RIGHTS GRANTED HEREUNDER, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGES.
*/ 
/* 
 * HISTORY
*/ 
/* $XConsortium: RepType.h /main/9 1995/07/13 17:47:50 drk $ */
/* (c) Copyright 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmRepType_h
#define _XmRepType_h


#include <Xm/Xm.h>


#ifdef __cplusplus
extern "C" {
#endif


#define XmREP_TYPE_INVALID		0x1FFF

typedef unsigned short XmRepTypeId ;

typedef struct
{   
    String rep_type_name ;
    String *value_names ;
    unsigned char *values ;
    unsigned char num_values ;
    Boolean reverse_installed ;
    XmRepTypeId rep_type_id ;
    }XmRepTypeEntryRec, *XmRepTypeEntry, XmRepTypeListRec, *XmRepTypeList ;


/********    Public Function Declarations    ********/

extern XmRepTypeId XmRepTypeRegister( 
                        String rep_type,
                        String *value_names,
                        unsigned char *values,
#if NeedWidePrototypes
                        unsigned int num_values) ;
#else
                        unsigned char num_values) ;
#endif /* NeedWidePrototypes */
extern void XmRepTypeAddReverse( 
#if NeedWidePrototypes
                        int rep_type_id) ;
#else
                        XmRepTypeId rep_type_id) ;
#endif /* NeedWidePrototypes */
extern Boolean XmRepTypeValidValue( 
#if NeedWidePrototypes
                        int rep_type_id,
                        unsigned int test_value,
#else
                        XmRepTypeId rep_type_id,
                        unsigned char test_value,
#endif /* NeedWidePrototypes */
                        Widget enable_default_warning) ;
extern XmRepTypeList XmRepTypeGetRegistered( void ) ;
extern XmRepTypeEntry XmRepTypeGetRecord( 
#if NeedWidePrototypes
                        int rep_type_id) ;
#else
                        XmRepTypeId rep_type_id) ;
#endif /* NeedWidePrototypes */
extern XmRepTypeId XmRepTypeGetId( 
                        String rep_type) ;
extern String * XmRepTypeGetNameList( 
#if NeedWidePrototypes
                        int rep_type_id,
                        int use_uppercase_format) ;
#else
                        XmRepTypeId rep_type_id,
                        Boolean use_uppercase_format) ;
#endif /* NeedWidePrototypes */
extern void XmRepTypeInstallTearOffModelConverter( void ) ;

/********    End Public Function Declarations    ********/



#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmRepType_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
