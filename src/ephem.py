# This file was created automatically by SWIG.
import ephemc
class ObjPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self,ephemc=ephemc):
        if self.thisown == 1 :
            ephemc.delete_Obj(self)
    def __setattr__(self,name,value):
        if name == "any" :
            ephemc.Obj_any_set(self,value.this)
            return
        if name == "anyss" :
            ephemc.Obj_anyss_set(self,value.this)
            return
        if name == "pl" :
            ephemc.Obj_pl_set(self,value.this)
            return
        if name == "f" :
            ephemc.Obj_f_set(self,value.this)
            return
        if name == "e" :
            ephemc.Obj_e_set(self,value.this)
            return
        if name == "h" :
            ephemc.Obj_h_set(self,value.this)
            return
        if name == "p" :
            ephemc.Obj_p_set(self,value.this)
            return
        if name == "es" :
            ephemc.Obj_es_set(self,value.this)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "any" : 
            return anyPtr(ephemc.Obj_any_get(self))
        if name == "anyss" : 
            return anyssPtr(ephemc.Obj_anyss_get(self))
        if name == "pl" : 
            return plPtr(ephemc.Obj_pl_get(self))
        if name == "f" : 
            return fPtr(ephemc.Obj_f_get(self))
        if name == "e" : 
            return ePtr(ephemc.Obj_e_get(self))
        if name == "h" : 
            return hPtr(ephemc.Obj_h_get(self))
        if name == "p" : 
            return pPtr(ephemc.Obj_p_get(self))
        if name == "es" : 
            return esPtr(ephemc.Obj_es_get(self))
        raise AttributeError,name
    def __repr__(self):
        return "<C Obj instance at %s>" % (self.this,)
class Obj(ObjPtr):
    def __init__(self,*_args,**_kwargs):
        self.this = apply(ephemc.new_Obj,_args,_kwargs)
        self.thisown = 1




class anyPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __setattr__(self,name,value):
        if name == "type" :
            ephemc.any_type_set(self,value)
            return
        if name == "flags" :
            ephemc.any_flags_set(self,value)
            return
        if name == "name" :
            ephemc.any_name_set(self,value)
            return
        if name == "ra" :
            ephemc.any_ra_set(self,value)
            return
        if name == "dec" :
            ephemc.any_dec_set(self,value)
            return
        if name == "gaera" :
            ephemc.any_gaera_set(self,value)
            return
        if name == "gaedec" :
            ephemc.any_gaedec_set(self,value)
            return
        if name == "azimuth" :
            ephemc.any_azimuth_set(self,value)
            return
        if name == "altitude" :
            ephemc.any_altitude_set(self,value)
            return
        if name == "elongation" :
            ephemc.any_elongation_set(self,value)
            return
        if name == "size" :
            ephemc.any_size_set(self,value)
            return
        if name == "magnitude" :
            ephemc.any_magnitude_set(self,value)
            return
        if name == "age" :
            ephemc.any_age_set(self,value)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "type" : 
            return ephemc.any_type_get(self)
        if name == "flags" : 
            return ephemc.any_flags_get(self)
        if name == "name" : 
            return ephemc.any_name_get(self)
        if name == "ra" : 
            return ephemc.any_ra_get(self)
        if name == "dec" : 
            return ephemc.any_dec_get(self)
        if name == "gaera" : 
            return ephemc.any_gaera_get(self)
        if name == "gaedec" : 
            return ephemc.any_gaedec_get(self)
        if name == "azimuth" : 
            return ephemc.any_azimuth_get(self)
        if name == "altitude" : 
            return ephemc.any_altitude_get(self)
        if name == "elongation" : 
            return ephemc.any_elongation_get(self)
        if name == "size" : 
            return ephemc.any_size_get(self)
        if name == "magnitude" : 
            return ephemc.any_magnitude_get(self)
        if name == "age" : 
            return ephemc.any_age_get(self)
        raise AttributeError,name
    def __repr__(self):
        return "<C any instance at %s>" % (self.this,)
class any(anyPtr):
    def __init__(self,this):
        self.this = this




class anyssPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __setattr__(self,name,value):
        if name == "sunDistance" :
            ephemc.anyss_sunDistance_set(self,value)
            return
        if name == "earthDistance" :
            ephemc.anyss_earthDistance_set(self,value)
            return
        if name == "helioLongitude" :
            ephemc.anyss_helioLongitude_set(self,value)
            return
        if name == "helioLatitude" :
            ephemc.anyss_helioLatitude_set(self,value)
            return
        if name == "phase" :
            ephemc.anyss_phase_set(self,value)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "sunDistance" : 
            return ephemc.anyss_sunDistance_get(self)
        if name == "earthDistance" : 
            return ephemc.anyss_earthDistance_get(self)
        if name == "helioLongitude" : 
            return ephemc.anyss_helioLongitude_get(self)
        if name == "helioLatitude" : 
            return ephemc.anyss_helioLatitude_get(self)
        if name == "phase" : 
            return ephemc.anyss_phase_get(self)
        raise AttributeError,name
    def __repr__(self):
        return "<C anyss instance at %s>" % (self.this,)
class anyss(anyssPtr):
    def __init__(self,this):
        self.this = this




class plPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __setattr__(self,name,value):
        if name == "code" :
            ephemc.pl_code_set(self,value)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "code" : 
            return ephemc.pl_code_get(self)
        raise AttributeError,name
    def __repr__(self):
        return "<C pl instance at %s>" % (self.this,)
class pl(plPtr):
    def __init__(self,this):
        self.this = this




class fPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __setattr__(self,name,value):
        if name == "class" :
            ephemc.f_class_set(self,value)
            return
        if name == "spectral" :
            ephemc.f_spectral_set(self,value)
            return
        if name == "ratio" :
            ephemc.f_ratio_set(self,value)
            return
        if name == "positionAngle" :
            ephemc.f_positionAngle_set(self,value)
            return
        if name == "epoch" :
            ephemc.f_epoch_set(self,value)
            return
        if name == "ra" :
            ephemc.f_ra_set(self,value)
            return
        if name == "dec" :
            ephemc.f_dec_set(self,value)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "class" : 
            return ephemc.f_class_get(self)
        if name == "spectral" : 
            return ephemc.f_spectral_get(self)
        if name == "ratio" : 
            return ephemc.f_ratio_get(self)
        if name == "positionAngle" : 
            return ephemc.f_positionAngle_get(self)
        if name == "epoch" : 
            return ephemc.f_epoch_get(self)
        if name == "ra" : 
            return ephemc.f_ra_get(self)
        if name == "dec" : 
            return ephemc.f_dec_get(self)
        raise AttributeError,name
    def __repr__(self):
        return "<C f instance at %s>" % (self.this,)
class f(fPtr):
    def __init__(self,this):
        self.this = this




class ePtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __setattr__(self,name,value):
        if name == "inc" :
            ephemc.e_inc_set(self,value)
            return
        if name == "Omega" :
            ephemc.e_Omega_set(self,value)
            return
        if name == "omega" :
            ephemc.e_omega_set(self,value)
            return
        if name == "a" :
            ephemc.e_a_set(self,value)
            return
        if name == "e" :
            ephemc.e_e_set(self,value)
            return
        if name == "M" :
            ephemc.e_M_set(self,value)
            return
        if name == "size" :
            ephemc.e_size_set(self,value)
            return
        if name == "cepoch" :
            ephemc.e_cepoch_set(self,value)
            return
        if name == "epoch" :
            ephemc.e_epoch_set(self,value)
            return
        if name == "magnitude" :
            ephemc.e_magnitude_set(self,value.this)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "inc" : 
            return ephemc.e_inc_get(self)
        if name == "Omega" : 
            return ephemc.e_Omega_get(self)
        if name == "omega" : 
            return ephemc.e_omega_get(self)
        if name == "a" : 
            return ephemc.e_a_get(self)
        if name == "e" : 
            return ephemc.e_e_get(self)
        if name == "M" : 
            return ephemc.e_M_get(self)
        if name == "size" : 
            return ephemc.e_size_get(self)
        if name == "cepoch" : 
            return ephemc.e_cepoch_get(self)
        if name == "epoch" : 
            return ephemc.e_epoch_get(self)
        if name == "magnitude" : 
            return MagPtr(ephemc.e_magnitude_get(self))
        raise AttributeError,name
    def __repr__(self):
        return "<C e instance at %s>" % (self.this,)
class e(ePtr):
    def __init__(self,this):
        self.this = this




class hPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __setattr__(self,name,value):
        if name == "epoch" :
            ephemc.h_epoch_set(self,value)
            return
        if name == "ep" :
            ephemc.h_ep_set(self,value)
            return
        if name == "inc" :
            ephemc.h_inc_set(self,value)
            return
        if name == "Omega" :
            ephemc.h_Omega_set(self,value)
            return
        if name == "omega" :
            ephemc.h_omega_set(self,value)
            return
        if name == "e" :
            ephemc.h_e_set(self,value)
            return
        if name == "qp" :
            ephemc.h_qp_set(self,value)
            return
        if name == "g" :
            ephemc.h_g_set(self,value)
            return
        if name == "k" :
            ephemc.h_k_set(self,value)
            return
        if name == "size" :
            ephemc.h_size_set(self,value)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "epoch" : 
            return ephemc.h_epoch_get(self)
        if name == "ep" : 
            return ephemc.h_ep_get(self)
        if name == "inc" : 
            return ephemc.h_inc_get(self)
        if name == "Omega" : 
            return ephemc.h_Omega_get(self)
        if name == "omega" : 
            return ephemc.h_omega_get(self)
        if name == "e" : 
            return ephemc.h_e_get(self)
        if name == "qp" : 
            return ephemc.h_qp_get(self)
        if name == "g" : 
            return ephemc.h_g_get(self)
        if name == "k" : 
            return ephemc.h_k_get(self)
        if name == "size" : 
            return ephemc.h_size_get(self)
        raise AttributeError,name
    def __repr__(self):
        return "<C h instance at %s>" % (self.this,)
class h(hPtr):
    def __init__(self,this):
        self.this = this




class pPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __setattr__(self,name,value):
        if name == "epoch" :
            ephemc.p_epoch_set(self,value)
            return
        if name == "ep" :
            ephemc.p_ep_set(self,value)
            return
        if name == "inc" :
            ephemc.p_inc_set(self,value)
            return
        if name == "qp" :
            ephemc.p_qp_set(self,value)
            return
        if name == "omega" :
            ephemc.p_omega_set(self,value)
            return
        if name == "Omega" :
            ephemc.p_Omega_set(self,value)
            return
        if name == "g" :
            ephemc.p_g_set(self,value)
            return
        if name == "k" :
            ephemc.p_k_set(self,value)
            return
        if name == "size" :
            ephemc.p_size_set(self,value)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "epoch" : 
            return ephemc.p_epoch_get(self)
        if name == "ep" : 
            return ephemc.p_ep_get(self)
        if name == "inc" : 
            return ephemc.p_inc_get(self)
        if name == "qp" : 
            return ephemc.p_qp_get(self)
        if name == "omega" : 
            return ephemc.p_omega_get(self)
        if name == "Omega" : 
            return ephemc.p_Omega_get(self)
        if name == "g" : 
            return ephemc.p_g_get(self)
        if name == "k" : 
            return ephemc.p_k_get(self)
        if name == "size" : 
            return ephemc.p_size_get(self)
        raise AttributeError,name
    def __repr__(self):
        return "<C p instance at %s>" % (self.this,)
class p(pPtr):
    def __init__(self,this):
        self.this = this




class esPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __setattr__(self,name,value):
        if name == "epoch" :
            ephemc.es_epoch_set(self,value)
            return
        if name == "n" :
            ephemc.es_n_set(self,value)
            return
        if name == "inc" :
            ephemc.es_inc_set(self,value)
            return
        if name == "raan" :
            ephemc.es_raan_set(self,value)
            return
        if name == "e" :
            ephemc.es_e_set(self,value)
            return
        if name == "ap" :
            ephemc.es_ap_set(self,value)
            return
        if name == "M" :
            ephemc.es_M_set(self,value)
            return
        if name == "decay" :
            ephemc.es_decay_set(self,value)
            return
        if name == "orbitNumber" :
            ephemc.es_orbitNumber_set(self,value)
            return
        if name == "elevation" :
            ephemc.es_elevation_set(self,value)
            return
        if name == "range" :
            ephemc.es_range_set(self,value)
            return
        if name == "rangev" :
            ephemc.es_rangev_set(self,value)
            return
        if name == "sublatitude" :
            ephemc.es_sublatitude_set(self,value)
            return
        if name == "sublongitude" :
            ephemc.es_sublongitude_set(self,value)
            return
        if name == "isEclipsed" :
            ephemc.es_isEclipsed_set(self,value)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "epoch" : 
            return ephemc.es_epoch_get(self)
        if name == "n" : 
            return ephemc.es_n_get(self)
        if name == "inc" : 
            return ephemc.es_inc_get(self)
        if name == "raan" : 
            return ephemc.es_raan_get(self)
        if name == "e" : 
            return ephemc.es_e_get(self)
        if name == "ap" : 
            return ephemc.es_ap_get(self)
        if name == "M" : 
            return ephemc.es_M_get(self)
        if name == "decay" : 
            return ephemc.es_decay_get(self)
        if name == "orbitNumber" : 
            return ephemc.es_orbitNumber_get(self)
        if name == "elevation" : 
            return ephemc.es_elevation_get(self)
        if name == "range" : 
            return ephemc.es_range_get(self)
        if name == "rangev" : 
            return ephemc.es_rangev_get(self)
        if name == "sublatitude" : 
            return ephemc.es_sublatitude_get(self)
        if name == "sublongitude" : 
            return ephemc.es_sublongitude_get(self)
        if name == "isEclipsed" : 
            return ephemc.es_isEclipsed_get(self)
        raise AttributeError,name
    def __repr__(self):
        return "<C es instance at %s>" % (self.this,)
class es(esPtr):
    def __init__(self,this):
        self.this = this




class MagPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __setattr__(self,name,value):
        if name == "m1" :
            ephemc.Mag_m1_set(self,value)
            return
        if name == "m2" :
            ephemc.Mag_m2_set(self,value)
            return
        if name == "whichm" :
            ephemc.Mag_whichm_set(self,value)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "m1" : 
            return ephemc.Mag_m1_get(self)
        if name == "m2" : 
            return ephemc.Mag_m2_get(self)
        if name == "whichm" : 
            return ephemc.Mag_whichm_get(self)
        raise AttributeError,name
    def __repr__(self):
        return "<C Mag instance at %s>" % (self.this,)
class Mag(MagPtr):
    def __init__(self,this):
        self.this = this




class CircumstancePtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self,ephemc=ephemc):
        if self.thisown == 1 :
            ephemc.delete_Circumstance(self)
    def __setattr__(self,name,value):
        if name == "mjd" :
            ephemc.Circumstance_mjd_set(self,value)
            return
        if name == "latitude" :
            ephemc.Circumstance_latitude_set(self,value)
            return
        if name == "longitude" :
            ephemc.Circumstance_longitude_set(self,value)
            return
        if name == "timezone" :
            ephemc.Circumstance_timezone_set(self,value)
            return
        if name == "temperature" :
            ephemc.Circumstance_temperature_set(self,value)
            return
        if name == "pressure" :
            ephemc.Circumstance_pressure_set(self,value)
            return
        if name == "elevation" :
            ephemc.Circumstance_elevation_set(self,value)
            return
        if name == "sunDip" :
            ephemc.Circumstance_sunDip_set(self,value)
            return
        if name == "epoch" :
            ephemc.Circumstance_epoch_set(self,value)
            return
        if name == "tzname" :
            ephemc.Circumstance_tzname_set(self,value)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "mjd" : 
            return ephemc.Circumstance_mjd_get(self)
        if name == "latitude" : 
            return ephemc.Circumstance_latitude_get(self)
        if name == "longitude" : 
            return ephemc.Circumstance_longitude_get(self)
        if name == "timezone" : 
            return ephemc.Circumstance_timezone_get(self)
        if name == "temperature" : 
            return ephemc.Circumstance_temperature_get(self)
        if name == "pressure" : 
            return ephemc.Circumstance_pressure_get(self)
        if name == "elevation" : 
            return ephemc.Circumstance_elevation_get(self)
        if name == "sunDip" : 
            return ephemc.Circumstance_sunDip_get(self)
        if name == "epoch" : 
            return ephemc.Circumstance_epoch_get(self)
        if name == "tzname" : 
            return ephemc.Circumstance_tzname_get(self)
        raise AttributeError,name
    def __repr__(self):
        return "<C Circumstance instance at %s>" % (self.this,)
class Circumstance(CircumstancePtr):
    def __init__(self,*_args,**_kwargs):
        self.this = apply(ephemc.new_Circumstance,_args,_kwargs)
        self.thisown = 1




class RiseSetPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self,ephemc=ephemc):
        if self.thisown == 1 :
            ephemc.delete_RiseSet(self)
    def __setattr__(self,name,value):
        if name == "flags" :
            ephemc.RiseSet_flags_set(self,value)
            return
        if name == "riseTime" :
            ephemc.RiseSet_riseTime_set(self,value)
            return
        if name == "riseAzimuth" :
            ephemc.RiseSet_riseAzimuth_set(self,value)
            return
        if name == "transitTime" :
            ephemc.RiseSet_transitTime_set(self,value)
            return
        if name == "transitAltitude" :
            ephemc.RiseSet_transitAltitude_set(self,value)
            return
        if name == "setTime" :
            ephemc.RiseSet_setTime_set(self,value)
            return
        if name == "setAzimuth" :
            ephemc.RiseSet_setAzimuth_set(self,value)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "flags" : 
            return ephemc.RiseSet_flags_get(self)
        if name == "riseTime" : 
            return ephemc.RiseSet_riseTime_get(self)
        if name == "riseAzimuth" : 
            return ephemc.RiseSet_riseAzimuth_get(self)
        if name == "transitTime" : 
            return ephemc.RiseSet_transitTime_get(self)
        if name == "transitAltitude" : 
            return ephemc.RiseSet_transitAltitude_get(self)
        if name == "setTime" : 
            return ephemc.RiseSet_setTime_get(self)
        if name == "setAzimuth" : 
            return ephemc.RiseSet_setAzimuth_get(self)
        raise AttributeError,name
    def __repr__(self):
        return "<C RiseSet instance at %s>" % (self.this,)
class RiseSet(RiseSetPtr):
    def __init__(self,*_args,**_kwargs):
        self.this = apply(ephemc.new_RiseSet,_args,_kwargs)
        self.thisown = 1






#-------------- FUNCTION WRAPPERS ------------------

formatDate = ephemc.formatDate

scanDate = ephemc.scanDate

rescanDate = ephemc.rescanDate

fromGregorian = ephemc.fromGregorian

toGregorian = ephemc.toGregorian

toDayOfWeek = ephemc.toDayOfWeek

toDaysInMonth = ephemc.toDaysInMonth

toYear = ephemc.toYear

fromYear = ephemc.fromYear

toNearestSecond = ephemc.toNearestSecond

toDay = ephemc.toDay

toHourOfDay = ephemc.toHourOfDay

formatSexagesimal = ephemc.formatSexagesimal

scanSexagesimal = ephemc.scanSexagesimal

rescanSexagesimal = ephemc.rescanSexagesimal

deltaRA = ephemc.deltaRA

degrad = ephemc.degrad

raddeg = ephemc.raddeg

hrdeg = ephemc.hrdeg

deghr = ephemc.deghr

hrrad = ephemc.hrrad

radhr = ephemc.radhr

formatHours = ephemc.formatHours

formatDegrees = ephemc.formatDegrees

formatDay = ephemc.formatDay

formatTime = ephemc.formatTime

describe = ephemc.describe

isDeepsky = ephemc.isDeepsky

computeLocation = ephemc.computeLocation

computeSeparation = ephemc.computeSeparation

computeSiderealTime = ephemc.computeSiderealTime

computeRiseSet = ephemc.computeRiseSet

computeTwilight = ephemc.computeTwilight

constellation = ephemc.constellation

constellationName = ephemc.constellationName

scanDB = ephemc.scanDB

formatDB = ephemc.formatDB

ap_as = ephemc.ap_as

as_ap = ephemc.as_ap

deltat = ephemc.deltat

heliocorr = ephemc.heliocorr

mm_mjed = ephemc.mm_mjed



#-------------- VARIABLE WRAPPERS ------------------

MetersPerAU = ephemc.MetersPerAU
LightTimeAU = ephemc.LightTimeAU
EarthRadius = ephemc.EarthRadius
MoonRadius = ephemc.MoonRadius
SunRadius = ephemc.SunRadius
FeetPerMeter = ephemc.FeetPerMeter
MJD0 = ephemc.MJD0
J2000 = ephemc.J2000
SecondsPerDay = ephemc.SecondsPerDay
MaxName = ephemc.MaxName
UNDEFOBJ = ephemc.UNDEFOBJ
FIXED = ephemc.FIXED
ELLIPTICAL = ephemc.ELLIPTICAL
HYPERBOLIC = ephemc.HYPERBOLIC
PARABOLIC = ephemc.PARABOLIC
EARTHSAT = ephemc.EARTHSAT
PLANET = ephemc.PLANET
NOBJTYPES = ephemc.NOBJTYPES
MERCURY = ephemc.MERCURY
VENUS = ephemc.VENUS
MARS = ephemc.MARS
JUPITER = ephemc.JUPITER
SATURN = ephemc.SATURN
URANUS = ephemc.URANUS
NEPTUNE = ephemc.NEPTUNE
PLUTO = ephemc.PLUTO
SUN = ephemc.SUN
MOON = ephemc.MOON
SRSCALE = ephemc.SRSCALE
PASCALE = ephemc.PASCALE
NCLASSES = ephemc.NCLASSES
MAG_HG = ephemc.MAG_HG
MAG_gk = ephemc.MAG_gk
EOD = ephemc.EOD
RS_NORISE = ephemc.RS_NORISE
RS_NOSET = ephemc.RS_NOSET
RS_NOTRANS = ephemc.RS_NOTRANS
RS_CIRCUMPOLAR = ephemc.RS_CIRCUMPOLAR
RS_NEVERUP = ephemc.RS_NEVERUP
RS_ERROR = ephemc.RS_ERROR
RS_RISERR = ephemc.RS_RISERR
RS_SETERR = ephemc.RS_SETERR
RS_TRANSERR = ephemc.RS_TRANSERR
topocentric = ephemc.topocentric
geocentric = ephemc.geocentric
MDY = ephemc.MDY
YMD = ephemc.YMD
DMY = ephemc.DMY
