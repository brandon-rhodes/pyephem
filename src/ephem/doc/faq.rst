
 >>> import ephem

**Q:**
 Why does an ``Observer`` not allow assignment to arbitrary attributes?
 I wanted to remember which of my friends lived in which cities,
 but attempting to set a ``friend`` attribute caused an exception:

 >>> boston = ephem.Observer()
 >>> boston.friend = 'John Adams'
 Traceback (most recent call last):
  ...
 AttributeError: 'Observer' object has no attribute 'friend'

**A:** 

 ``Observer`` objects restrict which of their attributes can be set
 to prevent users from misspelling attribute names.
 Users seem particularly prone to forgetting
 that longitude is called ``.long``
 which makes them confused about why their ``Observer``
 ignores settings like::

  boston.lon = '-71.0'
  boston.longitude = '-71.0'
  boston.Long = '-71.0'

 So the ``Observer`` object now forcefully prevents this mistake
 by preventing any attribute from being set
 that it does not already recognize.
 If you need an ``Observer`` object
 that can remember additional attributes,
 simply create your own sub-class of ``Observer`` and use that instead:

 >>> class MyObserver(ephem.Observer):
 ...     pass
 >>> boston = MyObserver()
 >>> boston.friend = 'John Adams'
 >>> print(boston.friend)
 John Adams
