bin_PROGRAMS = ut-desa-lib

AM_CPPFLAGS = -D_PLAT_LINUX_ -g -O0

ut_desa_lib_SOURCES = \
		       ut_main.cpp \
		       ut_base.cpp \
		       ut_regex.cpp

ut_desa_lib_LDFLAGS=-L../src/.libs/ -ldesa  -lcppunit -lrt
ut_desa_lib_CXXFLAGS=-I@srcdir@/../src -pthread

check: test_desa_lib

test_desa_lib:
	 ./ut-desa-lib


