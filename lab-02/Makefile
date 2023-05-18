CFLAGS = -std=gnu17 -Wpedantic -Wall -O0 -pipe -fno-plt -fPIC
FILENAME=README.md
UID = $(shell cat ${FILENAME} | grep '## UID' | grep -oe '\([0-9.]*\)')
ifeq ($(shell uname -s),Darwin)
	LDFLAGS =
else
	LDFLAGS = -lrt -Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now
endif

.PHONY: all
all: rr

rr: rr.o

tar:
	@tar -cf ${UID}-lab2-submission.tar rr.c README.md

.PHONY: clean
clean:
	rm -f rr.o rr
