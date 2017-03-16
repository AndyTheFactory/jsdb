#include "rslib.h"
#pragma hdrstop

#include <time.h>

//#include <owl/profile.h>

int32 DBF_DATABASE_TIMEOUT=300000;
// TProfile(RSINISECTION).GetInt("DBREFRESH",300);
    //Five minutes

bool DBF_AUTO_REFRESH=true;

void extern SetDBFFileRefresh(bool i)
 {
  DBF_AUTO_REFRESH=i;
 };

//----------------------------------------------
//
//  DBFCache
//  stores database records in a memory buffer.
//----------------------------------------------

DBFCache::DBFCache()
{
CacheMode=ReadForward;
LoInMem=1;
HiInMem=MaxAvail=0;
};

DBFCache::~DBFCache()
{
CacheMode=ReadForward;
LoInMem=HiInMem=MaxAvail=0;
};

void DBFCache::SetBufferSize(DBFFile * db,EDBCacheSize CacheSize)
{
 count_t size;
 size_t rl=db->RecordLength();
 HiInMem=MaxAvail=0;

switch (CacheSize)
   {
    case CACHE_TINY:  size=10*rl;
                      break; //10 records
    case CACHE_SMALL: size=100*rl;
                      break; //100 records
    case CACHE_BIG:   size = max (200UL*rl,65536UL - (65536UL % rl));
                      break; //64K
    case CACHE_HUGE:  size =  1048576UL - (1048576UL % rl);
                      break; //1M
    case CACHE_NONE:
    default: size=0; //always allocates at least enough space for one record.
   }

if (size<rl) size=rl;

MaxAvail = size / rl;
try {
    Storage.Resize(size);
    return;
}
catch(...)
{
}
try {
	Storage.Resize(rl);
    return;
}
catch(...)
{
 throw xdb("outofmem");
}
//set empty, will reload on next run.
}

bool DBFCache::ReloadData(DBFFile * db)
{
 if (!MaxAvail) return false;
 HiInMem = db->ReadCaseRange(LoInMem,MaxAvail,Storage,Storage.size);

   LastUsed = clock();
 return (HiInMem!=0);
}

bool DBFCache::ReloadRecord(DBFFile * db,count_t CaseNum)
{
 if (!HasRecord(CaseNum)) return true; //don't bother to reload it.
 if (NeedsReloading()) {return ReloadData(db);} //have to do it anyway...

//otherwise, reload just that one record
 count_t offset = (CaseNum - LoInMem);
 if (offset > MaxAvail) return true;  //no room anyway
 count_t rl = db->RecordLength();
 offset = offset * rl;
 char * data = (char *)((char*)(Storage) + offset);
 return (db->ReadCaseRange(CaseNum,1,data,rl)!=0);
}

bool DBFCache::NeedsReloading()
{
 if (!DBF_AUTO_REFRESH) return false;
 int32 time = clock();
 return (time < LastUsed ? true : time - LastUsed > DBF_DATABASE_TIMEOUT) ;
 //handle rollovers in case the computer has been running for 23.8 days or more.
}

bool DBFCache::HasRecord(count_t CaseNum)
{
 return ((CaseNum <=HiInMem) && (CaseNum >=LoInMem));
};

char * DBFCache::RecordData(DBFFile * db,count_t CaseNum)
{
if (!HasRecord(CaseNum))
  {
  LoInMem=CaseNum;
  if (!ReloadData(db)) return 0;
  if (!HasRecord(CaseNum)) return 0;
  }
  else
  {
  if (NeedsReloading())
      if (!ReloadData(db)) return 0;
  }

count_t offset = (CaseNum - LoInMem);
count_t rl = db->RecordLength();
 offset = offset * rl;
 return (char *)((char*)(Storage) + offset);
};

bool DBFCache::Load(DBFFile * db)
{
 if (!MaxAvail) return true; //that's what it's supposed to do.
 LoInMem=1;
 ReloadData(db);
 return true;
};

void DBFCache::Clear()
{
 HiInMem=0;
};


