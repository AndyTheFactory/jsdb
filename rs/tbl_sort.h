/*
class TableSorter: public TQuickSorter
{public:
 TDataList * Grid;
 TWORMList Index;
 size_t column;
 TStr temp;
 TableSorter(TDataList&g, size_t col, TIndexList*SortOrder);
 ~TableSorter();
 
 void PreSort();

 int Compare(size_t i,size_t j);
};
    */
class ReorderTable: public DataTable
{
 public:
 DataTable* Grid;
 bool AutoDelete;

 TPointer<TIndexList> Rows, Cols;

 ReorderTable(DataTable* g,bool AD);
 virtual ~ReorderTable();

 const char * filename() {return Grid->filename();}

 void GetConnectString(TStr& x);

 bool SetRowMap(TIndexList* R); //should have been created with new()
 bool SetColMap(TIndexList* R); //should have been created with new()
 void Sort(size_t column,bool ascending=true);

 const char * GetDataC(count_t i, size_t j); // *
 bool SetDataC(count_t i, size_t j, const char * c); // *

 double GetDataD(count_t i, size_t j);
 bool SetDataD(count_t i, size_t j, double d);

 count_t RowCount();
 size_t ColumnCount();

 const char * ColumnTitle(size_t index);  // *
 virtual bool SetTitle(size_t index,const char * c);  // *

 bool Optimize();
 bool Save();

// virtual ERowStatus GetRowStatus(count_t i);
 virtual EDBFieldType GetColStatus(size_t i);
 virtual int GetColWidth(size_t i); //number of chars, default is INT_MAX

 size_t AddCol(const char * title);
 count_t AddRow(TNameValueList& data);
 bool DelCol(size_t j);
 bool DelRow(count_t i);

 protected:
 count_t GetI(count_t i);
 size_t GetJ(size_t j);
 void CopyState();
};
