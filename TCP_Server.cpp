//
// Created by trida on 4/1/25.
//

#include "TCP_Server.h"

#include <iomanip>
#include <random>
#include <sstream>

#include "Database_Connection.h"

TCP_Server::TCP_Server() {

    if ( ( this->server_socket = socket( AF_INET,  SOCK_STREAM,  0 ) ) == 0 ) {
       Logger::server_error_log( "Error while creating socket" );
        exit( EXIT_FAILURE );
    }

    this->address.sin_family = AF_INET;
    this->address.sin_addr.s_addr = INADDR_ANY;
    this->address.sin_port = htons(  this->port );

    if ( bind( this->server_socket, reinterpret_cast<sockaddr *>( &this->address ), sizeof( this->address ) ) < 0 ) {
        Logger::server_error_log( "Error while binding server socket" );
        close( this->server_socket );
        exit( EXIT_FAILURE );
    }


}






void TCP_Server::handle_client( int client_socket ) {

    Database_Connection conn;

    if ( !conn.is_connected() ) {
        Logger::server_error_log( "Database connection failed" );
        return;
    }

    char rcv_buffer[ this->buffer_size ];

    int bytes_received;

    std::string whole_received_message;
    std::unordered_map<std::string, std::string> request_tokens;

    request_tokens[ "request_code" ] = "";
    request_tokens[ "username" ] = "";
    request_tokens[ "password" ] = "";
    request_tokens[ "email" ] = "";
    request_tokens[ "access_token" ] = "";
    request_tokens[ "refresh_token" ] = "";
    request_tokens[ "revoke_token" ] = "";
    request_tokens[ "playlist_name" ] = "";
    request_tokens[ "song_name" ] = "";
    request_tokens[ "user_name" ] = "";
    request_tokens[ "artist_name" ] = "";
    request_tokens[ "song_path" ] = "";

    while ( true ) {
        whole_received_message = "";
        bytes_received = recv( client_socket, rcv_buffer, this->buffer_size, 0 );

        if ( bytes_received < 0 ) {
            Logger::user_error_log( request_tokens[ "username" ], "Error while receiving message" );
            break;
        }

        if ( bytes_received == 0 ) {
            Logger::user_log( request_tokens[ "username" ], "Connection closed by client" );
            if ( !request_tokens[ "username" ].empty() ) {
                conn.delete_user_token( request_tokens[ "username" ] );
            }
            break;
        }

        whole_received_message += std::string( rcv_buffer );

        Logger::user_log( request_tokens[ "username" ], "Received message: \n" + whole_received_message );

        std::stringstream ss( whole_received_message );
        std::string item;

        while ( std::getline( ss, item, ';' ) ) {
            size_t pos = item.find(':');
            if ( pos != std::string::npos ) {
                std::string campo = item.substr( 0, pos );
                std::string value = item.substr( pos + 1 );
                request_tokens[ campo ] = value;
            }
        }

        std::string response;

        switch ( std::stoi( request_tokens[ "request_code" ] ) ) {

            default:
                break;

            case 1: {

                if ( conn.check_credentials( request_tokens[ "username" ], request_tokens[ "password" ] ) == 1 ) {
                    std::string access_token, refresh_token, revoke_token;
                    do{
                        access_token = generate_random_token();
                        refresh_token = generate_random_token();
                        revoke_token = generate_random_token();
                    }while( conn.check_token_generation( access_token, refresh_token, revoke_token ) == -1 ) ;

                    Logger::user_log( request_tokens[ "access_token" ], "Tokens created with success:\n" "access: " + access_token + "\n" + "refresh:" + refresh_token + "\n" + "revoke: " + revoke_token );

                    response = "response_code:" + std::to_string( 200 ) + ";access_token:" + access_token + ";refresh_token:" + refresh_token + ";revoke_token:" + revoke_token + "\n";

                    conn.add_token( access_token, refresh_token, revoke_token, request_tokens[ "username" ] );

                }else {

                    response = "response_code:" + std::to_string( 404 ) + "\n";
                    Logger::user_log( request_tokens[ "username" ], "Credential aren't valid" );

                }

                break;
            }

            case 2: {

                if ( conn.check_if_user_exists( request_tokens["username"] ) == 1) {
                    response = "response_code:" + std::to_string( 404 ) + "\n";
                    Logger::user_log( request_tokens[ "username" ], "Tried to register an already existing account" );
                }else {
                    response = "response_code:" + std::to_string( 200 ) + "\n";
                    Logger::user_log( request_tokens[ "username" ], "Registered new account successfully" );
                    conn.add_user( request_tokens[ "username" ], request_tokens[ "password" ] ); 
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

                int token_valid = conn.check_if_token_valid( request_tokens[ "username" ], request_tokens[ "access_token" ], 1 );

                if ( token_valid != 1 ) {

                    response = "response_code:" + std::to_string( 403 ) + "\n";
                    Logger::user_log( request_tokens[ "username" ], "Token validation failed, invalid access token" );
                    break;
                }

                int user_id = conn.find_user_id( request_tokens[ "username" ] );
            
                if ( user_id == -1 ) {
                    response = "response_code:" + std::to_string( 404 ) + "\n";
                    Logger::user_log( request_tokens[ "username" ], "User does not exist" );
                    break;
                }

                Logger::user_log( request_tokens[ "username" ], "Playlist sent_back" );
                std::string user_playlists = conn.get_user_playlist( user_id, request_tokens[ "username" ] );
                response = "response_code:" + std::to_string( 200 ) + ";playlists:" + user_playlists + "\n";
                break;
        }

        Logger::user_log( request_tokens[ "username" ], "Response\n" + response );
        send( client_socket, response.c_str(), response.size(), 0 );
    }

}

std::string TCP_Server::generate_random_token() {
    constexpr int length = 128;
    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 generator( rd() );
    std::uniform_int_distribution<size_t> distribution( 0, chars.size() - 1 );

    std::string randomString;
    randomString.reserve( length );

    for ( size_t i = 0; i < length; ++i ) {
        randomString += chars[ distribution( generator ) ];
    }

    return randomString;
}

void TCP_Server::Start() {

    if ( listen( this->server_socket, this->max_clients ) < 0 ) {
        Logger::server_error_log( "Error while listening on the server socket" + std::string( strerror( errno ) ) );
        close( this->server_socket );
        exit( EXIT_FAILURE );
    }

    Logger::server_log( "Server is listening on port" + this->port );

    while ( true ) {

        this->client_socket = accept( this->server_socket, reinterpret_cast<sockaddr *>( &this->address ), &this->address_len );
        if ( this->client_socket < 0 ) {
            Logger::server_error_log( "Server Socket acceptance failed" );
            continue;
        }

        std::lock_guard<std::mutex> lock(this->mtx);
        if ( this->threads.size() >= this->max_clients ) {
            Logger::server_log( "Max clients reached. Connection refused to: " + this->client_socket );
            close( this->client_socket );
        }else {
            Logger::server_log( "New client connected" );
            this->threads.emplace_back( [ this ] () {
                this->handle_client( this->client_socket );
            });
        }
    }

}



