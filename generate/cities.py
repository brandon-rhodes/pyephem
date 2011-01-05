#!/usr/bin/env python

from urllib2 import urlopen, HTTPError
import re

# Look up each city in "cities.in" on the http://www.fallingrain.com/
# web site, and append their geographic data into "cities.out".

cin = open('cities.in', 'r')
cout = open('cities.out', 'a')

def transchar(c):
    if c == ' ':
        return '32'
    elif c == '-':
        return '45'
    elif c == '.':
        return '46'
    return c

def get_page(name, n):
    url = 'http://www.fallingrain.com/world/a/' + '/'.join(
        [ transchar(c) for c in name[:n] ]
        )
    try:
        return urlopen(url).read()
    except HTTPError:
        return None

def rename(name):
    bettername = {
        'Hong Kong historical': 'Hong Kong',
        'Mexico': 'Mexico City',
        'Sankt-Peterburg': 'St. Petersburg',
        }.get(name, None)
    return bettername or name

class Entry(object):
    pass

def get_entries(content):
    entries = []
    for line in content.split('\n'):
        if line.startswith('<tr><td>'):
            fields = re.split(r'<[^>]*>', line)
            e = Entry()
            e.name = fields[3]
            e.type = fields[5]
            e.lat = fields[8]
            e.lon = fields[9]
            e.elev = fields[10]
            e.pop = int(fields[11])

            entries.append(e)

    return entries

for line in cin:
    line = line.strip()
    if not line or line.startswith('#'):
        continue

    name = line
    i = 4                      # first guess for how deep the URL goes
    content = get_page(name, i)
    if not content:             # we went too deep
        while not content:
            i -= 1
            content = get_page(name, i)
    else:
        while content and i <= len(name):
            old_content = content
            i += 1
            content = get_page(name, i)
        content = old_content

    entries = get_entries(content)
    entries = [ e for e in entries if e.type == 'city' ]
    entries = [ e for e in entries if e.name == name ]

    if entries:
        esort = [ (e.pop, e) for e in entries ]
        esort.sort()
        e = esort[-1][1]
        s = "    '%s': ('%s', '%s', %f)," % (rename(e.name), e.lat, e.long,
                                             float(e.elev) * 0.3048)
        print s
        print >>cout, s
    else:
        print '-------- cannot find:', name
        print >>cout, '-------- cannot find:', name
