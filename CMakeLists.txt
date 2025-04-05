cmake_minimum_required(VERSION 3.30)
project(ServerProgettoAndroid)

set(CMAKE_CXX_STANDARD 20)

# Trova OpenSSL
find_package(OpenSSL REQUIRED)

# Trova MySQL Connector/C++
find_library(MYSQLCPP_CONN NAMES mysqlcppconn)

if (NOT MYSQLCPP_CONN)
    message(FATAL_ERROR "MySQL Connector/C++ non trovato. Assicurati che sia installato!")
endif()

# Aggiungi l'eseguibile
add_executable(ServerProgettoAndroid
        main.cpp
        Database_Connection.cpp
        Database_Connection.h
        TCP_Server.cpp
        TCP_Server.h
)

# Linka MySQL Connector/C++ e OpenSSL
target_link_libraries(ServerProgettoAndroid ${MYSQLCPP_CONN} OpenSSL::SSL OpenSSL::Crypto)
