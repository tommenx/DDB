//
// Created by tommenx on 2018/10/27.
//

#include "local_sql.h"


string getMysqlIp(){
    return "tcp://127.0.0.1:3306";
}

string getUsername(){
    return "root";
}
string getPassword(){
    return "123456";
}
//用于DDL
bool localExecute(string db_name, string db_stmt) {
    MySQL_Driver *driver;
	Connection *con;
    Statement *stmt;
    driver = sql::mysql::get_driver_instance();
    con = driver->connect(getMysqlIp(),getUsername(),getPassword());
    con->setSchema(db_name);
    stmt = con->createStatement();
    bool ok = stmt->execute(db_stmt);
    cout << db_stmt << endl;
    con->close();
    delete stmt;
    delete con;
    return ok;
}
//执行更新操作
int localExecuteUpdate(string db_name, string db_stmt) {
    int cnt = -1;
    MySQL_Driver *driver;
	Connection *con;
    Statement *stmt;
    driver = sql::mysql::get_driver_instance();
    con = driver->connect(getMysqlIp(),getUsername(),getPassword());
    con->setSchema(db_name);
    stmt = con->createStatement();
    cout << db_stmt << endl;
    cnt = stmt->executeUpdate(db_stmt);
    con->close();
    delete stmt;
    delete con;
    return cnt;
}

string localExecuteQuery(string db_name, string db_stmt) {
    string res = " ";
    MySQL_Driver *driver;
	Connection *con;
    Statement *stmt;
    ResultSet *res_set;
    driver = sql::mysql::get_driver_instance();
    con = driver->connect(getMysqlIp(),getUsername(),getPassword());
    con->setSchema(db_name);
    stmt = con->createStatement();
    res_set = stmt->executeQuery(db_stmt);
    cout << db_stmt << endl;
    int col_cnt = res_set->getMetaData()->getColumnCount();
    bool f = true;
    while(res_set->next()){
       for(int i = 1;i <= col_cnt;i++){
           res.append(res_set->getString(i));
           res.append("\t");
       }
        res.append("\n");
    }

    // cout << res << endl;
    con->close();    
    delete res_set;
    delete stmt;
    delete con;

    return res;
}


bool localInsertTable(string db_name,string sql_flle,string table_name){
    string file_name = "tmp";
    // remove(file_name.c_str());
}

