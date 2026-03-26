#pragma once
#ifndef DATABASE_CONNECTION_H
#define DATABASE_CONNECTION_H

#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <stdexcept>

class DatabaseConnection {
private:
    SQLHENV m_env;
    SQLHDBC m_dbc;
    SQLHSTMT m_stmt;
    bool m_connected;

    void checkError(SQLRETURN ret, SQLHANDLE handle, SQLSMALLINT type);

public:
    DatabaseConnection();
    ~DatabaseConnection();

    bool connect(const std::wstring& server, const std::wstring& database,
        const std::wstring& user = L"", const std::wstring& password = L"");
    void disconnect();

    SQLHDBC getConnection() const { return m_dbc; }
    SQLHSTMT getStatement() const { return m_stmt; }
    bool isConnected() const { return m_connected; }

    void executeDirect(const std::wstring& query);
    SQLHSTMT prepareStatement(const std::wstring& query);
};

#endif