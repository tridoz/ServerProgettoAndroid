//
// Created by trida on 5/14/25.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>



class Logger {

private:
    static std::string get_time();

public:
    static void server_log( const std::string& message );
    static void user_log( const std::string& username, const std::string& message );
    static void database_log( const std::string& message );
    static void server_error_log( const std::string& message );
    static void user_error_log( const std::string& username, const std::string& message );
    static void database_error_log( const std::string& message, const std::string& exception_message );

};



#endif //LOGGER_H
