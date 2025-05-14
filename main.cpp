#include<iostream>

#include "Database_Connection.h"
#include "TCP_Server.h"


int main( int argc, char *argv[] ) {
    //Database_Connection db_conn;
    TCP_Server server;
    server.Start();

    return 0;
}
