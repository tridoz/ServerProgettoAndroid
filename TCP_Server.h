//
// Created by trida on 4/1/25.
//

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>
#include <mutex>

#include "Database_Connection.h"


class TCP_Server {
private:
    const int port = 8000;
    const int buffer_size = 4096;
    const int max_clients = 20;
    int server_socket, client_socket;

    std::vector<std::thread> threads;
    std::mutex mtx;

    struct sockaddr_in address;
    socklen_t address_len = sizeof(address);
    void handle_client(int client_socket);
    std::string generate_random_token();
public:
    TCP_Server();

    void Start();

};



#endif //TCP_SERVER_H
