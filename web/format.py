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

sys.stdout.write(content)
