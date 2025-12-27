# $Id: Makefile.am 8294 2024-02-22 21:13:20Z flaterco $

# Enable make distcheck to succeed when out-of-path includes and libs are
# necessary to build.  (Any other variables are still dropped....)
AM_DISTCHECK_CONFIGURE_FLAGS = CPPFLAGS="${CPPFLAGS}" LDFLAGS="${LDFLAGS}"

bin_PROGRAMS = build_tide_db restore_tide_db
dist_bin_SCRIPTS = rewrite_tide_db.sh

build_tide_db_SOURCES = build_tide_db.c xml.c build_tide_db.h
restore_tide_db_SOURCES = restore_tide_db.c restore_tide_db.h
