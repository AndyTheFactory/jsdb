class SpreadsheetTable : public DataTable
{ // * indicates those functions you need to override
 public:

 class TData
   {
     protected:
     TableLocation Loc;
     //TStr Data;
     const char * data;
     TData(count_t x, size_t y, const char * s);

     public:
     ~TData();
     int Compare(TData & o) { return Loc.Compare(o.Loc); }
     void Set(const char* s);
     const char* Get() {return data?data:"";}
     FRIEND SpreadsheetTable;
   };

 enum TType {Binary,Ascii,Html} Type; // default is Ascii
 int delimiter;
 size_t lastsearch;
 size_t lastsort;
 TableLocation Size;
 TList<TData> Items;
 TStringList Headers;

 Stream* StreamData;
 bool AutoDeleteStreamData;
 // StreamData is used to make a read-only ASCII file that is read only
 // one line at a time.

 SpreadsheetTable(); // *
 SpreadsheetTable(Stream* str, bool AD);
 SpreadsheetTable(const char * filename,bool FirstRowAsTitles=true);
 virtual ~SpreadsheetTable();

 void GetConnectString(TStr& x);

 const char * GetDataC(count_t i, size_t j); // *
 bool SetDataC(count_t i, size_t j, const char * c); // *

 count_t RowCount() {return Size.I;}
 size_t ColumnCount() {return Size.J;}

 bool Optimize(); //sort the list, remove empty cells

 count_t AddRow(TNameValueList & data);
 size_t AddCol(const char * title);
 bool DelRow(count_t i);
 bool DelCol(size_t i);

 void FreeMemory();

 virtual bool Save();

 bool SaveToStream(Stream & out,const char* delim=0,bool WriteTitles = true);

 bool LoadHeader(Stream & in,bool FirstRowAsTitles=true);
 bool LoadASCIILine(count_t x, Stream & in);
 bool LoadFromStream(Stream & in,bool FirstRowAsTitles=true);
  //makes the ASCII/binary determination by itself (looks for a signature)
  //the stream should be opened in binary mode.
  //FirstRowAsTitles is only necessary if the ASCII file doesn't begin
  //with a pound # sign.

 protected:
 size_t BSearch(TableLocation& Loc,size_t low, size_t high);
 TData* Get(count_t x, size_t  y,size_t & index);
 bool Add(count_t x, size_t y, const char * s);
 FRIEND TableDataCache;
};

