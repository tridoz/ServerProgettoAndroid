//
// Created by trida on 4/1/25.
//

#include "Database_Connection.h"


Database_Connection::Database_Connection() {
    this->driver = sql::mysql::get_mysql_driver_instance();
    this->connection.reset( this->driver->connect(this->host, this->username, this->password) );
    this->connection->setSchema(this->db_name);
    this->stmt.reset(this->connection->createStatement() );
}

int Database_Connection::find_playlist_id(std::string playlist_name) {

}

int Database_Connection::find_song_id(std::string song_name) {

}

std::string Database_Connection::get_user_playlist(int user_id, std::string username) {

     this->pstmt.reset( this->connection->prepareStatement("SELECT Title FROM Playlist WHERE Id_Utente = ?") );
    this->pstmt->setInt(1, user_id);

    this->result.reset( this->pstmt->executeQuery() );

    if ( !this->result->next() ) {
        return "[no playlist found]";
    }

    std::string playlist_set = "[";

    do {
        std::string title = this->result->getString("Title");
        playlist_set += title + "|" + username +"&";
    } while ( this->result->next() );

    playlist_set += "]";
    return playlist_set;
}

int Database_Connection::find_user_id(std::string username) {
    this->pstmt.reset(this->connection->prepareStatement("SELECT Id FROM Users WHERE Username = ?") );
    this->pstmt->setString(1, username);

    this->result.reset( this->pstmt->executeQuery() );

    this->result->last();
    if( this->result->getRow() == 1){
        return this->result->getInt("Id");
    }

    return -1;

}

int Database_Connection::add_token(std::string access_token, std::string refresh_token, std::string revoke_token, std::string username) {

    int userId = find_user_id(username);

    if(userId == -1) {
        return -1;
    }

    this->pstmt.reset( this->connection->prepareStatement("INSERT INTO Tokens VALUES(?,?,?,?)") );
    this->pstmt->setString(1, access_token);
    this->pstmt->setString(2, refresh_token);
    this->pstmt->setString(3, revoke_token);
    this->pstmt->setInt(4, userId);

    this->pstmt->executeUpdate();
    return 1;

}

int Database_Connection::add_user(std::string username, std::string password) {
    std::string hashed_password = calculate_hash(password);

    this->pstmt.reset( this->connection->prepareStatement("SELECT Id from Users ORDER BY Id ASC") );
    this->result.reset( this->pstmt->executeQuery() );

    int id = 1;
    if ( this->result->next()) {
        this->result->last();
        id = this->result->getInt("Id");
    }


    this->pstmt.reset( this->connection->prepareStatement("INSERT INTO Users VALUES(?,?,?)") );
    this->pstmt->setInt(1, id);
    this->pstmt->setString(2, username);
    this->pstmt->setString(3, hashed_password);

    this->pstmt->executeUpdate();
    return 1;
}

int Database_Connection::check_credentials(std::string username, std::string password) {
    std::string hashed_password = calculate_hash(password);
    this->pstmt.reset(this->connection->prepareStatement("SELECT * FROM Users WHERE Username = ? AND Password = ?"));
    this->pstmt->setString(1, username);
    this->pstmt->setString(2, hashed_password);

    this->result.reset(this->pstmt->executeQuery() );
    this->result->last();
    if ( this->result->getRow() == 1)
        return 1;

    return -1;

}

int Database_Connection::delete_user_token(std::string username) {
    int id_utente = find_user_id(username);

    this->pstmt.reset( this->connection->prepareStatement("DELETE FROM Tokens WHERE Id_Utente = ?") );
    this->pstmt->setInt(1, id_utente);
    this->pstmt->executeUpdate();

    return 1;
}

int Database_Connection::check_if_token_valid(std::string username, std::string token, int token_type) {
    int id_utente = find_user_id(username);

    if ( id_utente == -1) {
        std::cout<<"Utente " << username << " non trovato nel database\n";
        return -1;
    }

    std::cout<<"Utente " << username << " trovato nel database con codice: " << id_utente <<"\n";


    std::string query ;

    switch (token_type) {
        case 1:
            query = "SELECT * FROM Tokens WHERE Id_Utente = ? AND Access_Token = ?";
            break;

        case 2:
            query = "SELECT * FROM Tokens WHERE Id_Utente = ? AND Refresh_token = ?";
        break;

        case 3:
            query = "SELECT * FROM Tokens WHERE Id_Utente = ? AND Revoke_token = ?";
            break;
    }

    this->pstmt.reset( this->connection->prepareStatement(query) );

    this->pstmt->setInt(1, id_utente);
    this->pstmt->setString(2, token);

    this->result.reset( this->pstmt->executeQuery() );

    sql::ResultSetMetaData* meta = this->result->getMetaData();
    int columnCount = meta->getColumnCount();

    this->result->last();
    if ( this->result->getRow() == 1) {
        return 1;
    }

    return -1;
}

int Database_Connection::check_token_generation(std::string accesso_token, std::string refresh_token, std::string revoke_token) {
    this->pstmt.reset(this->connection->prepareStatement(
        "SELECT * FROM Tokens WHERE "
        "Access_Token = ? OR Access_Token = ? OR Access_Token = ? OR "
        "Refresh_Token = ? OR Refresh_Token = ? OR Refresh_Token = ? OR "
        "Revoke_Token = ? OR Revoke_Token = ? OR Revoke_Token = ?"
    ));

    // Assegna le variabili ai parametri della query
    this->pstmt->setString(1, accesso_token);
    this->pstmt->setString(2, refresh_token);
    this->pstmt->setString(3, revoke_token);
    this->pstmt->setString(4, accesso_token);
    this->pstmt->setString(5, refresh_token);
    this->pstmt->setString(6, revoke_token);
    this->pstmt->setString(7, accesso_token);
    this->pstmt->setString(8, refresh_token);
    this->pstmt->setString(9, revoke_token);

    this->result.reset(this->pstmt->executeQuery());

    if ( !this->result->next() ) {
        return 1;
    }


    return -1;

}

int Database_Connection::check_if_user_exists(std::string username) {
    this->pstmt.reset( this->connection->prepareStatement("SELECT * FROM Users WHERE Username = ?"));
    this->pstmt->setString(1, username);
    this->result.reset(this->pstmt->executeQuery());

    this->result->last();
    if ( this->result->getRow() == 1 )
        return 1;

    return -1;
}

int Database_Connection::add_song_to_liked(std::string username, std::string song_name) {

}

int Database_Connection::add_song_to_playlist(std::string playlist, std::string song_name, std::string username) {

}

int Database_Connection::delete_token(std::string access_token, std::string refresh_token, std::string revoke_token) {

}

int Database_Connection::get_playlist_content(std::string playlist_name) {

}

int Database_Connection::search_artists(std::string artist_name) {

}

int Database_Connection::search_playlists(std::string playlist_name) {

}

int Database_Connection::search_song_in_playlist(std::string username, std::string playlist_name, std::string song_name) {

}

int Database_Connection::search_songs(std::string song_name) {

}

int Database_Connection::search_users(std::string user_name) {

}

int Database_Connection::create_playlist(std::string username, std::string playlist_name, bool isPrivate) {

}

std::string Database_Connection::calculate_hash(std::string input) {

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.c_str(), input.size());
    SHA256_Final(hash, &sha256);

    std::string output;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        char buf[3];
        sprintf(buf, "%02x", hash[i]);
        output += buf;
    }
    return output;
}


