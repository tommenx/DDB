//
// Created by tommenx on 2018/10/27.
//
#include <string>
#include <iostream>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/mysql_driver.h>
#define DDBDEMO_LOCAL_SQL_H

using std::string;
using std::cout;
using std::endl;
using sql::mysql::MySQL_Driver;
using sql::Connection;
using sql::Statement;
using std::string;
using sql::ResultSet;
using std::ofstream;


string getMysqlIp();
string getUsername();
string getPassword();
bool localExecute(string db_name,string db_stmt);
int localExecuteUpdate(string db_name,string db_stmt);
string localExecuteQuery(string db_name,string db_stmt);
bool localInsertTable(string db_name,string sql_flle,string table_name);
