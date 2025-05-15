//
// Created by trida on 4/1/25.
//

#include "Database_Connection.h"



Database_Connection::Database_Connection() {
    try {
        this->driver = sql::mysql::get_mysql_driver_instance();
    }catch (sql::SQLException &exception) {
        Logger::database_error_log("Error while sql::mysql::get_driver_instance", exception.what() );
        return;
    }
    Logger::database_log("[driver] initialized correctly");

    try {
        this->connection.reset( this->driver->connect( this->host, this->username, this->password ) );
    }catch ( sql::SQLException &exception ) {
        Logger::database_error_log("Error while connecting to database", exception.what() );
        return;
    }
    Logger::database_log("[connection] initialized correctly");

    try {
        this->connection->setSchema( this->db_name );
    }catch (sql::SQLException &exception) {
        Logger::database_error_log("Error while setting database schema", exception.what() );
        return;
    }
    Logger::database_log("[database schema] initialized correctly");

    try{
        this->stmt.reset( this->connection->createStatement() );
    }catch (sql::SQLException &exception) {
        Logger::database_error_log("Error while creating statement", exception.what() );
        return;
    }
    Logger::database_log("[statement] initialized correctly");
    this->connected = true;

}

int Database_Connection::find_playlist_id( const std::string& playlist_name ) {

}

int Database_Connection::find_song_id( const std::string& song_name ) {

}

std::string Database_Connection::get_user_playlist(const int user_id, const std::string& username ) {

    try {
        this->pstmt.reset( this->connection->prepareStatement( "SELECT Title FROM Playlist WHERE Id_Utente = ?" ) );
    }catch (sql::SQLException &exception) {
        Logger::database_error_log("Error while creating statement in [get_user_playlist]", exception.what() );
        return "[error]";
    }



    try {
        this->pstmt->setInt( 1, user_id );
    }catch (sql::SQLException &exception) {

        Logger::database_error_log("Error while setting [Int] parameter [user_id] in [get_user_playlist]", exception.what() );
        return "[error]";
    }

    try {
        this->result.reset( this->pstmt->executeQuery() );
    }catch ( sql::SQLException &exception ) {
        std::string sql_template = "SELECT Title FROM Playlist WHERE Id_Utente = ?";
        std::vector<std::string> values = {std::to_string(user_id)};
        Logger::database_error_log("Error while executing query [" + get_generated_query(sql_template, values) + "]" +" in [get_user_playlist]", exception.what() );
        return "[error]";
    }

    try{
        if ( !this->result->next() ) {
            return "[no playlist found]";
        }
    }catch ( sql::SQLException &exception ) {
        Logger::database_error_log("Error while checking if [result] is empty in [get_user_playlist]", exception.what() );
        return "[error]";
    }

    std::string playlist_set = "[";

    try {
        do {
            std::string title;
            try {
                title = this->result->getString( "Title" );
            }catch ( sql::SQLException &exception ) {
                Logger::database_error_log("Error while getting [Title] from [result] in [get_user_playlist] ", exception.what() );
                return "[error]";
            }

            playlist_set += title + "|" + username +"&";

        } while ( this->result->next() );

    }catch ( sql::SQLException &exception ) {
        Logger::database_error_log("Error while iterating over [result] in [get_user_playlist]", exception.what() );
        return "[error]";
    }

    playlist_set += "]";
    return playlist_set;

}

int Database_Connection::find_user_id( const std::string& username ) {
    this->pstmt.reset(this->connection->prepareStatement( "SELECT Id FROM Users WHERE Username = ?" ) );
    this->pstmt->setString( 1, username );

    this->result.reset( this->pstmt->executeQuery() );

    this->result->last();

    if( this->result->getRow() == 1){
        return this->result->getInt( "Id" );
    }

    return -1;

}

int Database_Connection::add_token( const std::string& access_token, const std::string& refresh_token, const std::string& revoke_token, const std::string& username ) {

    const int userId = find_user_id( username );

    if(userId == -1) {
        return -1;
    }

    this->pstmt.reset( this->connection->prepareStatement("INSERT INTO Tokens VALUES(?,?,?,?)" ) );
    this->pstmt->setString( 1, access_token );
    this->pstmt->setString( 2, refresh_token );
    this->pstmt->setString( 3, revoke_token );
    this->pstmt->setInt( 4, userId );


    this->pstmt->executeUpdate();
    return 1;

}

int Database_Connection::add_user( const std::string& username, const std::string &password ) {
    const std::string hashed_password = calculate_hash( password );

    this->pstmt.reset( this->connection->prepareStatement( "SELECT Id from Users ORDER BY Id ASC" ) );
    this->result.reset( this->pstmt->executeQuery() );

    int id = 1;
    if ( this->result->next() ) {
        this->result->last();
        id = this->result->getInt( "Id" );
    }


    this->pstmt.reset( this->connection->prepareStatement( "INSERT INTO Users VALUES(?,?,?)" ) );
    this->pstmt->setInt( 1, id );
    this->pstmt->setString( 2, username );
    this->pstmt->setString( 3, hashed_password );

    this->pstmt->executeUpdate();
    return 1;
}

int Database_Connection::check_credentials( const std::string& username, const std::string &password ) {
    const std::string hashed_password = calculate_hash( password );
    this->pstmt.reset( this->connection->prepareStatement( "SELECT * FROM Users WHERE Username = ? AND Password = ?" ) );
    this->pstmt->setString( 1, username );
    this->pstmt->setString( 2, hashed_password );

    this->result.reset( this->pstmt->executeQuery() );
    this->result->last();
    if ( this->result->getRow() == 1 )
        return 1;

    return -1;

}

int Database_Connection::delete_user_token( const std::string& username ) {
    const int id_utente = find_user_id( username );

    this->pstmt.reset( this->connection->prepareStatement( "DELETE FROM Tokens WHERE Id_Utente = ?" ) );
    this->pstmt->setInt( 1, id_utente );
    this->pstmt->executeUpdate();

    return 1;
}

int Database_Connection::check_if_token_valid( const std::string& username, const std::string& token, const int token_type ) {
    const int id_utente = find_user_id( username );

    if ( id_utente == -1 ) {

        std::cout<<"Utente " << username << " non trovato nel database\n";
        return -1;
    }

    std::cout << "Utente " << username << " trovato nel database con codice: " << id_utente << "\n";


    std::string query ;

    switch ( token_type ) {
        case 1:
            query = "SELECT * FROM Tokens WHERE Id_Utente = ? AND Access_Token = ?" ;
            break;

        case 2:
            query = "SELECT * FROM Tokens WHERE Id_Utente = ? AND Refresh_token = ?" ;
        break;

        case 3:
            query = "SELECT * FROM Tokens WHERE Id_Utente = ? AND Revoke_token = ?" ;
            break;
    }

    this->pstmt.reset( this->connection->prepareStatement(query) );

    this->pstmt->setInt( 1, id_utente );
    this->pstmt->setString( 2, token );

    this->result.reset( this->pstmt->executeQuery() );

    sql::ResultSetMetaData* meta = this->result->getMetaData();
    int columnCount = meta->getColumnCount();

    this->result->last();
    if ( this->result->getRow() == 1 ) {
        return 1;
    }

    return -1;
}

int Database_Connection::check_token_generation( std::string access_token, std::string refresh_token, std::string revoke_token ) {

    try {
        this->pstmt.reset(this->connection->prepareStatement(
            "SELECT * FROM Tokens WHERE "
            "Access_Token = ? OR Access_Token = ? OR Access_Token = ? OR "
            "Refresh_Token = ? OR Refresh_Token = ? OR Refresh_Token = ? OR "
            "Revoke_Token = ? OR Revoke_Token = ? OR Revoke_Token = ?"
        ));
    }catch (sql::SQLException &exception) {
        Logger::database_error_log("Error while creating [pstmt] in [check_token_generation]", exception.what() );
        return -1;
    }
    Logger::database_log("Statement created correctly in [check_token_generation]");

    const std::vector<std::string> values = {
        access_token, refresh_token, revoke_token,
        access_token, refresh_token, revoke_token,
        access_token, refresh_token, revoke_token
    };

    for (int i = 0; i < values.size(); ++i) {
        try {
            this->pstmt->setString(i + 1, values[i]);
        } catch (sql::SQLException &exception) {
            Logger::database_error_log("Error while setting parameter " + std::to_string(i + 1) + " in [check_token_generation]", exception.what());
            return -1;
        }
    }

    Logger::database_log("All parameters set correctly in [check_token_generation]");

    try {
        this->result.reset( this->pstmt->executeQuery() );
    }catch (sql::SQLException &exception) {
        const std::string sql_template = "SELECT * FROM Tokens WHERE "
            "Access_Token = ? OR Access_Token = ? OR Access_Token = ? OR "
            "Refresh_Token = ? OR Refresh_Token = ? OR Refresh_Token = ? OR "
            "Revoke_Token = ? OR Revoke_Token = ? OR Revoke_Token = ?";

        Logger::database_error_log("Error while executing query [ " + get_generated_query(sql_template, values) + " in [check_token_generation]", exception.what() );
        return -1;
    }
    Logger::database_log("Query executed correctly in [check_token_generation]");

    try {
        if ( !this->result->next() ) {
            Logger::database_log("Result checked with success in [check_token_generation]");
            return 1;
        }
    }catch ( sql::SQLException &exception ) {
        Logger::database_error_log("Error while checking if [result] is empty in [check_token_generation]", exception.what() );
        return -1;
    }

    Logger::database_log("Result checked with success in [check_token_generation]");
    return -1;

}

int Database_Connection::check_if_user_exists( std::string username ) {
    this->pstmt.reset( this->connection->prepareStatement( "SELECT * FROM Users WHERE Username = ?" ) );
    this->pstmt->setString( 1, username );
    this->result.reset( this->pstmt->executeQuery() );

    this->result->last();
    if ( this->result->getRow() == 1 )
        return 1;

    return -1;
}

int Database_Connection::add_song_to_liked( std::string username, std::string song_name ) {

}

int Database_Connection::add_song_to_playlist( std::string playlist, std::string song_name, std::string username ) {

}

int Database_Connection::delete_token( std::string access_token, std::string refresh_token, std::string revoke_token ) {

}

int Database_Connection::get_playlist_content( std::string playlist_name ) {

}

int Database_Connection::search_artists( std::string artist_name ) {

}

int Database_Connection::search_playlists( std::string playlist_name ) {

}

int Database_Connection::search_song_in_playlist( std::string username, std::string playlist_name, std::string song_name ) {

}

int Database_Connection::search_songs( std::string song_name ) {

}

int Database_Connection::search_users( std::string user_name ) {

}

int Database_Connection::create_playlist( std::string username, std::string playlist_name, bool isPrivate ) {

}

std::string Database_Connection::calculate_hash( std::string input ) {

    unsigned char hash[ SHA256_DIGEST_LENGTH ];
    SHA256_CTX sha256;
    SHA256_Init( &sha256 );
    SHA256_Update(&sha256, input.c_str(), input.size() );
    SHA256_Final( hash, &sha256 );

    std::string output;
    for (int i = 0 ; i < SHA256_DIGEST_LENGTH ; i++ ) {
        char buf[ 3 ];
        sprintf( buf, "%02x", hash[ i ] );
        output += buf;
    }
    return output;
}


std::string Database_Connection::escape_sql_string(const std::string &value) {
    std::string escaped;
    for (char c : value) {
        if (c == '\'') {
            escaped += "''";
        } else {
            escaped += c;
        }
    }
    return "'" + escaped + "'";
}

std::string Database_Connection::get_generated_query(const std::string &query_template, const std::vector<std::string> &values) {

    std::ostringstream result;
    size_t pos = 0;
    size_t val_index = 0;

    while (pos < query_template.length()) {
        if (query_template[pos] == '?') {
            if (val_index >= values.size()) {
                return "Error generating query";
            }
            result << escape_sql_string(values[val_index++]);
            pos++;
        } else {
            result << query_template[pos++];
        }
    }

    if (val_index < values.size()) {
        return "Error generating query";
    }

    return result.str();
}

bool Database_Connection::is_connected() {
    return this->connected;
}




