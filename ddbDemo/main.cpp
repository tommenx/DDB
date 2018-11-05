#include <iostream>
#include "local_sql.h"

void excute(SQL &);
void excute_update(SQL &);
void excute_query(SQL &);

int main(){
    SQL sql("tcp://127.0.0.1:3306","root","mimacuowu");
//    excute_query(sql);
    excute(sql);
    return 0;
}

void excute(SQL &sql){
    string sql_stmt = "CREATE TABLE test("
                      "id INT NOT NULL,"
                      "name VARCHAR(20) NOT NULL,"
                      "PRIMARY KEY (id)"
                      ");";
    string sql_stmt_2 = "DROP TABLE test;";
    bool ok = sql.localExcute("checkin",sql_stmt);
    if(ok)
        std::cout << "ok" << std::endl;
    else
        std::cout << "not ok" << std::endl;
}

void excute_update(SQL &sql){
    string db_name = "checkin";
    string db_stmt = "UPDATE";
    sql.localExcuteUpdate(db_name,db_stmt);
}

void excute_query(SQL &sql){
    string db_name = "checkin";
    string db_stmt = "SELECT * FROM t_user;";
    string res = sql.localExcuteQuery(db_name,db_stmt);
    cout << res << endl;
}