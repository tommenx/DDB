#include <string>
#include <iostream>
#include <vector>
#include <regex>
#include "parser.h"
#include "local_sql.h"
#include "rpc_sql.h"
using namespace std;

void load_data(TableInfo info, Fragment1 fragment, Load load);
void create_table(Table &table);
void delete_table(DeleteQuery deleteQuery);
void insert_table(Insert &insert,Fragment1 fragments,int values_count);

bool CreateParser(std::string createSql,Table& sql);
void printCreate(Table& sql);

bool InsertParser(std::string insertStr , Insert& sql);
void printInsert(Insert &sql);

bool deleteParser(std::string str , DeleteQuery& sql);
void printDeleteParser(DeleteQuery& sql);
void parserCon( std::string instr, std::string outstr[]);
std::string trimSpace(std::string s);
int lenn(std::string a[]);