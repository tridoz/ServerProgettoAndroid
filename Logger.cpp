//
// Created by trida on 5/14/25.
//

#include "Logger.h"

#include <chrono>
#include <cstring>

std::string Logger::get_time() {

    const auto now = std::chrono::system_clock::now();
    const std::time_t current_time = std::chrono::system_clock::to_time_t( now );

    const std::tm* local_time = std::localtime( &current_time );
    std::ostringstream oss;

    oss << std::put_time( local_time, "%Y-%m-%d %H:%M:%S" );
    return "[" + oss.str() + "]";

}

void Logger::server_log(const std::string &message ){
    std::ofstream file( "server.log", std::ios::app );
    std::string log = "LOG: " +  get_time() + "\n" + message + "\n\n";

    file << message;
    std::cout << message;

    file.close();
}

void Logger::server_error_log( const std::string& message ) {
    std::ofstream file( "server.log", std::ios::app );
    std::string cause = strerror( errno );

    std::string log = "ERR: " + get_time() + "\n" + message + "\n" + cause + "\n\n";
    file << log;
    std::cerr << log;
    file.close();
}

void Logger::user_log( const std::string& username, const std::string& message ) {
    std::ofstream file( username+".log", std::ios::app );

    const std::string log = "LOG: " + get_time() + "\n" + message + "\n\n";
    file << log;
    std::cout << log;

    file.close();
}

void Logger::user_error_log( const std::string& username, const std::string& message ) {
    std::ofstream file( username+".log", std::ios::app );
    std::string cause = strerror( errno );

    const std::string log = "ERR: " + get_time() + "\n" + message + "\n";
    file << log;
    std::cout << log;

    file.close();
}

void Logger::database_log( const std::string& message ) {
    std::ofstream file("database.log", std::ios::app);
    std::string log = "LOG: " + get_time() + "\n" + message + "\n\n";
    file << log;
    std::cout << log;
    file.close();
}

void Logger::database_error_log( const std::string& message, const std::string& exception_message ) {
    std::ofstream file("database.log", std::ios::app);

    std::string log = "ERR: " + get_time() + "\n" + message + "\n" + exception_message + "\n\n";
    file << log;
    std::cerr << log;

    file.close();
}