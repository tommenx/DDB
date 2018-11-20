#include "rpc_sql.h"
#include <string>
#include <iostream>

int main(){
    string sql_statement = "CREATE TABLE Persons (Id_P int, LastName varchar(255),  FirstName varchar(255), Address varchar(255), City varchar(255));";
    RPCExecute("127.0.0.1","checkin",sql_statement);
    return 0;
}