/*-------------------------------------------------------------------------
 *
 * pg_synthesize_wal.c
 *		  Module to generate PostgreSQL Write-Ahead Log records of various
 *        sizes.
 *
 * Copyright (c) 2022, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *		  contrib/pg_synthesize_wal/pg_synthesize_wal.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/xlog.h"
#include "access/xlog_internal.h"
#include "access/xlogdefs.h"
#include "access/xloginsert.h"
#include "access/xlogreader.h"
#include "funcapi.h"
#include "miscadmin.h"
#include "utils/pg_lsn.h"

PG_MODULE_MAGIC;

/*---- Structures ----*/
/*
 * SynthesizeWAL record message.
 */
typedef struct xl_synthesizewal_message
{
	Size		message_size;	/* size of the message */
	char		message[FLEXIBLE_ARRAY_MEMBER]; 	/* payload */
} xl_synthesizewal_message;

/*---- Macros ----*/
#define PG_SYNTHESIZE_WAL_RMGR_NAME "SynthesizeWAL"
#define SizeOfSynthesizeWALMessage	(offsetof(xl_synthesizewal_message, message))
#define XLOG_SYNTHESIZE_WAL_MESSAGE	0x00

/*---- Global variables ----*/
static RmgrId rmid = RM_EXPERIMENTAL_ID;
static RmgrData rmgr;

/*---- Function declarations ----*/
void		_PG_init(void);
void		_PG_fini(void);

/* RMGR API*/
void		synthesizewal_redo(XLogReaderState *record);
void		synthesizewal_desc(StringInfo buf, XLogReaderState *record);
const char *synthesizewal_identify(uint8 info);

static void MakeRandomMessage(char *message, size_t size);
static XLogRecPtr LogSynthesizedMessage(const char *message, size_t size);

PG_FUNCTION_INFO_V1(pg_synthesize_wal_record);
PG_FUNCTION_INFO_V1(pg_synthesize_wal_record_bytea);
PG_FUNCTION_INFO_V1(pg_synthesize_wal_record_text);

/*
 * Module load callback
 */
void
_PG_init(void)
{
	/*
	 * In order to create our own custom resource manager, we have to be loaded
	 * via shared_preload_libraries.
	 */
	if (!process_shared_preload_libraries_in_progress)
		ereport(ERROR,
				(errmsg("pg_synthesize_wal module must be loaded via shared_preload_libraries")));

	MemSet(&rmgr, 0, sizeof(rmgr));
	rmgr.rm_name = PG_SYNTHESIZE_WAL_RMGR_NAME;
	rmgr.rm_redo = synthesizewal_redo;
	rmgr.rm_desc = synthesizewal_desc;
	rmgr.rm_identify = synthesizewal_identify;

	RegisterCustomRmgr(rmid, &rmgr);
}

/*
 * Module unload callback
 */
void
_PG_fini(void)
{
	/* Not used for now. */
}

/* RMGR API implementation */

/*
 * Redo is basically just noop for synthesized WAL messages.
 */
void
synthesizewal_redo(XLogReaderState *record)
{
	uint8		info = XLogRecGetInfo(record) & ~XLR_INFO_MASK;

	if (info != XLOG_SYNTHESIZE_WAL_MESSAGE)
		elog(PANIC, "synthesizewal_redo: unknown op code %u", info);
}

void
synthesizewal_desc(StringInfo buf, XLogReaderState *record)
{
	char	   *rec = XLogRecGetData(record);
	uint8		info = XLogRecGetInfo(record) & ~XLR_INFO_MASK;

	if (info == XLOG_SYNTHESIZE_WAL_MESSAGE)
	{
		xl_synthesizewal_message *xlrec = (xl_synthesizewal_message *) rec;

		Assert(xlrec->message_size > 0);

		appendStringInfo(buf, "payload (%zu bytes): %s", xlrec->message_size, xlrec->message);

		/*
		 * XXX: we might need write message payload as a series of hex bytes.
		 */
#if 0
		{
			char	   *sep = "";

			for (int cnt = 0; cnt < xlrec->message_size; cnt++)
			{
				appendStringInfo(buf, "%s%02X", sep,
								 (unsigned char) xlrec->message[cnt]);
				sep = " ";
			}
		}
#endif
	}
}

const char *
synthesizewal_identify(uint8 info)
{
	if ((info & ~XLR_INFO_MASK) == XLOG_SYNTHESIZE_WAL_MESSAGE)
		return "MESSAGE";

	return NULL;
}

/*
 * Write synthesized message into WAL.
 */
XLogRecPtr
LogSynthesizedMessage(const char *message, size_t size)
{
	xl_synthesizewal_message xlrec;

	xlrec.message_size = size;

	XLogBeginInsert();
	XLogRegisterData((char *) &xlrec, SizeOfSynthesizeWALMessage);
	XLogRegisterData(unconstify(char *, message), size);

	/*
	 * XXX: we might need to mark this record as unimportant.
	 */
#if 0
	XLogSetRecordFlags(XLOG_MARK_UNIMPORTANT);
#endif

	return XLogInsert(rmid, XLOG_SYNTHESIZE_WAL_MESSAGE);
}

/*
 * Make random message to synthesize WAL record.
 */
void
MakeRandomMessage(char *message, size_t size)
{
	static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int i;

	for (i = 0; i < size; i++)
	{
		int key = rand() % (int)(sizeof(charset) -1);
		message[i] = charset[key];
	}
}

/*
 * SQL function for writing a synthesized message into WAL.
 */
Datum
pg_synthesize_wal_record(PG_FUNCTION_ARGS)
{
	int64		size = PG_GETARG_INT64(0);
	XLogRecPtr	lsn;
	char 		*message;

	if (size <= 0)
		ereport(ERROR,
				(errmsg("size %lld must be greater than zero to synthesize WAL record",
						(long long) size)));

	message = (char *) palloc0(size);

	MakeRandomMessage(message, size);

	lsn = LogSynthesizedMessage(message, size);

	pfree(message);

	PG_RETURN_LSN(lsn);
}

/*
 * SQL function for writing a user-supplied message (in bytea form) into WAL.
 */
Datum
pg_synthesize_wal_record_bytea(PG_FUNCTION_ARGS)
{
	bytea	   *data = PG_GETARG_BYTEA_PP(0);
	XLogRecPtr	lsn;

	lsn = LogSynthesizedMessage(VARDATA_ANY(data), VARSIZE_ANY_EXHDR(data));

	PG_RETURN_LSN(lsn);
}

/*
 * SQL function for writing a user-supplied message (in text form) into WAL.
 */
Datum
pg_synthesize_wal_record_text(PG_FUNCTION_ARGS)
{
	/* bytea and text are compatible */
	return pg_synthesize_wal_record_bytea(fcinfo);
}
