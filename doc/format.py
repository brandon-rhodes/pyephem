#!/usr/bin/python

import re, sys

substitutions = [
    (r" `", ' &#8216;'),
    (r"` ", '&#8217; '),
    (r"^`", '&#8216;'),
    (r"`$", '&#8217;'),
    (r"`(?=[\w<'])", '&#8216;'),  # left single quote
    (r'(?<=[\-\]\w,.?!>])`', '&#8217;'),  # right single quote
    ('---', '&mdash;'),
    ('~', '&nbsp;'),
    ]

content = sys.stdin.read()

content = re.compile(r' #(.*)$', re.M).sub(r' #<i>\1</i>', content)
content = re.compile(r' *(>>>|\.\.\.) ([^#\n]*)'
                     ).sub(r'\1 <b>\2</b>', content)

for rx, sb in substitutions:
    content = re.sub(rx, sb, content)

def heading_to_name(s):
    return ''.join(s.split())

heading_re = re.compile(r'<h([2-9])>(.*?)</h\1>')
headings = heading_re.findall(content)

toc = '<dl>'
last = 0
for levelstr, heading in headings:
    level = int(levelstr)
    slink = '<a href="#%s">%s</a>' % (heading_to_name(heading), heading)
    if level == 2:
        toc += '<dt>%s\n' % slink
    elif level == 3:
        if last != 3:
            toc += '<dd>'
        toc += '%s<br>\n' % slink
    last = level
toc += '</dl>'

content = content.replace('_TABLE_OF_CONTENTS_', toc)

def named_heading(match):
    level, heading = match.groups()
    name = heading_to_name(heading)
    return '<h%s><a name="%s">%s</a></h%s>' % (level, name, heading, level)

content = heading_re.sub(named_heading, content)

sys.stdout.write(content)
