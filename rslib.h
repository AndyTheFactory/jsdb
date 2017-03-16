#ifndef _RSLIB_H
#define _RSLIB_H
#include "rs/compiler.h"
#include "rs/defs.h"
#ifdef __sun__
#pragma pack(1)
#else
#pragma pack(push,1)
#endif
#include "rs/string.h"
#include "rs/list.h"
#include "rs/pointer.h"
#include "rs/char.h"
#include "rs/system.h"
#include "rs/except.h"
#include "rs/file.h"
#include "rs/stream.h"
#include "rs/streamproc.h"
#include "rs/dbf.h"
#include "rs/ezf.h"
#include "rs/xml.h"
#include "rs/table.h"
#include "rs/mail.h"
#include "rs/sort.h"
#include "rs/os.h"
#include "rs/query.h"
#include "rs/parse.h"
#include "rs/gif.h"
#include "rs/acrobat.h"
#include "rs/zip.h"
#ifndef TBL_NO_SQL
#include "rs/sql.h"
#include "rs/tbl_sql.h"
#endif
#include <errno.h>
#ifdef __sun__
#pragma pack(4)
#else
#pragma pack(pop)
#endif
#endif
