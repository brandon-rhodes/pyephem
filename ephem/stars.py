"""A Catalog of 115 Bright Stars

The function star() will create and return a PyEphem Body representing
the star whose name you provide.

>>> star('Aldebaran')

Data is adapted from the version of the Hipparcos star catalog at:
ftp://adc.gsfc.nasa.gov/pub/adc/archives/catalogs/1/1239/hip_main.dat.gz
Of the thousand brighest Hipparcos stars, those with proper names
registered at http://simbad.u-strasbg.fr/simbad/ were chosen.

Sources of data:

Star names:
"IAU Catalog of Star Names (IAU-CSN)":
https://www.iau.org/science/scientific_bodies/working_groups/280/
Which links to:
http://www.pas.rochester.edu/~emamajek/WGSN/IAU-CSN.txt

Star names and navigation data:
Her Majesty's Nautical Almanac Office Publications ('Almanac'):
    NP314-18 "The Nautical Almanac" (2017).
    DP330 "NavPac and Compact Data" (2015).
    NP303(AP3270) "Rapid Sight Reduction Tables for Navigation" (2012).

Star astronomical data ('the catalogue'):
https://heasarc.gsfc.nasa.gov/cgi-bin/W3Browse/w3hdprods.pl
 
There are some mismatches of star names between the almanacs and this database. In particular:
    In early digital publications of DP330 'Fomalhaut' is misspelled as 'Formalhaut'.
    Almanac/IAU-CSN name 'Eltanin' is formally known as 'Gamma Draconis' in the catalogue.
    Almanac/IAU-CSN name 'Hadar' is formally known as 'Beta Centauri' in the catalogue.
    Almanac/IAU-CSN name 'Rigil Kentaurus' is formally known as 'Alpha Centauri A' in the catalogue.
"""

db = """\
Sirrah,f|S|B9,0:08:23.2|135.68,29:05:27|-162.95,2.07,2000,0
Caph,f|S|F2,0:09:10.1|523.39,59:09:01|-180.42,2.28,2000,0
Algenib,f|S|B2,0:13:14.2|4.7,15:11:01|-8.24,2.83,2000,0
Schedar,f|S|K0,0:40:30.4|50.36,56:32:15|-32.17,2.24,2000,0
Mirach,f|S|M0,1:09:43.8|175.59,35:37:15|-112.23,2.07,2000,0
Achernar,f|S|B3,1:37:42.8|88.02,-57:14:12|-40.08,0.45,2000,0
Almach,f|S|B8,2:03:53.9|43.08,42:19:48|-50.85,2.10,2000,0
Hamal,f|S|K2,2:07:10.3|190.73,23:27:46|-145.77,2.01,2000,0
Polaris,f|S|F7,2:31:47.1|44.22,89:15:51|-11.74,1.97,2000,0
Menkar,f|S|M2,3:02:16.8|-11.81,4:05:24|-78.76,2.54,2000,0
Algol,f|S|B8,3:08:10.1|2.39,40:57:20|-1.44,2.09,2000,0
Electra,f|S|B6,3:44:52.5|21.55,24:06:48|-44.92,3.72,2000,0
Taygeta,f|S|B6,3:45:12.5|19.35,24:28:03|-41.63,4.30,2000,0
Maia,f|S|B8,3:45:49.6|21.09,24:22:04|-45.03,3.87,2000,0
Merope,f|S|B6,3:46:19.6|21.17,23:56:54|-42.67,4.14,2000,0
Alcyone,f|S|B7,3:47:29.1|19.35,24:06:19|-43.11,2.85,2000,0
Atlas,f|S|B8,3:49:09.7|17.77,24:03:13|-44.7,3.62,2000,0
Zaurak,f|S|M1,3:58:01.7|60.51,-13:30:30|-111.34,2.97,2000,0
Aldebaran,f|S|K5,4:35:55.2|62.78,16:30:35|-189.36,0.87,2000,0
Rigel,f|S|B8,5:14:32.3|1.87,-8:12:06|-0.56,0.18,2000,0
Capella,f|S|M1,5:16:41.3|75.52,45:59:57|-427.13,0.08,2000,0
Bellatrix,f|S|B2,5:25:07.9|-8.75,6:20:59|-13.28,1.64,2000,0
Elnath,f|S|B7,5:26:17.5|23.28,28:36:28|-174.22,1.65,2000,0
Nihal,f|S|G5,5:28:14.7|-5.03,-20:45:33|-85.92,2.81,2000,0
Mintaka,f|S|O9,5:32:00.4|1.67,-0:17:57|0.56,2.25,2000,0
Arneb,f|S|F0,5:32:43.8|3.27,-17:49:20|1.54,2.58,2000,0
Alnilam,f|S|B0,5:36:12.8|1.49,-1:12:07|-1.06,1.69,2000,0
Alnitak,f|S|O9,5:40:45.5|3.99,-1:56:33|2.54,1.74,2000,0
Saiph,f|S|B0,5:47:45.4|1.55,-9:40:11|-1.2,2.07,2000,0
Betelgeuse,f|S|M2,5:55:10.3|27.33,7:24:25|10.86,0.45,2000,0
Menkalinan,f|S|A2,5:59:31.8|-56.41,44:56:51|-0.88,1.90,2000,0
Mirzam,f|S|B1,6:22:42.0|-3.45,-17:57:21|-0.47,1.98,2000,0
Canopus,f|S|F0,6:23:57.1|19.99,-52:41:45|23.67,-0.62,2000,0
Alhena,f|S|A0,6:37:42.7|-2.04,16:23:58|-66.92,1.93,2000,0
Sirius,f|S|A0,6:45:09.3|-546.01,-16:42:47|-1223.08,-1.44,2000,0
Adara,f|S|B2,6:58:37.6|2.63,-28:58:20|2.29,1.50,2000,0
Wezen,f|S|F8,7:08:23.5|-2.75,-26:23:36|3.33,1.83,2000,0
Castor,f|S|A2,7:34:36.0|-206.33,31:53:19|-148.18,1.58,2000,0
Procyon,f|S|F5,7:39:18.5|-716.57,5:13:39|-1034.58,0.40,2000,0
Pollux,f|S|K0,7:45:19.4|-625.69,28:01:35|-45.95,1.16,2000,0
Naos,f|S|O5,8:03:35.1|-30.82,-40:00:12|16.77,2.21,2000,0
Alphard,f|S|K3,9:27:35.3|-14.49,-8:39:31|33.25,1.99,2000,0
Regulus,f|S|B7,10:08:22.5|-249.4,11:58:02|4.91,1.36,2000,0
Algieba,f|S|K0,10:19:58.2|310.77,19:50:31|-152.88,2.01,2000,0
Merak,f|S|A1,11:01:50.4|81.66,56:22:56|33.74,2.34,2000,0
Dubhe,f|S|F7,11:03:43.8|-136.46,61:45:04|-35.25,1.81,2000,0
Denebola,f|S|A3,11:49:03.9|-499.02,14:34:20|-113.78,2.14,2000,0
Phecda,f|S|A0,11:53:49.7|107.76,53:41:41|11.16,2.41,2000,0
Minkar,f|S|K2,12:10:07.5|-71.52,-22:37:11|10.55,3.02,2000,0
Megrez,f|S|A3,12:15:25.5|103.56,57:01:57|7.81,3.32,2000,0
Gienah Corvi,f|S|B8,12:15:48.5|-159.58,-17:32:31|22.31,2.58,2000,0
Mimosa,f|S|B0,12:47:43.3|-48.24,-59:41:19|-12.82,1.25,2000,0
Alioth,f|S|A0,12:54:01.6|111.74,55:57:35|-8.99,1.76,2000,0
Vindemiatrix,f|S|G8,13:02:10.8|-275.05,10:57:33|19.96,2.85,2000,0
Mizar,f|S|A2,13:23:55.4|121.23,54:55:32|-22.01,2.23,2000,0
Spica,f|S|B1,13:25:11.6|-42.5,-11:09:40|-31.73,0.98,2000,0
Alcor,f|S|A5,13:25:13.4|120.35,54:59:17|-16.94,3.99,2000,0
Alcaid,f|S|B3,13:47:32.5|-121.23,49:18:48|-15.56,1.85,2000,0
Agena,f|S|B1,14:03:49.4|-33.96,-60:22:23|-25.06,0.61,2000,0
Thuban,f|S|A0,14:04:23.4|-56.52,64:22:33|17.19,3.67,2000,0
Arcturus,f|S|K2,14:15:40.3|-1093.45,19:11:14|-1999.4,-0.05,2000,0
Izar,f|S|A0,14:44:59.3|-50.65,27:04:27|20,2.35,2000,0
Kochab,f|S|K4,14:50:42.4|-32.29,74:09:20|11.91,2.07,2000,0
Alphecca,f|S|A0,15:34:41.2|120.38,26:42:54|-89.44,2.22,2000,0
Unukalhai,f|S|K2,15:44:16.0|134.66,6:25:32|44.14,2.63,2000,0
Antares,f|S|M1,16:29:24.5|-10.16,-26:25:55|-23.21,1.06,2000,0
Rasalgethi,f|S|M5,17:14:38.9|-6.71,14:23:25|32.78,2.78,2000,0
Shaula,f|S|B1,17:33:36.5|-8.9,-37:06:13|-29.95,1.62,2000,0
Rasalhague,f|S|A5,17:34:56.0|110.08,12:33:38|-222.61,2.08,2000,0
Cebalrai,f|S|K2,17:43:28.4|-40.67,4:34:01|158.8,2.76,2000,0
Etamin,f|S|K5,17:56:36.4|-8.52,51:29:20|-23.05,2.24,2000,0
Kaus Australis,f|S|B9,18:24:10.4|-39.61,-34:23:04|-124.05,1.79,2000,0
Vega,f|S|A0,18:36:56.2|201.02,38:46:59|287.46,0.03,2000,0
Sheliak,f|S|A8,18:50:04.8|1.1,33:21:46|-4.46,3.52,2000,0
Nunki,f|S|B2,18:55:15.9|13.87,-26:17:48|-52.65,2.05,2000,0
Sulafat,f|S|B9,18:58:56.6|-2.76,32:41:22|1.77,3.25,2000,0
Arkab Prior,f|S|B9,19:22:38.3|7.31,-44:27:32|-22.43,3.96,2000,0
Arkab Posterior,f|S|F2,19:23:13.1|92.78,-44:47:59|-53.73,4.27,2000,0
Rukbat,f|S|B8,19:23:53.2|32.67,-40:36:56|-120.81,3.96,2000,0
Albereo,f|S|K3,19:30:43.3|-7.09,27:57:35|-5.63,3.05,2000,0
Tarazed,f|S|K3,19:46:15.6|15.72,10:36:48|-3.08,2.72,2000,0
Altair,f|S|A7,19:50:46.7|536.82,8:52:03|385.54,0.76,2000,0
Alshain,f|S|G8,19:55:18.8|46.35,6:24:29|-481.32,3.71,2000,0
Sadr,f|S|F8,20:22:13.7|2.43,40:15:24|-0.93,2.23,2000,0
Peacock,f|S|B2,20:25:38.9|7.71,-56:44:06|-86.15,1.94,2000,0
Deneb,f|S|A2,20:41:25.9|1.56,45:16:49|1.55,1.25,2000,0
Alderamin,f|S|A7,21:18:34.6|149.91,62:35:08|48.27,2.45,2000,0
Alfirk,f|S|B2,21:28:39.6|12.6,70:33:39|8.73,3.23,2000,0
Enif,f|S|K2,21:44:11.1|30.02,9:52:30|1.38,2.38,2000,0
Sadalmelik,f|S|G2,22:05:47.0|17.9,-0:19:11|-9.93,2.95,2000,0
Alnair,f|S|B7,22:08:13.9|127.6,-46:57:38|-147.91,1.73,2000,0
Fomalhaut,f|S|A3,22:57:38.8|329.22,-29:37:19|-164.22,1.17,2000,0
Scheat,f|S|M2,23:03:46.3|187.76,28:04:57|137.61,2.44,2000,0
Markab,f|S|B9,23:04:45.6|61.1,15:12:19|-42.56,2.49,2000,0
Acamar,f|S|A4,02 58 15.7156|-53.53,-40 18 17.046|25.71,2.88,2000,0
Acrux,f|S|B0,12 26 35.9414|-35.37,-63 05 56.601|-14.73,0.77,2000,0
Adhara,f|S|B2,06 58 37.5467|2.63,-28 58 19.522|2.29,1.50,2000,0
Alkaid,f|S|B3,13 47 32.5461|-121.23,+49 18 47.890|-15.56,1.85,2000,0
Alpheratz,f|S|B9,00 08 23.1680|135.68,+29 05 26.981|-162.95,2.07,2000,0
Ankaa,f|S|K0,00 26 16.8674|232.76,-42 18 18.439|-353.64,2.40,2000,0
Atria,f|S|K2,16 48 39.8658|17.85,-69 01 39.486|-32.92,1.91,2000,0
Avior,f|S|K3,08 22 30.8647|-25.34,-59 30 34.338|22.72,1.86,2000,0
Diphda,f|S|K0,00 43 35.2283|232.79,-17 59 12.063|32.71,2.04,2000,0
Eltanin,f|S|K5,17 56 36.3779|-8.52,+51 29 20.224|-23.05,2.24,2000,0
Formalhaut,f|S|A3,22 57 38.8256|329.22,-29 37 18.613|-164.22,1.17,2000,0
Gacrux,f|S|M4,12 31 09.9293|27.94,-57 06 45.249|-264.33,1.59,2000,0
Gienah,f|S|B8,12 15 48.4678|-159.58,-17 32 31.141|22.31,2.58,2000,0
Hadar,f|S|B1,14 03 49.4446|-33.96,-60 22 22.722|-25.06,0.61,2000,0
Menkent,f|S|K0,14 06 41.3247|-519.29,-36 22 07.305|-517.87,2.06,2000,0
Miaplacidus,f|S|A2,09 13 12.2408|-157.66,-69 43 02.901|108.91,1.67,2000,0
Mirfak,f|S|F5,03 24 19.3485|24.11,+49 51 40.474|-26.01,1.79,2000,0
Rigil Kentaurus,f|S|G2,14 39 36.4958|-3678.19,-60 50 02.310|481.84,-0.01,2000,0
Sabik,f|S|A2,17 10 22.6624|41.16,-15 43 30.531|97.65,2.43,2000,0
Suhail,f|S|K4,09 07 59.7771|-23.21,-43 25 57.447|14.28,2.23,2000,0
Zubenelgenubi,f|S|A3,14 50 52.7773|-105.69,-16 02 29.798|-69.00,2.75,2000,0
"""
# Rigil Kentaurus
# ---------------
# There is some uncertainty about 'Rigil Kentaurus', presumably because of the
# high proper motion of this star.
#
# Original data from https://heasarc.gsfc.nasa.gov/cgi-bin/W3Browse/w3hdprods.pl
# is this:
# |name     |ra           |dec          |pm_ra   |pm_dec  |spect_type|vmag |astrom_ref_dbl|survey_star|_Search_Offset          |
# |HIP 71683|14 39 40.8985|-60 50 06.529|-3678.19|  481.84|G2V       |-0.01|A             |S          |0.541 (Alpha Centauri A)|
#
# This translates into this database line:
# 'Rigil Kentaurus,f|S|G2,14 39 40.8985|-3678.19,-60 50 06.529|481.84,-0.01,2000,0'
# But this fails the test when comparing with data in NP314-18 by around
# 1.1 minutes of arc.
#
# Substituting the RA, Dec. of 219.902066, -60.833975 from
# http://www.pas.rochester.edu/~emamajek/WGSN/IAU-CSN.txt
# With the database line:
# 'Rigil Kentaurus,f|S|G2,14 39 36.4958|-3678.19,-60 50 02.310|481.84,-0.01,2000,0'
# The test then passes against NP314-18 passes.
#
# I have added a test from the Nautical Almanac 2002 and that also passes with the
# second database line but fails with the first.
#
# Conclusion:
# This database line fails tests:
# Rigil Kentaurus,f|S|G2,14 39 40.8985|-3678.19,-60 50 06.529|481.84,-0.01,2000,0
# This database line passes tests:
# Rigil Kentaurus,f|S|G2,14 39 36.4958|-3678.19,-60 50 02.310|481.84,-0.01,2000,0

stars = {}

def build_stars():
    global stars
    import ephem
    for line in db.strip().split('\n'):
        star = ephem.readdb(line)
        stars[star.name] = star

build_stars()
del build_stars

def star(name, *args, **kwargs):
    star = stars[name].copy()
    if args or kwargs:
        star.compute(*args, **kwargs)
    return star

# The 57 navigational stars by number
STAR_NUMBER_NAME = {
    7 : "Acamar",
    5 : "Achernar",
    30 : "Acrux",
    19 : "Adhara",
    10 : "Aldebaran",
    32 : "Alioth",
    34 : "Alkaid",
    55 : "Alnair",
    15 : "Alnilam",
    25 : "Alphard",
    41 : "Alphecca",
    1 : "Alpheratz",
    51 : "Altair",
    2 : "Ankaa",
    42 : "Antares",
    37 : "Arcturus",
    43 : "Atria",
    22 : "Avior",
    13 : "Bellatrix",
    16 : "Betelgeuse",
    17 : "Canopus",
    12 : "Capella",
    53 : "Deneb",
    28 : "Denebola",
    4 : "Diphda",
    27 : "Dubhe",
    14 : "Elnath",
    47 : "Eltanin",
    54 : "Enif",
    56 : "Formalhaut",
    31 : "Gacrux",
    29 : "Gienah",
    35 : "Hadar",
    6 : "Hamal",
    48 : "Kaus Australis",
    40 : "Kochab",
    57 : "Markab",
    8 : "Menkar",
    36 : "Menkent",
    24 : "Miaplacidus",
    9 : "Mirfak",
    50 : "Nunki",
    52 : "Peacock",
    21 : "Pollux",
    20 : "Procyon",
    46 : "Rasalhague",
    26 : "Regulus",
    11 : "Rigel",
    38 : "Rigil Kentaurus",
    44 : "Sabik",
    3 : "Schedar",
    45 : "Shaula",
    18 : "Sirius",
    33 : "Spica",
    23 : "Suhail" ,
    49 : "Vega",
    39 : "Zubenelgenubi",
}

# The 57 navigational stars reverse lookup, {name : number, ...}
STAR_NAME_NUMBER = {}
for k, v in STAR_NUMBER_NAME.items():
    assert v not in STAR_NAME_NUMBER
    STAR_NAME_NUMBER[v] = k

