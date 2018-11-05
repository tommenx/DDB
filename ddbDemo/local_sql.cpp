//
// Created by tommenx on 2018/10/27.
//

#include "local_sql.h"

SQL::SQL(string m_ip, string m_user, string m_pwd) :ip(m_ip),user(m_user),pwd(m_pwd){
    driver = sql::mysql::get_driver_instance();
    con = driver->connect(ip,user,pwd);
}
//用于DDL
bool SQL::localExcute(string db_name, string db_stmt) {
    Statement *stmt;
    con->setSchema(db_name);
    stmt = con->createStatement();
    bool ok = stmt->execute(db_stmt);
    delete stmt;
    return ok;
}
//执行更新操作
int SQL::localExcuteUpdate(string db_name, string db_stmt) {
    int cnt = -1;
    Statement *stmt;
    con->setSchema(db_name);
    stmt = con->createStatement();
    cnt = stmt->executeUpdate(db_stmt);
    delete stmt;
    return cnt;
}

string SQL::localExcuteQuery(string db_name, string db_stmt) {
    string res;
    Statement *stmt;
    ResultSet *res_set;
    con->setSchema(db_name);
    stmt = con->createStatement();
    res_set = stmt->executeQuery(db_stmt);
    int col_cnt = res_set->getMetaData()->getColumnCount();
    while(res_set->next()){
//        for(int i = 0;i < col_cnt;i++){
//            res.append(res_set->getString(i));
//            res.append("\t");
//        }
//        res.append("\n");
        res.append(std::to_string(res_set->getUInt("F_ID")));
        res.append("\t");
        res.append(res_set->getString("F_Name"));
        res.append("\t");
        res.append(res_set->getString("F_PhotoUrl"));
        res.append("\n");

    }
    delete stmt;
    delete res_set;
    return res;
}
SQL::~SQL() {
    con->close();
    delete con;
    delete driver;
}

