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
    return "456789";
}
//用于DDL
bool localExecute(string db_name, string db_stmt) {
    MySQL_Driver *driver;
	Connection *con;
    Statement *stmt;
    sql::ConnectOptionsMap connection_properties;
    connection_properties["hostName"] = "tcp://127.0.0.1";
    connection_properties["userName"] = getUsername();
    connection_properties["password"] = getPassword();
    connection_properties["port"] = 3306;
    connection_properties["CLIENT_LOCAL_FILES"] = true;

    driver = sql::mysql::get_driver_instance();
    con = driver->connect(connection_properties);
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
    string res;
    MySQL_Driver *driver;
	Connection *con;
    Statement *stmt;
    ResultSet *res_set;
    sql::ConnectOptionsMap connection_properties;
    connection_properties["hostName"] = "tcp://127.0.0.1:3306";
    connection_properties["userName"] = getUsername();
    connection_properties["password"] = getPassword();
    connection_properties["port"] = 3306;
    connection_properties["CLIENT_LOCAL_FILES"] = true;
    driver = sql::mysql::get_driver_instance();
    // con = driver->connect(getMysqlIp(),getUsername(),getPassword());
    con = driver->connect(connection_properties);
    con->setSchema(db_name);
    stmt = con->createStatement();
    res_set = stmt->executeQuery(db_stmt);
    cout << db_stmt << endl;
    int col_cnt = res_set->getMetaData()->getColumnCount();
    bool f = true;
    
    while(res_set->next()){
       for(int i = 1;i <= col_cnt;i++){
            // cout << "dataType:" << res_set->getMetaData()->getColumnType(i) << endl;
            // int = 5
            // char = 11 
            if(5 == res_set->getMetaData()->getColumnType(i)){
                res.append(res_set->getString(i));
            }
            else{
                string t = "'" + res_set->getString(i) + "'";
                res.append(t);
            }
            if(i != col_cnt)
                res.append(",");
       }
        res.append("\n");
    }
    res.pop_back();

    // cout << res << endl;
    con->close();    
    delete res_set;
    delete stmt;
    delete con;

    return res;
}


bool localInsertTable(string db_name,string sql_file,string table_name){
    string db_stmt = "";
    MySQL_Driver *driver;
    Connection *con;
    Statement *stmt;
    bool res = false;
    sql::ConnectOptionsMap connection_properties;
    connection_properties["hostName"] = "tcp://127.0.0.1:3306";
    connection_properties["userName"] = getUsername();
    connection_properties["password"] = getPassword();
    connection_properties["port"] = 3306;
    connection_properties["CLIENT_LOCAL_FILES"] = true;

    try{
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(connection_properties);
        // con = driver->connect(getMysqlIp(),getUsername(),getPassword());
        con->setSchema(db_name);
        stmt = con->createStatement();
        db_stmt = "LOAD DATA INFILE '" 
        + sql_file 
        + "' INTO TABLE " 
        + table_name 
        + " FIELDS TERMINATED BY '\t' " 
        + "LINES TERMINATED BY '\n';";
        cout << db_stmt << endl;
        // string sql = "select * from customer";
        res = stmt ->execute(db_stmt);

        cout << "LOAD: " << res << endl;
        con->close();
        delete stmt;
        delete con;
        return res;
    } catch (sql::SQLException &e){
        cout << "# ERR: SQLException in " << __FILE__;
        // cout << "(" << __FUNCTION__ << ") on line "»
        // << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }   
}

