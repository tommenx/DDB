#include <iostream>
#include "local_sql.h"
#include "rpc_server.h"

void excute();
void excute_update();
void excute_query();

int main(){

//   excute_query();
    startServer();
    return 0;
}

void excute(){
    string sql_stmt = "CREATE TABLE test("
                      "id INT NOT NULL,"
                      "name VARCHAR(20) NOT NULL,"
                      "PRIMARY KEY (id)"
                      ");";
    string sql_stmt_2 = "DROP TABLE test;";
    bool ok = localExecute("checkin",sql_stmt);
    if(ok)
        std::cout << "ok" << std::endl;
    else
        std::cout << "not ok" << std::endl;
}

void excute_update(){
    string db_name = "checkin";
    string db_stmt = "UPDATE";
    localExecuteUpdate(db_name,db_stmt);
}

void excute_query(){
    string db_name = "checkin";
    string db_stmt = "SELECT * FROM customer;";
    string res = localExecuteQuery(db_name,db_stmt);
    cout << res << endl;
}
