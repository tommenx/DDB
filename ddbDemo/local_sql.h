//
// Created by tommenx on 2018/10/27.
//

#ifndef DDBDEMO_LOCAL_SQL_H

#include <string>
#include <iostream>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <mysql_driver.h>
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

class SQL {
public:
    SQL(string m_ip,string m_user,string m_pwd );
    bool localExcute(string db_name,string db_stmt);
    int localExcuteUpdate(string db_name,string db_stmt);
    string localExcuteQuery(string db_name,string db_stmt);
    ~SQL();
private:
    string ip,user,pwd;
    MySQL_Driver *driver;
    Connection *con;
};


#endif //DDBDEMO_LOCAL_SQL_H
