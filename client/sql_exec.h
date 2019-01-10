#include <string>
#include <iostream>
#include <vector>
#include <regex>
#include "parser.h"
#include "local_sql.h"
#include "metadata.h"
#include "rpc_sql.h"
using namespace std;

void load_data(Table info, Fragment1 fragment, Load load);
void create_table(Table table);
void delete_table(DeleteQuery deleteQuery);
void insert_table(Insert &insert,Fragment1 fragments);

bool CreateParser(std::string createSql,Table &table);
void printCreate(Table& sql);

bool InsertParser(std::string insertStr , Insert& sql);
void printInsert(Insert &sql);

bool deleteParser(std::string str , DeleteQuery& sql);
void printDeleteParser(DeleteQuery& sql);
void parserCon( std::string instr, std::string outstr[]);
std::string trimSpace(std::string s);
int lenn(std::string a[]);

////select parser
void sel_parser(std::string sel_item ,SelectQuery& sql);
void fro_parser(std::string fro_item,SelectQuery& sql);
void whe_parser(std::string whe_item,SelectQuery& sql);
bool SelectParser(std::string selectSql,SelectQuery &sql);//////main
void printSelectQuery(SelectQuery &sql1);
////


///load
bool ParserLoad(std::string str , Load& sql);
void printLoad(Load& sql);

/////site
bool ParserDefineSite(std::string str , SiteInfo& sql);
void printDefineSite(SiteInfo& sql);

// frag
void parserAtt( std::string instr, std::string outstr[]);
bool FragmentParser(std::string fragmentSql,Fragment& sql);
void printFragment(Fragment &sql);

//setSite
void printMessage();
void setSiteInfos(SiteInfos infos);
void setMe(string ip,int port,string db_name);