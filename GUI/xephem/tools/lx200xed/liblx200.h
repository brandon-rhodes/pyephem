#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define LX200_FALSE -1
#define LX200_TRUE   1

#define LX200_EMULATE_FALSE    0
#define LX200_EMULATE_TRUE     1
#define LX200_EMULATE_FD    5000

/*Setable values for options*/
#define LX200_OPT_LONG_FORMAT  1
#define LX200_OPT_SHORT_FORMAT 2
#define LX200_OPT_CLOCK24      3
#define LX200_OPT_CLOCK12      4
#define LX200_OPT_CONTINOUS    5
#define LX200_OPT_FLASH50      6
#define LX200_OPT_FLASH25      7
#define LX200_OPT_FLASH10      8

/*Object type bit masks*/
#define LX200_TYPE_GALAXIES     0x00
#define LX200_TYPE_PLANETARY    0x01
#define LX200_TYPE_DIFFUSE      0x02
#define LX200_TYPE_GLOBULAR     0x04
#define LX200_TYPE_OPEN         0x08

/*telescope modes*/
#define LX200_MODE_ALTAZ       1
#define LX200_MODE_LAND        2
#define LX200_MODE_POLAR       3
#define LX200_MODE_GPOLAR      4

#define BAUDRATE B9600
#define TELESCOPE "/dev/ttyS0"

#define DEBUG

#define _POSIX_SOURCE 1

#define LX200_INITIATOR    ":"
#define LX200_TERMINATOR   "#"
#define LX200_INITIATOR_C  ':'
#define LX200_TERMINATOR_C '#'
#define LX200_DEGREE       223
#define LX200_RETURN_OK    '0'

/* Catalogs */
#define LX200_ALIGN_CATALOG     0
#define LX200_SAO_CATALOG       1
#define LX200_GCVS_CATALOG      2
#define LX200_MESSIER_CATALOG   3
#define LX200_NGC_CATALOG       4
#define LX200_IC_CATALOG        5
#define LX200_UGC_CATALOG       6

/*Some code clarifiers*/
#define private
#define public

/*Pre-defines*/
private char lx200_read_one(int);
private int lx200_fmt_number(int, int, char, char *);

/*The _get_ macros */
#define lx200_get(x,y,z,s)              lx200_get_generic(x,y,z,s)
#define lx200_get_ra(x,y)               lx200_get(x,"GR",y,"05:55:3")
#define lx200_get_dec(x,y)              lx200_get(x,"GD",y,"+07:23:26")
#define lx200_get_alt(x,y)              lx200_get(x,"GA",y,"62:06:29")
#define lx200_get_ax(x,y)               lx200_get(x,"GZ",y,"206:57:17")
#define lx200_get_sidereal(x,y)         lx200_get(x,"GS",y,"01:30:15")
#define lx200_get_local12(x,y)          lx200_get(x,"Ga",y,"01:30:15")
#define lx200_get_local24(x,y)          lx200_get(x,"GL",y,"21:30:15")
#define lx200_get_date(x,y)             lx200_get(x,"GC",y,"11/25/99")
#define lx200_get_latitude(x,y)         lx200_get(x,"Gt",y,"+32:47:09")
#define lx200_get_longitude(x,y)        lx200_get(x,"Gg",y,"+96:47:37")
#define lx200_get_GMT_offset(x,y)       lx200_get(x,"GG",y,"+06")
#define lx200_get_obj_RA(x,y)           lx200_get(x,"Gr",y,"+06:08:44")
#define lx200_get_obj_dec(x,y)          lx200_get(x,"Gd",y,"+24:15:18")
#define lx200_get_filter_type(x,y)      lx200_get(x,"Gy",y,"GPDC")
#define lx200_get_filter_quality(x,y)   lx200_get(x,"Gq",y,"EX")
#define lx200_get_filter_horizon(x,y)   lx200_get(x,"Gh",y,"00")
#define lx200_get_filter_minmag(x,y)    lx200_get(x,"Gb",y,"+20.0")
#define lx200_get_filter_maxmag(x,y)    lx200_get(x,"Gf",y,"-01.0")
#define lx200_get_filter_minsize(x,y)   lx200_get(x,"GI",y,"000'")
#define lx200_get_filter_maxsize(x,y)   lx200_get(x,"Gs",y,"200'")
#define lx200_get_field_radius(x,y)     lx200_get(x,"GF",y,"015'")
#define lx200_get_field_info(x,y)       lx200_get(x,"Lf",y,"004 CNGC1976 SU DNEB MAG 3,9 SZ 66.0'")
#define lx200_get_obj_field(x,y)        lx200_get(x,"LI",y,"CNGC1976 SU DNEB MAG 3,9 SZ 66.0'")
#define lx200_get_track(x,y)            lx200_get(x,"GT",y,"60.1")
#define lx200_get_status(x,y)           lx200_get(x,"D",y,"24")

/*The _fset_ marcos*/
#define lx200_set(x,y,z)                lx200_set_generic(x,y,z)
#define lx200_fset_sidereal(x,y)        lx200_set(x,"SS",y)
#define lx200_fset_local24(x,y)         lx200_set(x,"SL",y)
#define lx200_fset_latitude(x,y)        lx200_set(x,"St",y)
#define lx200_fset_longitude(x,y)       lx200_set(x,"Sg",y)
#define lx200_fset_GMT_offset(x,y)      lx200_set(x,"SG",y)
#define lx200_fset_obj_RA(x,y)          lx200_set(x,"Sr",y)
#define lx200_fset_obj_dec(x,y)         lx200_set(x,"Sd",y)
#define lx200_fset_filter_type(x,y)     lx200_set(x,"Sy",y)
#define lx200_fset_filter_horizon(x,y)  lx200_set(x,"Sh",y)
#define lx200_fset_filter_minmag(x,y)   lx200_set(x,"Sb",y)
#define lx200_fset_filter_maxmag(x,y)   lx200_set(x,"Sf",y)
#define lx200_fset_filter_minsize(x,y)  lx200_set(x,"SI",y)
#define lx200_fset_filter_maxsize(x,y)  lx200_set(x,"Ss",y)
#define lx200_fset_field_radius(x,y)    lx200_set(x,"SF",y)
#define lx200_fset_track_freq(x,y)      lx200_set(x,"ST",y)
#define lx200_fset_star_catalog(x,y)    lx200_set(x,"Ls",y)
#define lx200_fset_ext_catalog(x,y)     lx200_set(x,"Lo",y)
#define lx200_fset_star(x,y)            lx200_set(x,"LS",y)
#define lx200_fset_messier(x,y)         lx200_set(x,"LM",y)
#define lx200_fset_ext(x,y)             lx200_set(x,"LC",y)

/*A few shorthanders to make code easier to read*/
#define lx200_set_ext_ngc(x)            lx200_fset_ext_catalog(x,"0")
#define lx200_set_ext_ic(x)             lx200_fset_ext_catalog(x,"1")
#define lx200_set_ext_ugc(x)            lx200_fset_ext_catalog(x,"2")

/*Commands that the scope doesn't return a reply*/
#define lx200_cmd(x,y)                  lx200_send_command(x,y)
#define lx200_toggle_format(x)          lx200_cmd(x,"U")
#define lx200_move_north(x)             lx200_cmd(x,"Mn")
#define lx200_move_south(x)             lx200_cmd(x,"Ms")
#define lx200_move_east(x)              lx200_cmd(x,"Me")
#define lx200_move_west(x)              lx200_cmd(x,"Mw")
#define lx200_stop_north(x)             lx200_cmd(x,"Qn")
#define lx200_stop_south(x)             lx200_cmd(x,"Qs")
#define lx200_stop_east(x)              lx200_cmd(x,"Qe")
#define lx200_stop_west(x)              lx200_cmd(x,"Qw")
#define lx200_stop_slew(x)              lx200_cmd(x,"Q")
#define lx200_set_speed_guide(x)        lx200_cmd(x,"RG")
#define lx200_set_speed_center(x)       lx200_cmd(x,"RC")
#define lx200_set_speed_find(x)         lx200_cmd(x,"RM")
#define lx200_set_speed_slew(x)         lx200_cmd(x,"RS")
#define lx200_step_quality(x)           lx200_cmd(x,"Sq")
#define lx200_find_start(x)             lx200_cmd(x,"LF")
#define lx200_find_next(x)              lx200_cmd(x,"LN")
#define lx200_find_prev(x)              lx200_cmd(x,"LB")
#define lx200_reticle_brighter(x)       lx200_cmd(x,"B+")
#define lx200_reticle_dimmer(x)         lx200_cmd(x,"B-")
#define lx200_focus_out(x)              lx200_cmd(x,"F+")
#define lx200_focus_in(x)               lx200_cmd(x,"F-")
#define lx200_focus_stop(x)             lx200_cmd(x,"FQ");
#define lx200_set_focus_fast(x)         lx200_cmd(x,"FF");
#define lx200_set_focus_slow(x)         lx200_cmd(x,"FS");
#define lx200_set_track_manual(x)       lx200_cmd(x,"TM")
#define lx200_set_track_quartz(x)       lx200_cmd(x,"TQ")
#define lx200_track_increment(x)        lx200_cmd(x,"T+")
#define lx200_track_decrement(x)        lx200_cmd(x,"T-")
#define lx200_toggle_clock_format(x)    lx200_cmd(x,"H")
#define lx200_toggle_smart_learn(x)     lx200_cmd(x,"Q1")
#define lx200_toggle_smart_update(x)    lx200_cmd(x,"Q2")
#define lx200_toggle_smart_erase(x)     lx200_cmd(x,"Q3")
#define lx200_toggle_smart_dlearn(x)    lx200_cms(x,"Q4")
#define lx200_toggle_smart_dcorrect(x)  lx200_cms(x,"Q5")

/*Format macros*/
#define lx200_format_messier(y,x)       lx200_fmt_number(y,3,'\0',x)
#define lx200_format_ngc(y,x)           lx200_fmt_number(y,4,'\0',x)
#define lx200_format_ic(y,x)            lx200_fmt_number(y,4,'\0',x)
#define lx200_format_ugc(y,x)           lx200_fmt_number(y,5,'\0',x)
#define lx200_format_date(M,D,Y,x)      lx200_fmt_time(M,D,Y,'/','/',FALSE,x)
#define lx200_format_latitude(D,M,S,x)  lx200_fmt_time(D,M,S,LX200_DEGREE,';',TRUE,x)
#define lx200_format_longitude(D,M,S,x) lx200_fmt_coord(D,M,S,LX200_DEGREE,';',TRUE,x)
#define lx200_format_GMT_offset(H,x)    lx200_fmt_hour(H,TRUE,NULL,x)
#define lx200_format_RA(H,M,S,x)        lx200_fmt_time(H,M,S,':',':',FALSE,x)
#define lx200_format_dec(D,M,S,x)       lx200_fmt_time(D,M,S,LX200_DEGREE,':',TRUE,x)
#define lx200_format_horizon(D,x)       lx200_fmt_hour(H,FALSE,LX200_DEGREE,x)
#define lx200_format_magnitude(M,x)     lx200_fmt_magnitude(M,TRUE,x)
#define lx200_format_size(y,x)          lx200_fmt_number(y,3,'\0',x)
#define lx200_format_radius(y,x)        lx200_fmt_number(y,3,'\'',x)
#define lx200_format_track_freq(y,x)    lx200_fmt_magnitude(y,FALSE,x)


/*Predecs*/
int lx200_open_scope(char *);
int lx200_close_scope(int);
int lx200_read_ok(int);
char lx200_read_one(int);
int lx200_read_two(int, char *);
int lx200_write_to_scope(int, char *);
int lx200_send_ACK(int);
int lx200_read_from_scope(int, char *);
int lx200_get_generic(int, char *, char *, char *);
int lx200_get_mode(int);
int lx200_get_site_name(int, char *, int);
int lx200_get_clock_format(int);
int lx200_goto(int);
int lx200_set_generic(int, char *, char *);
int lx200_send_command(int, char *);
int lx200_fset_date(int, char *);
int lx200_set_reticle_flash(int, int);
int lx200_obj_sync(int, char *);
int lx200_set_site_name(int, char *, int);
int lx200_goto_star(int, int, int);
int lx200_goto_RADec(int, char *, char *);
int lx200_goto_ext(int, int, int);
int lx200_set_format(int, int);
int lx200_set_filter_type(int, int);
int lx200_set_site_number(int, int);
int lx200_convert_RA(char *, int *, int *, int *);
int lx200_convert_Dec(char *, int *, int *, int *);
int lx200_map_planet_id(char *);
int lx200_fmt_number(int, int, char, char *);
int lx200_fmt_time(int, int, int, char, char, int, char *);
int lx200_fmt_coord(int, int, int, char, char, char *);
int lx200_fmt_hour(int, int, char, char *);
int lx200_fmt_magnitude(double, int, char *);
int lx200_get_lib_version(char *);
int lx200_set_lib_emulate(int, int);






/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: liblx200.h,v $ $Date: 2001/12/09 08:18:09 $ $Revision: 1.3 $ $Name:  $
 */
