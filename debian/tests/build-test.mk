#!/usr/bin/make -f

PKG=libosinfo-1.0

CFLAGS=$(shell pkg-config --cflags $(PKG))
LIBS=$(shell pkg-config --libs $(PKG))

all: build-test clean

build-test: debian/tests/build-test.c
	echo $(LIBS)
	gcc -o $@ $(CFLAGS) $< $(LIBS)
	@echo "Build test of $< succeeded"

clean:
	@rm -f build-test
