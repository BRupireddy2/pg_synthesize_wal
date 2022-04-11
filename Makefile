# contrib/pg_synthesize_wal/Makefile

MODULE_big = pg_synthesize_wal
OBJS = \
	$(WIN32RES) \
	pg_synthesize_wal.o
PGFILEDESC = "pg_synthesize_wal - Module to generate PostgreSQL Write-Ahead Log records of various sizes"

EXTENSION = pg_synthesize_wal
DATA = pg_synthesize_wal--1.0.sql

REGRESS = pg_synthesize_wal

# pg_walinspect is required only for test purposes, pg_synthesize_wal doesn't use it internally
EXTRA_INSTALL = contrib/pg_walinspect

REGRESS_OPTS = --temp-config $(top_srcdir)/contrib/pg_synthesize_wal/synthesizewal.conf

# Disabled because these tests require "wal_level=replica", which
# some installcheck users do not have (e.g. buildfarm clients).
NO_INSTALLCHECK = 1

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_synthesize_wal
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
