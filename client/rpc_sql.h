#include "rpc/client.h"
#include <iostream>
#include <string>
using std::string;
using std::cout;
using std::endl;

bool RPCExecute(string ip,string db_name,string stmt);
int RPCExecuteUpdate(string ip,string db_name,string stmt);
string RPCExecuteQuery(string ip,string db_name,string stmt);
bool RPCExecuteIndsertTable(string ip,string db_name,string stmt);
