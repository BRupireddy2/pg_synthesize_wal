/* contrib/pg_synthesize_wal/pg_synthesize_wal--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pg_synthesize_wal" to load this file. \quit

--
-- pg_synthesize_wal_record()
--
-- Writes a synthesized message into WAL.
--
CREATE FUNCTION pg_synthesize_wal_record(IN size int8,
    OUT lsn pg_lsn
)
AS 'MODULE_PATHNAME', 'pg_synthesize_wal_record'
LANGUAGE C STRICT PARALLEL UNSAFE;

--
-- pg_synthesize_wal_record()
--
-- Writes a user-supplied message in bytea form into WAL.
--
CREATE FUNCTION pg_synthesize_wal_record(IN data bytea,
    OUT lsn pg_lsn
)
AS 'MODULE_PATHNAME', 'pg_synthesize_wal_record_bytea'
LANGUAGE C STRICT PARALLEL UNSAFE;

--
-- pg_synthesize_wal_record()
--
-- Writes a user-supplied message in text form into WAL.
--
CREATE FUNCTION pg_synthesize_wal_record(IN data text,
    OUT lsn pg_lsn
)
AS 'MODULE_PATHNAME', 'pg_synthesize_wal_record_text'
LANGUAGE C STRICT PARALLEL UNSAFE;
