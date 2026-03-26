#pragma once
#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include "DatabaseConnection.h"
#include "Models.h"
#include <string>

class AuthManager {
private:
    DatabaseConnection* m_db;
    User m_currentUser;
    Role m_currentRole;
    bool m_authenticated;

    std::wstring hashPassword(const std::wstring& password);

public:
    AuthManager(DatabaseConnection* db);

    bool login(const std::wstring& username, const std::wstring& password);
    void logout();

    bool hasPermission(bool canDo) const;
    bool canCreate() const { return m_currentRole.canCreate; }
    bool canRead() const { return m_currentRole.canRead; }
    bool canUpdate() const { return m_currentRole.canUpdate; }
    bool canDelete() const { return m_currentRole.canDelete; }

    bool isAuthenticated() const { return m_authenticated; }
    const User& getCurrentUser() const { return m_currentUser; }
    const Role& getCurrentRole() const { return m_currentRole; }
};

#endif