//used for memory tables, not file tables. Use spreadsheet if you're going to
//save it to a file. Maybe later we'll handle INI files.
class  ParameterTable : public DataTable
{ // * indicates those functions you need to override
 public:

 TNameValueList* Data;
 bool AutoDelete;

 ParameterTable(); // *
 ParameterTable(TNameValueList* Data, bool AutoDelete); // *
 virtual ~ParameterTable();

 void GetConnectString(TStr& x);

 const char * GetDataC(count_t i, size_t j); // *
 bool SetDataC(count_t i, size_t j, const char * c); // *

 count_t RowCount() {return Data->Count();}
 size_t ColumnCount() {return 2;}

 count_t AddRow(TNameValueList & data);
 bool DelRow(count_t i);
};

