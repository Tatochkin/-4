#include "AuthManager.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

// Простая хеш-функция (в реальном проекте используйте SHA256)
std::wstring AuthManager::hashPassword(const std::wstring& password) {
    std::hash<std::wstring> hasher;
    size_t hash = hasher(password);
    std::wstringstream ss;
    ss << std::hex << std::setfill(L'0') << std::setw(16) << hash;
    return ss.str();
}

AuthManager::AuthManager(DatabaseConnection* db) : m_db(db), m_authenticated(false) {
    m_currentUser.userId = -1;
    m_currentRole.roleId = -1;
}

bool AuthManager::login(const std::wstring& username, const std::wstring& password) {
    if (!m_db || !m_db->isConnected()) return false;

    std::wstring hashedPass = hashPassword(password);

    std::wstring query = L"SELECT u.UserID, u.Username, u.Email, u.RoleID, u.IsActive, "
        L"r.RoleName, r.CanCreate, r.CanRead, r.CanUpdate, r.CanDelete "
        L"FROM Users u "
        L"INNER JOIN Roles r ON u.RoleID = r.RoleID "
        L"WHERE u.Username = ? AND u.PasswordHash = ? AND u.IsActive = 1";

    SQLHSTMT stmt = m_db->prepareStatement(query);

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0,
        (SQLWCHAR*)username.c_str(), 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 255, 0,
        (SQLWCHAR*)hashedPass.c_str(), 0, NULL);

    SQLRETURN ret = SQLExecute(stmt);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLWCHAR buffer[256];
        SQLLEN ind;

        if (SQLFetch(stmt) == SQL_SUCCESS) {
            SQLGetData(stmt, 1, SQL_C_LONG, &m_currentUser.userId, sizeof(int), &ind);
            SQLGetData(stmt, 2, SQL_C_WCHAR, buffer, sizeof(buffer), &ind);
            m_currentUser.username = buffer;
            SQLGetData(stmt, 3, SQL_C_WCHAR, buffer, sizeof(buffer), &ind);
            m_currentUser.email = buffer;
            SQLGetData(stmt, 4, SQL_C_LONG, &m_currentUser.roleId, sizeof(int), &ind);
            SQLGetData(stmt, 5, SQL_C_LONG, &m_currentUser.isActive, sizeof(bool), &ind);

            SQLGetData(stmt, 6, SQL_C_WCHAR, buffer, sizeof(buffer), &ind);
            m_currentRole.roleName = buffer;
            SQLGetData(stmt, 7, SQL_C_LONG, &m_currentRole.canCreate, sizeof(bool), &ind);
            SQLGetData(stmt, 8, SQL_C_LONG, &m_currentRole.canRead, sizeof(bool), &ind);
            SQLGetData(stmt, 9, SQL_C_LONG, &m_currentRole.canUpdate, sizeof(bool), &ind);
            SQLGetData(stmt, 10, SQL_C_LONG, &m_currentRole.canDelete, sizeof(bool), &ind);

            m_currentRole.roleId = m_currentUser.roleId;
            m_authenticated = true;

            // Обновление времени последнего входа
            std::wstring updateQuery = L"UPDATE Users SET LastLogin = GETDATE() WHERE UserID = ?";
            SQLHSTMT updateStmt = m_db->prepareStatement(updateQuery);
            SQLBindParameter(updateStmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
                &m_currentUser.userId, 0, NULL);
            SQLExecute(updateStmt);

            SQLFreeHandle(SQL_HANDLE_STMT, updateStmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            return true;
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return false;
}

void AuthManager::logout() {
    m_authenticated = false;
    m_currentUser.userId = -1;
    m_currentRole.roleId = -1;
}

bool AuthManager::hasPermission(bool canDo) const {
    return m_authenticated && canDo;
}