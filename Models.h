#pragma once
#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include <ctime>

struct Role {
    int roleId;
    std::wstring roleName;
    bool canCreate;
    bool canRead;
    bool canUpdate;
    bool canDelete;
    time_t createdAt;
};

struct User {
    int userId;
    std::wstring username;
    std::wstring email;
    int roleId;
    bool isActive;
    time_t createdAt;
    time_t lastLogin;
};

struct Department {
    int departmentId;
    std::wstring departmentName;
    std::wstring location;
    std::wstring phone;
    int managerId;
    time_t createdAt;
};

struct Employee {
    int employeeId;
    std::wstring firstName;
    std::wstring lastName;
    std::wstring patronymic;
    std::wstring position;
    double salary;
    std::wstring hireDate;
    std::wstring birthDate;
    std::wstring email;
    std::wstring phone;
    int departmentId;
    int userId;
    bool isActive;
};

struct Supplier {
    int supplierId;
    std::wstring supplierName;
    std::wstring contactPerson;
    std::wstring phone;
    std::wstring email;
    std::wstring address;
    std::wstring taxNumber;
    bool isActive;
};

struct ProductCategory {
    int categoryId;
    std::wstring categoryName;
    int parentCategoryId;
    std::wstring description;
};

struct Product {
    int productId;
    std::wstring productCode;
    std::wstring productName;
    int categoryId;
    int supplierId;
    double unitPrice;
    int stockQuantity;
    int reorderLevel;
    bool discontinued;
    time_t createdAt;
};

struct Order {
    int orderId;
    std::wstring orderNumber;
    std::wstring orderDate;
    std::wstring customerName;
    std::wstring customerPhone;
    std::wstring customerEmail;
    int employeeId;
    int statusId;
    double totalAmount;
    bool paymentStatus;
    std::wstring notes;
};

struct OrderDetail {
    int orderDetailId;
    int orderId;
    int productId;
    int quantity;
    double unitPrice;
    double discount;
};

struct OrderItem {
    int productId;
    int quantity;
    double unitPrice;
};

struct AuditLog {
    int auditId;
    std::wstring tableName;
    int recordId;
    std::wstring actionType;
    std::wstring oldData;
    std::wstring newData;
    int changedByUserId;
    time_t changedAt;
};

struct Warehouse {
    int warehouseId;
    std::wstring warehouseName;
    std::wstring location;
    int managerEmployeeId;
};

struct StockItem {
    int stockItemId;
    int warehouseId;
    int productId;
    int quantity;
    time_t lastUpdated;
};

#endif