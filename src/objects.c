typedef struct {
     PyObject_HEAD
     Now now;
} ObserverObject;

typedef struct {
     PyObject_HEAD
     Now now;			/* cache of last observer */
     Obj obj;			/* the ephemeris object */
     RiseSet riset;		/* rising and setting */
     unsigned now_valid : 2;	/* 0=no, 1=mjd and epoch only, 2=all */
     unsigned obj_valid : 2;	/* 0=no, 1=geocentric only, 2=all */
     unsigned riset_valid : 2;	/* 0=no, 2=yes */
} BodyObject;

typedef struct {
     PyObject_HEAD
     unsigned now_valid : 2;	/* 0=no, 1=mjd and epoch only, 2=all */
     unsigned obj_valid : 2;	/* 0=no, 1=geocentric only, 2=all */
     unsigned riset_valid : 2;	/* 0=no, 2=yes */
     Now now;			/* cache of last observer */
     Obj obj;			/* the ephemeris object */
     RiseSet riset;		/* rising and setting */
     unsigned llibration_valid : 2; /* 0=no, nonzero=yes */
     unsigned moon_colong_valid : 2; /* 0=no, nonzero=yes */
     double llat, llon;		/* libration */
     double c, k, s;		/* co-longitude and illumination */
} MoonObject;

typedef struct {
     PyObject_HEAD
     unsigned now_valid : 2;	/* 0=no, 1=mjd and epoch only, 2=all */
     unsigned obj_valid : 2;	/* 0=no, 1=geocentric only, 2=all */
     unsigned riset_valid : 2;	/* 0=no, 2=yes */
     Now now;			/* cache of last observer */
     Obj obj;			/* the ephemeris object */
     RiseSet riset;		/* rising and setting */
     unsigned satrings_valid : 2; /* 0=no, nonzero=yes */
     double etilt, stilt;	/* tilt of rings */
} SaturnObject;
