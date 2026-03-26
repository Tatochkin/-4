#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "DatabaseConnection.h"
#include "Models.h"
#include "Utils.h"
#include <vector>
#include <tuple>
#include <memory>

class Repository {
protected:
    DatabaseConnection* m_db;

public:
    Repository(DatabaseConnection* db) : m_db(db) {}
    virtual ~Repository() = default;
};

class EmployeeRepository : public Repository {
public:
    EmployeeRepository(DatabaseConnection* db) : Repository(db) {}

    bool create(const Employee& employee);
    Employee getById(int id);
    std::vector<Employee> getAll();
    bool update(const Employee& employee);
    bool remove(int id);

    std::vector<Employee> search(int departmentId, double minSalary, double maxSalary,
        const std::wstring& hireDateFrom, const std::wstring& hireDateTo,
        const std::wstring& position, const std::wstring& searchText);

    std::vector<Employee> getPaged(int pageNumber, int pageSize, const std::wstring& sortColumn,
        const std::wstring& sortDirection, int& totalCount);
};

class ProductRepository : public Repository {
public:
    ProductRepository(DatabaseConnection* db) : Repository(db) {}

    bool create(const Product& product);
    Product getById(int id);
    std::vector<Product> getAll();
    bool update(const Product& product);
    bool remove(int id);
    bool checkAvailability(int productId, int quantity);
};

class OrderRepository : public Repository {
public:
    OrderRepository(DatabaseConnection* db) : Repository(db) {}

    int createOrder(const std::wstring& customerName, const std::wstring& customerPhone,
        const std::wstring& customerEmail, int employeeId,
        const std::vector<OrderItem>& items, std::wstring& errorMessage);

    Order getById(int id);
    std::vector<Order> getAll();
    bool updateStatus(int orderId, int statusId);

    std::vector<std::tuple<int, std::wstring, std::wstring, double, int, double>>
        getTopProducts(const std::wstring& startDate, const std::wstring& endDate, int topCount);
};

#endif