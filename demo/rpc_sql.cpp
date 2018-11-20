#include "rpc_sql.h"

bool RPCExecute(string ip,string db_name,string stmt){
    rpc::client client(ip,8080);
    bool ok = client.call("localExecute",db_name,stmt).as<bool>();
    cout <<  "localExecute\t" << stmt << "[" << ok << "]" << endl;
    return ok;
}

int RPCExecuteUpdate(string ip,string db_name,string stmt){
    rpc::client client(ip,8080);
    int cnt = client.call("localExecuteUpdate",db_name,stmt).as<int>();
    cout <<  "localExecuteUpdate\t" << stmt << "[" << cnt << "]" << endl;
    return cnt;
}
string RPCExecuteQuery(string ip,string db_name,string stmt){
    rpc::client client(ip,8080);
    string res = client.call("localExecuteQuery",db_name,stmt).as<string>();
    cout <<  "localExecuteQuery\t" << stmt << "\n";
    cout << res << endl;
    return res;
}
bool RPCExecuteIndsertTable(string ip,string db_name,string stmt){
    rpc::client client(ip,8080);
    bool ok = client.call("localInsertTable",db_name,stmt).as<bool>();
    cout <<  "localExecuteTable\t" << stmt << "[" << ok << "]" << endl;
    return ok;
}