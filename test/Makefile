.PHONY: all reset

BINS=fmt stack cas hook hook2 verypush stress1 fmtarr drain
BINS+=bench-expire expire dumptwice syncget ns nsabuse
CFLAGS=-I../../src -I ../src
LDFLAGS=../src/libsdb.a
SCRIPTS=createfail.sh testgrep.sh delkey.sh
ALL=all

all: $(BINS) $(SCRIPTS) reset
	@suite/run.sh
	-cd api && $(MAKE)

asan:
	$(MAKE) CFLAGS="-fsanitize=address ${CFLAGS}" ALL=asan all

$(SCRIPTS):
	./$@

$(BINS):
	$(CC) -o $@ $@.c $(LDFLAGS) $(CFLAGS)
	./$@

reset:
	cd reset && $(MAKE) $(ALL)

add10k:
	sh add10k.sh

clean:
	rm -f f dumptwice.db
	rm -f a test.db db a.gz a.out $(BINS)

.PHONY: ${SCRIPTS} ${BINS}
