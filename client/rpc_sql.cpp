#include "rpc_sql.h"

vector<string> split(string& src,const string& separator){
    vector<string> dest;
    if(src == "")
        return dest;
    string str = src;
    string substring;
    int start = 0, index;
    dest.clear();
    index = str.find_first_of(separator,start);
    do
    {
        if (index != string::npos)
        {    
            substring = str.substr(start,index-start );
            dest.push_back(substring);
            start =index+separator.size();
            index = str.find(separator,start);
            if (start == string::npos) break;
        }
    }while(index != string::npos);

    substring = str.substr(start);
    dest.push_back(substring);
    return dest;
}

bool RPCExecute(string ip,int port,string db_name,string stmt){
    rpc::client client(ip,port);
    bool ok = client.call("localExecute",db_name,stmt).as<bool>();
    // cout <<  "localExecute\t" << stmt << "[" << ok << "]" << endl;
    return ok;
}

int RPCExecuteUpdate(string ip,int port,string db_name,string stmt){
    rpc::client client(ip,port);
    int cnt = client.call("localExecuteUpdate",db_name,stmt).as<int>();
    cout <<  "localExecuteUpdate\t" << stmt << "[" << cnt << "]" << endl;
    return cnt;
}
string RPCExecuteQuery(string ip,int port,string db_name,string stmt){
    rpc::client client(ip,port);
    string res = client.call("localExecuteQuery",db_name,stmt).as<string>();
    cout <<  "localExecuteQuery\t" << stmt << "\n";
    //cout << res << endl;
    return res;
}
bool RPCExecuteIndsertTable(string ip,int port,string db_name,string stmt){
    rpc::client client(ip,port);
    bool ok = client.call("localInsertTable",db_name,stmt).as<bool>();
    cout <<  "localExecuteTable\t" << stmt << "[" << ok << "]" << endl;
    return ok;
}

bool RPCInsertFromSelect(string ip1,int port1,string ip2,int port2,string tb1,string tb2,string db1,string db2){
    string sql1 = "SELECT * FROM " + tb1;
    string res1 = RPCExecuteQuery(ip1,port1,db1,sql1);
    string insert = "INSERT INTO " + tb2 + " VALUES";
    vector<string> res = split(res1,"\n");
    for(int i = 0;i < res.size();i ++){
        insert += "(";
        insert += res[i];
        insert += "),";
    }

    insert.pop_back();
    //cout << insert << endl;
    RPCExecute(ip2,port2,db2,insert);
    cout << "ok" << endl;
    return true;
}
