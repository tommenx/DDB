//
// Created by tommenx on 2018/10/27.
//

#include "rpc_server.h"


void startServer(){
   rpc::server srv(8080);
   srv.bind("localExecute",&localExecute);
   srv.bind("localExecuteUpdate",&localExecuteUpdate);
   srv.bind("localExecuteQuery",&localExecuteQuery);
   srv.bind("localInsertTable",&localInsertTable);
   srv.suppress_exceptions(true);
   srv.run();
   cout << "srv.run()" << endl;
}
