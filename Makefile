#
# Building data sets
#

D=./data
E=./extensions/data
BDLS=$(patsubst $D/%, %, $(wildcard $D/*.9910 $D/*.1020 $D/*.2040))
BDLSRCS=$(patsubst %, $E/%.c, $(BDLS))

data: $(BDLSRCS)

$(BDLSRCS): $E/%.c: $D/% bin/rebuild-plmoon-data
	bin/rebuild-plmoon-data $D/$* > $@
