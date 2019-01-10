#include "rpc_server.h"


void startServer(int port){
    cout << "StartListening" << endl;
    rpc::server srv(port);
    srv.bind("localExecute",&localExecute);
    srv.bind("localExecuteUpdate",&localExecuteUpdate);
    srv.bind("localExecuteQuery",&localExecuteQuery);
    srv.bind("localInsertTable",&localInsertTable);
    srv.suppress_exceptions(true);
    srv.run();
    cout << "srv.run()" << endl;
}