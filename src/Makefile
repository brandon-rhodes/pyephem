all: ../ephem.py ephem.i ../web/pyephem-manual.html

../ephem.py: ephem.py
	cp $< $@

ephem.py: ephem.i
	swig -dnone -python -globals preference -shadow $<

ephem.i: format_swig.m4 ephem.m4
	m4 --prefix-builtins format_swig.m4 ephem.m4 > $@

../web/pyephem-manual.html: format_html.m4 ephem.m4
	m4 --prefix-builtins format_html.m4 ephem.m4 > $@

clean:
	rm -f ephem.i pyephem-manual.html
