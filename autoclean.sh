#!/bin/sh
make clean
make distclean
rm -f Makefile.in
rm -f aclocal.m4
rm -rf autom4te.cache/
rm -rf build-aux
rm -f m4/libtool.m4
rm -f 'm4/lt~obsolete.m4'
rm -f m4/ltoptions.m4
rm -f m4/ltsugar.m4
rm -f m4/ltversion.m4
rm -f config.h.in
rm -f 'config.h.in~'
rm -f configure
rm -f include/Makefile.in
rm -f include/libtw/Makefile.in
rm -f libtw/Makefile.in
rm -f src/Makefile.in
rm -f configure.scan
rm -f autoscan-2.69.log
