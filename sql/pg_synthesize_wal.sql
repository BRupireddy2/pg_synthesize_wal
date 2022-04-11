CREATE EXTENSION pg_walinspect;

CREATE EXTENSION pg_synthesize_wal;

-- Make sure checkpoints don't interfere with the test.
SELECT 'init' FROM pg_create_physical_replication_slot('regress_pg_synthesize_wal_slot', true, false);

SELECT pg_current_wal_flush_lsn() AS wal_lsn1 \gset

SELECT COUNT(*) >= 0 AS ok FROM pg_synthesize_wal_record(50);

SELECT COUNT(*) >= 0 AS ok FROM pg_synthesize_wal_record('synthesized wal record'::text);

SELECT COUNT(*) >= 0 AS ok FROM pg_synthesize_wal_record('synthesized wal record'::bytea);

CREATE TABLE sample_tbl(col1 int, col2 int);

SELECT pg_current_wal_flush_lsn() AS wal_lsn2 \gset

INSERT INTO sample_tbl SELECT * FROM generate_series(1, 2);

-- ===================================================================
-- Test if the above synthesized WAL records exist in the WAL file
-- ===================================================================

SELECT COUNT(*) = 3 AS ok FROM pg_get_wal_records_info(:'wal_lsn1', :'wal_lsn2')
    WHERE resource_manager = 'SynthesizeWAL';

-- ===================================================================
-- Clean up
-- ===================================================================

SELECT pg_drop_replication_slot('regress_pg_synthesize_wal_slot');

DROP TABLE sample_tbl;
