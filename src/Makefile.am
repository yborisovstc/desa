includedir = @includedir@/desa

AM_CPPFLAGS = -g -O0

lib_LTLIBRARIES = libdesa.la

libdesa_la_SOURCES = \
		       miface.h \
		       mconnpoint.h \
		       mdes.h \
		       connpoint.h \
		       connpoint.cpp \
		       system.h \
		       system.cpp \
		       state.h \
		       state.cpp

include_HEADERS = \
		  miface.h \
		  mdes.h


libdesa_la_LDFLAGS = -rdynamic -no-undefined -lrt
