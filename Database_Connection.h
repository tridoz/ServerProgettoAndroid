#ifndef DATABASE_CONNECTION_H
#define DATABASE_CONNECTION_H

#include <iostream>
#include <sstream>

#include <mysql_driver.h>
#include <mysql_connection.h>

#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>

#include <openssl/sha.h>

#include "Logger.h"

class Database_Connection {
private:

    bool connected = false;
    const std::string host = "tcp://localhost:3306";
    const std::string username = "trida";
    const std::string password = "Mogg4356%#TRIDAPALI";
    const std::string db_name = "ProgettoAndroid";

    sql::mysql::MySQL_Driver *driver;
    std::unique_ptr<sql::Connection> connection;
    std::unique_ptr<sql::Statement> stmt;
    std::unique_ptr<sql::PreparedStatement> pstmt;
    std::unique_ptr<sql::ResultSet> result;


    std::string calculate_hash(std::string input);

    std::string get_generated_query( const std::string& query_template, const std::vector<std::string>& values);
    std::string escape_sql_string( const std::string& value );

public:
    Database_Connection();

    int find_user_id( const std::string& username );
    int find_playlist_id( const std::string& playlist_name );
    int find_song_id( const std::string& song_name );

    int check_if_user_exists( std::string username );
    int check_credentials( const std::string& username, const std::string &password );
    int check_if_token_valid( const std::string& username, const std::string& token, int token_type );
    int check_token_generation( std::string access_token, std::string refresh_token, std::string revoke_token );
    int get_playlist_content( std::string playlist_name );
    int search_songs( std::string song_name );
    int search_playlists( std::string playlist_name );
    int search_artists( std::string artist_name );
    int search_users( std::string user_name );
    int search_song_in_playlist( std::string username, std::string playlist_name, std::string song_name );

    std::string get_user_playlist( int user_id, const std::string& username );

    int add_user( const std::string& username, const std::string &password );
    int add_token( const std::string& access_token, const std::string& refresh_token, const std::string& revoke_token, const std::string& username );
    int add_song_to_playlist( std::string playlist, std::string song_name, std::string username );
    int add_song_to_liked( std::string username, std::string song_name );
    int create_playlist( std::string username, std::string playlist_name, bool isPrivate );

    int delete_token( std::string access_token, std::string refresh_token, std::string revoke_token) ;
    int delete_user_token( const std::string& username );

    bool is_connected();
};



#endif //DATABASE_CONNECTION_H
