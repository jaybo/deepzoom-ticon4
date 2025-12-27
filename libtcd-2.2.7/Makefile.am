# $Id: Makefile.am 7212 2020-04-08 15:51:03Z flaterco $

lib_LTLIBRARIES        = libtcd.la
libtcd_la_SOURCES     = bit_pack.c tide_db.c tide_db_default.h tide_db_header.h
nodist_include_HEADERS = tcd.h
dist_pkgdata_DATA      = libtcd.html

# Added at the behest of libtoolize.
ACLOCAL_AMFLAGS = -I m4

# -version-info current:revision:age

# Shared lib versions are almost, but not quite, completely unlike
# release versions.  They are sufficiently unlike that if the two
# happen to be the same it will cause people to jump to the conclusion
# that you don't know what you are doing.

# If the library source code has changed at all since the last update,
# then increment revision (c:r:a becomes c:r+1:a).

# If any interfaces have been added, removed, or changed since the
# last update, increment current, and set revision to 0.

# If any interfaces have been added since the last public release,
# then increment age.

# If any interfaces have been removed since the last public release,
# then set age to 0.

# FIXME: Update libtool version info on each release
libtcd_la_LDFLAGS = -version-info 1:2:0
