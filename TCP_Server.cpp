//
// Created by trida on 4/1/25.
//

#include "TCP_Server.h"

#include <iomanip>
#include <random>
#include <sstream>

#include "Database_Connection.h"

TCP_Server::TCP_Server() {

    if (( this->server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
       this->error_log("Error while creating socket");
        exit(EXIT_FAILURE);
    }

    this->address.sin_family = AF_INET;
    this->address.sin_addr.s_addr = INADDR_ANY;
    this->address.sin_port = htons(this->port);

    if ( bind(this->server_socket, (struct sockaddr *) &this->address, sizeof(this->address)) < 0 ) {
        this->error_log("Error while binding server socket");
        close(this->server_socket);
        exit(EXIT_FAILURE);
    }


}

std::string TCP_Server::get_time() {

    auto now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);

    std::tm* local_time = std::localtime(&current_time);
    std::ostringstream oss;

    oss << std::put_time(local_time, "%Y-%m%d %H:%M:%S");
    return "[" + oss.str() + "]";

}


void TCP_Server::general_log(std::string message) {
    std::ofstream file("server.log", std::ios::app);
    file << "LOG: " << this->get_time() <<"\n" << message << "\n\n";
    file.close();
}

void TCP_Server::error_log(std::string message) {
    std::ofstream file("server.log", std::ios::app);
    std::string cause = strerror(errno);
    file << "ERR: " << this->get_time() <<"\n" << message << "\n" << cause << "\n\n";
    file.close();
}

void TCP_Server::user_log(std::string username, std::string message) {
    std::ofstream file(username+".log", std::ios::app);
    file <<"LOG: " << this->get_time() << "\n" + message + "\n\n";
    file.close();
}

void TCP_Server::user_error_log(std::string username, std::string message) {
    std::ofstream file(username+".log", std::ios::app);
    std::string cause = strerror(errno);
    file << "ERR: " << this->get_time() << "\n" << message << "\n" << cause << "\n\n";
    file.close();
}

void TCP_Server::handle_client(int client_socket) {

    Database_Connection conn;

    char rcv_buffer[ this->buffer_size ];

    int bytes_received;

    std::string whole_received_message;
    std::unordered_map<std::string, std::string> request_tokens;

    request_tokens["request_code"] = "";
    request_tokens["username"] = "";
    request_tokens["password"] = "";
    request_tokens["email"] = "";
    request_tokens["access_token"] = "";
    request_tokens["refresh_token"] = "";
    request_tokens["revoke_token"] = "";
    request_tokens["playlist_name"] = "";
    request_tokens["song_name"] = "";
    request_tokens["user_name"] = "";
    request_tokens["artist_name"] = "";
    request_tokens["song_path"] = "";

    while (true) {
        whole_received_message = "";
        bytes_received = recv(client_socket, rcv_buffer, this->buffer_size, 0);

        if (bytes_received < 0) {
            this->user_error_log(request_tokens["username"], "Error while receiving message");
            break;
        }

        if (bytes_received == 0) {
            this->user_log(request_tokens["username"], "Connection closed by client");
            if ( request_tokens["username"] != "" ) {
                conn.delete_user_token(request_tokens["username"]);
            }
            break;
        }

        whole_received_message += std::string(rcv_buffer);


        this->user_log(request_tokens["username"], "Received message: <" + whole_received_message + ">");

        std::stringstream ss(whole_received_message);
        std::string item;

        while ( std::getline(ss, item, ';')) {
            size_t pos = item.find(':');
            if ( pos != std::string::npos) {
                std::string campo = item.substr(0, pos);
                std::string valore = item.substr(pos + 1);
                request_tokens[campo] = valore;
            }
        }

        std::string response;

        switch ( std::stoi(request_tokens["request_code"])) {

            default:
                break;

            case 1: {
                int check_credentials_output = conn.check_credentials( request_tokens["username"], request_tokens["password"]);
                if (check_credentials_output == 1) {
                    std::string access_token = "", refresh_token = "", revoke_token = "";
                    do{
                        access_token = generate_random_token();
                        refresh_token = generate_random_token();
                        revoke_token = generate_random_token();
                    }while( conn.check_token_generation(access_token, refresh_token, revoke_token) == -1 ) ;

                    response = "response_code:"+std::to_string(200)+";access_token:"+access_token+";refresh_token:"+refresh_token+";revoke_token:"+revoke_token+"\n";
                    conn.add_token(access_token, refresh_token, revoke_token, request_tokens["username"]);

                }else {
                    response = "response_code:"+std::to_string(404)+"\n";
                }

                break;
            }

            case 2: {
                int check_if_user_exist_output = conn.check_if_user_exists(request_tokens["username"]);
                if ( check_if_user_exist_output == 1) {
                    response = "response_code:"+std::to_string(404)+"\n";
                }else {
                    response = "response_code:"+std::to_string(200)+"\n";
                    conn.add_user(request_tokens["username"], request_tokens["password"]);
                }
                break;
            }

            case 3:
                break;

            case 4:
                break;

            case 5:
                break;

            case 6:
                break;

            case 7:
                break;

            case 8:
                break;

            case 9:
                break;

            case 10:
                break;

            case 11:
                break;

            case 12:
                break;

            case 13:
                break;

            case 14:
                break;

            case 15:
                int token_valid = conn.check_if_token_valid(request_tokens["username"], request_tokens["access_token"], 1);

                if ( token_valid != 1) {
                    std::cout<<"Username e Access_Token per il get delle playlist utente non validi\n";

                    response = "response_code:"+std::to_string(403)+"\n";
                    break;
                }

                int user_id = conn.find_user_id(request_tokens["username"]);
            
                if ( user_id == -1) {
                    std::cout<<"Questo Username per il get delle playlist utente non esiste\n";
                    response = "response_code:"+std::to_string(404)+"\n";
                    break;
                }

                std::string user_playlists = conn.get_user_playlist(user_id, request_tokens["username"]);
                response = "response_code:"+std::to_string(200)+";playlists:"+user_playlists+"\n";

                break;
        }

        send(client_socket, response.c_str(), response.size(), 0);
    }

}

std::string TCP_Server::generate_random_token() {
    int length = 128;
    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<size_t> distribution(0, chars.size() - 1);

    std::string randomString;
    randomString.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        randomString += chars[distribution(generator)];
    }

    return randomString;
}




void TCP_Server::Start() {

    if (listen(this->server_socket, this->max_clients) < 0) {
        perror("Server Socket listen failed");
        close(this->server_socket);
        exit(EXIT_FAILURE);
    }

    std::cout<<"Server Listening on port "<<this->port<<"\n";

    while (true) {

        this->client_socket = accept(this->server_socket, (struct sockaddr*)&this->address, &this->address_len);
        if ( this->client_socket < 0) {
            perror("Server Socket acceptance failed");
            continue;
        }

        std::lock_guard<std::mutex> lock(this->mtx);
        if ( this->threads.size() >= this->max_clients ) {
            std::cout<<"Max clients reached. Connection refused: "<<this->client_socket<<std::endl;
            close( this->client_socket );
        }else {
            std::cout<<"Nuovo client accettato"<<std::endl;
            this->threads.emplace_back([this]() {
                this->handle_client(this->client_socket);
            });
        }
    }

}



