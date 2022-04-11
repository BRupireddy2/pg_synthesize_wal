# pg_synthesize_wal

A PostgreSQL extension providing functions to generate Write-Ahead Log (WAL) records of various sizes ("huge" WAL records spanning many WAL files). It can either generate random WAL record data of size provided by user or it can also accept the WAL record data from the user (bytea or text form). This extension is "ONLY" recommended on developer or testing or sandbox servers not on production servers. It requires to be loaded via PostgreSQL's shared_preload_libraries configuration parameter (GUC) as it internally uses "Custom WAL Resource Managers" feature [1] to register a custom resource manager with PostgreSQL.

SQL functions
=============
- pg_synthesize_wal_record(IN size int8) - generates WAL record with random data of provided size
- pg_synthesize_wal_record(IN data bytea) - generates WAL record with provided data in bytea form
- pg_synthesize_wal_record(IN data text) - generates WAL record with provided data in text form 

Compatibility with PostgreSQL
=============================
Version 15 and above.

Installation
============
Easiest way to use the extension's source code is to copy it as contrib/pg_synthesize_wal in PostgreSQL source code and run "make install" to compile and "make check" for tests.

Usage
=====
Add pg_synthesize_wal to PostgreSQL's shared_preload_libraries either via postgresql.conf file or ALTER SYTEM SET command and restart the PostgreSQL database cluster i.e. restart the postmaster. Then, create the extension with CREATE EXTENSION pg_synthesize_wal; command and use its functions.

Dependencies
============
pg_synthesize_wal requires pg_walinspect extension (which is available in core PostgreSQL version 15 and above) only for running tests ("make check") but pg_walinspect is not used by it internally.

LICENSE
=======
pg_synthesize_wal is free software distributed under the PostgreSQL Licence.

Copyright (c) 1996-2022, PostgreSQL Global Development Group

Developer
=========
This extension is developed and being maintained by Bharath Rupireddy.

- Twitter: https://twitter.com/BRupireddy
- LinkedIn: www.linkedin.com/in/bharath-rupireddy

Bug Report: https://github.com/BRupireddy/pg_synthesize_wal or <bharath.rupireddyforpostgres@gmail.com>

[1] https://git.postgresql.org/gitweb/?p=postgresql.git;a=commitdiff;h=5c279a6d350205cc98f91fb8e1d3e4442a6b25d1
