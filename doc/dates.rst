

Like angles, dates are magic: both strings and numbers.

 >>> import ephem
 >>> d = ephem.date('1984/05/30 12:23:45.13')
 >>> print d
 1984/5/30 12:23:45

 >>> isinstance(d, float)
 True
 >>> print 'Behind the string %s is the number %f.' % (d, d)
 Behind the string 1984/5/30 12:23:45 is the number 30831.016495.



 >>> d.datetime()
 datetime.datetime(1984, 5, 30, 12, 23, 45, 130000)


Can have it give you pieces of date.

How to construct?

 >>> print ephem.date(30831.0159723)     # float
 1984/5/30 12:23:00
 >>> print ephem.date(30831)             # integer
 1984/5/30 12:00:00
 >>> print ephem.date('1984/05/30 12:23') # string
 1984/5/30 12:23:00
 >>> print ephem.date('1984/05/30')       # partial string
 1984/5/30 00:00:00

 >>> from datetime import date, datetime
 >>> print ephem.date(date(1984, 5, 30))
 1984/5/30 00:00:00
 >>> print ephem.date(datetime(1984, 5, 30, 12, 23))
 1984/5/30 12:23:00

So there.  Based on:

 >>> print ephem.date(0)
 1899/12/31 12:00:00

Which is not very interesting; but this means you can add or subtract
a number of days:

 >>> print ephem.date(d - 2)
 1984/5/28 12:23:45
 >>> print ephem.date(d + 2)
 1984/6/1 12:23:45

We also provide constants for a minute and a second.

 >>> print ephem.date(d - ephem.minute)
 1984/5/30 12:22:45
 >>> print ephem.date(d - ephem.second)
 1984/5/30 12:23:44
