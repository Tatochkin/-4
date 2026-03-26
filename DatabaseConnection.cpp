#include "DatabaseConnection.h"
#include <iostream>

void DatabaseConnection::checkError(SQLRETURN ret, SQLHANDLE handle, SQLSMALLINT type) {
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) return;

    SQLWCHAR sqlState[6];
    SQLWCHAR message[256];
    SQLINTEGER nativeError;
    SQLSMALLINT messageLen;

    SQLGetDiagRecW(type, handle, 1, sqlState, &nativeError, message, 256, &messageLen);

    std::wstring errorMsg = L"SQL Error: ";
    errorMsg += message;

    throw std::runtime_error(std::string(errorMsg.begin(), errorMsg.end()));
}

DatabaseConnection::DatabaseConnection() : m_env(nullptr), m_dbc(nullptr), m_stmt(nullptr), m_connected(false) {
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_env);
    SQLSetEnvAttr(m_env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, m_env, &m_dbc);
}

DatabaseConnection::~DatabaseConnection() {
    disconnect();
    if (m_dbc) SQLFreeHandle(SQL_HANDLE_DBC, m_dbc);
    if (m_env) SQLFreeHandle(SQL_HANDLE_ENV, m_env);
}

bool DatabaseConnection::connect(const std::wstring& server, const std::wstring& database,
    const std::wstring& user, const std::wstring& password) {

    // Варианты серверов для SQL Server Express
    std::vector<std::wstring> serversToTry;

    if (server.empty() || server == L"localhost" || server == L"." || server == L"(local)") {
        serversToTry.push_back(L"localhost\\SQLEXPRESS");
        serversToTry.push_back(L".\\SQLEXPRESS");
        serversToTry.push_back(L"(local)\\SQLEXPRESS");
        serversToTry.push_back(L"localhost");
        serversToTry.push_back(L".");
        serversToTry.push_back(L"(local)");

        // Получаем имя компьютера
        wchar_t computerName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
        if (GetComputerNameW(computerName, &size)) {
            serversToTry.push_back(std::wstring(computerName) + L"\\SQLEXPRESS");
            serversToTry.push_back(computerName);
        }
    }
    else {
        serversToTry.push_back(server);
    }

    // Список драйверов для проб
    const wchar_t* driversToTry[] = {
        L"ODBC Driver 17 for SQL Server",
        L"ODBC Driver 13 for SQL Server",
        L"SQL Server Native Client 11.0",
        L"SQL Server Native Client 10.0",
        L"SQL Server"
    };

    // Пробуем все комбинации
    for (const auto& serverName : serversToTry) {
        for (int i = 0; i < sizeof(driversToTry) / sizeof(driversToTry[0]); i++) {
            std::wstring connStr;

            if (user.empty()) {
                connStr = L"DRIVER={" + std::wstring(driversToTry[i]) + L"};SERVER=" + serverName +
                    L";DATABASE=" + database + L";Trusted_Connection=yes;";
            }
            else {
                connStr = L"DRIVER={" + std::wstring(driversToTry[i]) + L"};SERVER=" + serverName +
                    L";DATABASE=" + database + L";UID=" + user + L";PWD=" + password + L";";
            }

            SQLRETURN ret = SQLDriverConnectW(m_dbc, NULL, (SQLWCHAR*)connStr.c_str(), SQL_NTS,
                NULL, 0, NULL, SQL_DRIVER_COMPLETE);

            if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
                m_connected = true;
                SQLAllocHandle(SQL_HANDLE_STMT, m_dbc, &m_stmt);
                return true;
            }

            // Очищаем соединение для следующей попытки
            SQLDisconnect(m_dbc);
        }
    }

    return false;
}

void DatabaseConnection::disconnect() {
    if (m_stmt) {
        SQLFreeHandle(SQL_HANDLE_STMT, m_stmt);
        m_stmt = nullptr;
    }
    if (m_dbc && m_connected) {
        SQLDisconnect(m_dbc);
        m_connected = false;
    }
}

void DatabaseConnection::executeDirect(const std::wstring& query) {
    SQLRETURN ret = SQLExecDirectW(m_stmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    checkError(ret, m_stmt, SQL_HANDLE_STMT);
}

SQLHSTMT DatabaseConnection::prepareStatement(const std::wstring& query) {
    if (m_stmt) {
        SQLFreeHandle(SQL_HANDLE_STMT, m_stmt);
        m_stmt = nullptr;
    }
    SQLAllocHandle(SQL_HANDLE_STMT, m_dbc, &m_stmt);
    SQLPrepareW(m_stmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    return m_stmt;
}