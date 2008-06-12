#
# Building data sets
#

D=./extensions/data
G=./generate
BDLS=$(patsubst $G/%, %, $(wildcard $G/*.9910 $G/*.1020))
BDLSRCS=$(patsubst %, $D/%.c, $(BDLS))

data: $(BDLSRCS)

$(BDLSRCS): $D/%.c: $G/% $G/satxyz.py
	bin/rebuild-plmoon-data $G/$* > $@
