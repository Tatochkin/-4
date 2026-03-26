#include "Repository.h"
#include <iostream>
#include <string>
#include <tuple>
#include <sstream>

// EmployeeRepository implementation

bool EmployeeRepository::create(const Employee& employee) {
    if (!Validator::validateName(employee.firstName) || !Validator::validateName(employee.lastName)) {
        throw std::runtime_error("Некорректное имя или фамилия");
    }
    if (!Validator::validateEmail(employee.email)) {
        throw std::runtime_error("Некорректный email");
    }
    if (!Validator::validatePhone(employee.phone)) {
        throw std::runtime_error("Некорректный телефон");
    }
    if (!Validator::validateSalary(employee.salary)) {
        throw std::runtime_error("Некорректная зарплата");
    }
    if (!Validator::validateDate(employee.hireDate) || !Validator::validateDate(employee.birthDate)) {
        throw std::runtime_error("Некорректная дата");
    }

    std::wstring query = L"{CALL sp_EmployeeCRUD(?,?,?,?,?,?,?,?,?,?,?,?,?)}";
    SQLHSTMT stmt = m_db->prepareStatement(query);

    SQLWCHAR action[] = L"CREATE";
    SQLWCHAR firstName[51], lastName[51], patronymic[51], position[101];
    SQLWCHAR email[101], phone[21], hireDate[11], birthDate[11];

    wcscpy_s(firstName, 51, employee.firstName.c_str());
    wcscpy_s(lastName, 51, employee.lastName.c_str());
    wcscpy_s(patronymic, 51, employee.patronymic.c_str());
    wcscpy_s(position, 101, employee.position.c_str());
    wcscpy_s(email, 101, employee.email.c_str());
    wcscpy_s(phone, 21, employee.phone.c_str());
    wcscpy_s(hireDate, 11, employee.hireDate.c_str());
    wcscpy_s(birthDate, 11, employee.birthDate.c_str());

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, action, 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, NULL, 0, NULL);
    SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, firstName, 0, NULL);
    SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, lastName, 0, NULL);
    SQLBindParameter(stmt, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, patronymic, 0, NULL);
    SQLBindParameter(stmt, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, position, 0, NULL);
    SQLBindParameter(stmt, 7, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 12, 2, (void*)&employee.salary, 0, NULL);
    SQLBindParameter(stmt, 8, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, hireDate, 0, NULL);
    SQLBindParameter(stmt, 9, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, birthDate, 0, NULL);
    SQLBindParameter(stmt, 10, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, email, 0, NULL);
    SQLBindParameter(stmt, 11, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, phone, 0, NULL);
    SQLBindParameter(stmt, 12, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (void*)&employee.departmentId, 0, NULL);

    int newId = 0;
    SQLBindParameter(stmt, 13, SQL_PARAM_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &newId, 0, NULL);

    SQLRETURN ret = SQLExecute(stmt);

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

Employee EmployeeRepository::getById(int id) {
    Employee emp = {};
    emp.employeeId = -1;

    std::wstring query = L"SELECT EmployeeID, FirstName, LastName, Patronymic, Position, Salary, "
        L"CONVERT(VARCHAR(10), HireDate, 120), CONVERT(VARCHAR(10), BirthDate, 120), "
        L"Email, Phone, DepartmentID, UserID, IsActive FROM Employees WHERE EmployeeID = ? AND IsActive = 1";

    SQLHSTMT stmt = m_db->prepareStatement(query);
    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &id, 0, NULL);

    if (SQLExecute(stmt) == SQL_SUCCESS || SQLExecute(stmt) == SQL_SUCCESS_WITH_INFO) {
        if (SQLFetch(stmt) == SQL_SUCCESS) {
            SQLWCHAR buffer[256];
            SQLLEN ind;

            SQLGetData(stmt, 1, SQL_C_LONG, &emp.employeeId, sizeof(int), &ind);
            SQLGetData(stmt, 2, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.firstName = buffer;
            SQLGetData(stmt, 3, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.lastName = buffer;
            SQLGetData(stmt, 4, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.patronymic = buffer;
            SQLGetData(stmt, 5, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.position = buffer;
            SQLGetData(stmt, 6, SQL_C_DOUBLE, &emp.salary, sizeof(double), &ind);
            SQLGetData(stmt, 7, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.hireDate = buffer;
            SQLGetData(stmt, 8, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.birthDate = buffer;
            SQLGetData(stmt, 9, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.email = buffer;
            SQLGetData(stmt, 10, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.phone = buffer;
            SQLGetData(stmt, 11, SQL_C_LONG, &emp.departmentId, sizeof(int), &ind);
            SQLGetData(stmt, 12, SQL_C_LONG, &emp.userId, sizeof(int), &ind);
            SQLGetData(stmt, 13, SQL_C_LONG, &emp.isActive, sizeof(bool), &ind);
        }
    }

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return emp;
}

std::vector<Employee> EmployeeRepository::getAll() {
    std::vector<Employee> employees;
    std::wstring query = L"SELECT EmployeeID, FirstName, LastName, Patronymic, Position, Salary, "
        L"CONVERT(VARCHAR(10), HireDate, 120), CONVERT(VARCHAR(10), BirthDate, 120), "
        L"Email, Phone, DepartmentID, UserID, IsActive FROM Employees WHERE IsActive = 1";

    SQLHSTMT stmt = m_db->prepareStatement(query);

    if (SQLExecute(stmt) == SQL_SUCCESS || SQLExecute(stmt) == SQL_SUCCESS_WITH_INFO) {
        while (SQLFetch(stmt) == SQL_SUCCESS) {
            Employee emp;
            SQLWCHAR buffer[256];
            SQLLEN ind;

            SQLGetData(stmt, 1, SQL_C_LONG, &emp.employeeId, sizeof(int), &ind);
            SQLGetData(stmt, 2, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.firstName = buffer;
            SQLGetData(stmt, 3, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.lastName = buffer;
            SQLGetData(stmt, 4, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.patronymic = buffer;
            SQLGetData(stmt, 5, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.position = buffer;
            SQLGetData(stmt, 6, SQL_C_DOUBLE, &emp.salary, sizeof(double), &ind);
            SQLGetData(stmt, 7, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.hireDate = buffer;
            SQLGetData(stmt, 8, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.birthDate = buffer;
            SQLGetData(stmt, 9, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.email = buffer;
            SQLGetData(stmt, 10, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.phone = buffer;
            SQLGetData(stmt, 11, SQL_C_LONG, &emp.departmentId, sizeof(int), &ind);
            SQLGetData(stmt, 12, SQL_C_LONG, &emp.userId, sizeof(int), &ind);
            SQLGetData(stmt, 13, SQL_C_LONG, &emp.isActive, sizeof(bool), &ind);

            employees.push_back(emp);
        }
    }

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return employees;
}

bool EmployeeRepository::update(const Employee& employee) {
    if (!Validator::validateName(employee.firstName) || !Validator::validateName(employee.lastName)) {
        throw std::runtime_error("Некорректное имя или фамилия");
    }
    if (!Validator::validateEmail(employee.email)) {
        throw std::runtime_error("Некорректный email");
    }
    if (!Validator::validatePhone(employee.phone)) {
        throw std::runtime_error("Некорректный телефон");
    }
    if (!Validator::validateSalary(employee.salary)) {
        throw std::runtime_error("Некорректная зарплата");
    }

    std::wstring query = L"UPDATE Employees SET FirstName = ?, LastName = ?, Patronymic = ?, "
        L"Position = ?, Salary = ?, HireDate = ?, BirthDate = ?, "
        L"Email = ?, Phone = ?, DepartmentID = ? WHERE EmployeeID = ?";

    SQLHSTMT stmt = m_db->prepareStatement(query);

    SQLWCHAR firstName[51], lastName[51], patronymic[51], position[101];
    SQLWCHAR email[101], phone[21], hireDate[11], birthDate[11];

    wcscpy_s(firstName, 51, employee.firstName.c_str());
    wcscpy_s(lastName, 51, employee.lastName.c_str());
    wcscpy_s(patronymic, 51, employee.patronymic.c_str());
    wcscpy_s(position, 101, employee.position.c_str());
    wcscpy_s(email, 101, employee.email.c_str());
    wcscpy_s(phone, 21, employee.phone.c_str());
    wcscpy_s(hireDate, 11, employee.hireDate.c_str());
    wcscpy_s(birthDate, 11, employee.birthDate.c_str());

    int empId = employee.employeeId;

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, firstName, 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, lastName, 0, NULL);
    SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, patronymic, 0, NULL);
    SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, position, 0, NULL);
    SQLBindParameter(stmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 12, 2, (void*)&employee.salary, 0, NULL);
    SQLBindParameter(stmt, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, hireDate, 0, NULL);
    SQLBindParameter(stmt, 7, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, birthDate, 0, NULL);
    SQLBindParameter(stmt, 8, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, email, 0, NULL);
    SQLBindParameter(stmt, 9, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, phone, 0, NULL);
    SQLBindParameter(stmt, 10, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (void*)&employee.departmentId, 0, NULL);
    SQLBindParameter(stmt, 11, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &empId, 0, NULL);

    SQLRETURN ret = SQLExecute(stmt);

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

bool EmployeeRepository::remove(int id) {
    std::wstring query = L"UPDATE Employees SET IsActive = 0 WHERE EmployeeID = ?";
    SQLHSTMT stmt = m_db->prepareStatement(query);

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &id, 0, NULL);

    SQLRETURN ret = SQLExecute(stmt);

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

std::vector<Employee> EmployeeRepository::search(int departmentId, double minSalary, double maxSalary,
    const std::wstring& hireDateFrom, const std::wstring& hireDateTo,
    const std::wstring& position, const std::wstring& searchText) {
    std::vector<Employee> employees;
    std::wstring query = L"{CALL sp_SearchEmployees(?,?,?,?,?,?,?)}";
    SQLHSTMT stmt = m_db->prepareStatement(query);

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &departmentId, 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 12, 2, (void*)&minSalary, 0, NULL);
    SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 12, 2, (void*)&maxSalary, 0, NULL);

    SQLWCHAR fromDate[11] = { 0 }, toDate[11] = { 0 }, pos[101] = { 0 }, search[101] = { 0 };
    wcscpy_s(fromDate, 11, hireDateFrom.c_str());
    wcscpy_s(toDate, 11, hireDateTo.c_str());
    wcscpy_s(pos, 101, position.c_str());
    wcscpy_s(search, 101, searchText.c_str());

    SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, fromDate, 0, NULL);
    SQLBindParameter(stmt, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, toDate, 0, NULL);
    SQLBindParameter(stmt, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, pos, 0, NULL);
    SQLBindParameter(stmt, 7, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, search, 0, NULL);

    if (SQLExecute(stmt) == SQL_SUCCESS || SQLExecute(stmt) == SQL_SUCCESS_WITH_INFO) {
        while (SQLFetch(stmt) == SQL_SUCCESS) {
            Employee emp;
            SQLWCHAR buffer[256];
            SQLLEN ind;

            SQLGetData(stmt, 1, SQL_C_LONG, &emp.employeeId, sizeof(int), &ind);
            SQLGetData(stmt, 2, SQL_C_WCHAR, buffer, sizeof(buffer), &ind);
            // Полное имя пропускаем
            SQLGetData(stmt, 3, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.position = buffer;
            SQLGetData(stmt, 4, SQL_C_DOUBLE, &emp.salary, sizeof(double), &ind);
            SQLGetData(stmt, 5, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.hireDate = buffer;
            SQLGetData(stmt, 6, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.email = buffer;
            SQLGetData(stmt, 7, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.phone = buffer;

            employees.push_back(emp);
        }
    }

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return employees;
}

std::vector<Employee> EmployeeRepository::getPaged(int pageNumber, int pageSize, const std::wstring& sortColumn,
    const std::wstring& sortDirection, int& totalCount) {
    std::vector<Employee> employees;
    totalCount = 0;

    std::wstring query = L"{CALL sp_GetEmployeesPaged(?,?,?,?)}";
    SQLHSTMT stmt = m_db->prepareStatement(query);

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &pageNumber, 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &pageSize, 0, NULL);

    SQLWCHAR sortCol[51], sortDir[5];
    wcscpy_s(sortCol, 51, sortColumn.c_str());
    wcscpy_s(sortDir, 5, sortDirection.c_str());

    SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, sortCol, 0, NULL);
    SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 4, 0, sortDir, 0, NULL);

    if (SQLExecute(stmt) == SQL_SUCCESS || SQLExecute(stmt) == SQL_SUCCESS_WITH_INFO) {
        while (SQLFetch(stmt) == SQL_SUCCESS) {
            Employee emp;
            SQLWCHAR buffer[256];
            SQLLEN ind;

            SQLGetData(stmt, 1, SQL_C_LONG, &emp.employeeId, sizeof(int), &ind);
            SQLGetData(stmt, 2, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.lastName = buffer;
            SQLGetData(stmt, 3, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.firstName = buffer;
            SQLGetData(stmt, 4, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.patronymic = buffer;
            SQLGetData(stmt, 5, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.position = buffer;
            SQLGetData(stmt, 6, SQL_C_DOUBLE, &emp.salary, sizeof(double), &ind);
            SQLGetData(stmt, 7, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.hireDate = buffer;
            SQLGetData(stmt, 8, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.email = buffer;
            SQLGetData(stmt, 9, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); emp.phone = buffer;
            SQLGetData(stmt, 10, SQL_C_LONG, &totalCount, sizeof(int), &ind);

            employees.push_back(emp);
        }
    }

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return employees;
}

// ProductRepository implementation

bool ProductRepository::create(const Product& product) {
    if (!Validator::validateProductCode(product.productCode)) {
        throw std::runtime_error("Некорректный код товара");
    }
    if (!Validator::validatePositiveNumber(product.unitPrice)) {
        throw std::runtime_error("Некорректная цена");
    }

    std::wstring query = L"INSERT INTO Products (ProductCode, ProductName, CategoryID, SupplierID, "
        L"UnitPrice, StockQuantity, ReorderLevel, Discontinued) "
        L"VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

    SQLHSTMT stmt = m_db->prepareStatement(query);

    SQLWCHAR code[21], name[101];
    wcscpy_s(code, 21, product.productCode.c_str());
    wcscpy_s(name, 101, product.productName.c_str());

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, code, 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, name, 0, NULL);
    SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (void*)&product.categoryId, 0, NULL);
    SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (void*)&product.supplierId, 0, NULL);
    SQLBindParameter(stmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 12, 2, (void*)&product.unitPrice, 0, NULL);
    SQLBindParameter(stmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (void*)&product.stockQuantity, 0, NULL);
    SQLBindParameter(stmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (void*)&product.reorderLevel, 0, NULL);
    SQLBindParameter(stmt, 8, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (void*)&product.discontinued, 0, NULL);

    SQLRETURN ret = SQLExecute(stmt);

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

Product ProductRepository::getById(int id) {
    Product prod = {};
    prod.productId = -1;

    std::wstring query = L"SELECT ProductID, ProductCode, ProductName, CategoryID, SupplierID, "
        L"UnitPrice, StockQuantity, ReorderLevel, Discontinued "
        L"FROM Products WHERE ProductID = ?";

    SQLHSTMT stmt = m_db->prepareStatement(query);
    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &id, 0, NULL);

    if (SQLExecute(stmt) == SQL_SUCCESS || SQLExecute(stmt) == SQL_SUCCESS_WITH_INFO) {
        if (SQLFetch(stmt) == SQL_SUCCESS) {
            SQLWCHAR buffer[256];
            SQLLEN ind;

            SQLGetData(stmt, 1, SQL_C_LONG, &prod.productId, sizeof(int), &ind);
            SQLGetData(stmt, 2, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); prod.productCode = buffer;
            SQLGetData(stmt, 3, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); prod.productName = buffer;
            SQLGetData(stmt, 4, SQL_C_LONG, &prod.categoryId, sizeof(int), &ind);
            SQLGetData(stmt, 5, SQL_C_LONG, &prod.supplierId, sizeof(int), &ind);
            SQLGetData(stmt, 6, SQL_C_DOUBLE, &prod.unitPrice, sizeof(double), &ind);
            SQLGetData(stmt, 7, SQL_C_LONG, &prod.stockQuantity, sizeof(int), &ind);
            SQLGetData(stmt, 8, SQL_C_LONG, &prod.reorderLevel, sizeof(int), &ind);
            SQLGetData(stmt, 9, SQL_C_LONG, &prod.discontinued, sizeof(bool), &ind);
        }
    }

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return prod;
}

std::vector<Product> ProductRepository::getAll() {
    std::vector<Product> products;
    std::wstring query = L"SELECT ProductID, ProductCode, ProductName, CategoryID, SupplierID, "
        L"UnitPrice, StockQuantity, ReorderLevel, Discontinued "
        L"FROM Products WHERE Discontinued = 0";

    SQLHSTMT stmt = m_db->prepareStatement(query);

    if (SQLExecute(stmt) == SQL_SUCCESS || SQLExecute(stmt) == SQL_SUCCESS_WITH_INFO) {
        while (SQLFetch(stmt) == SQL_SUCCESS) {
            Product prod;
            SQLWCHAR buffer[256];
            SQLLEN ind;

            SQLGetData(stmt, 1, SQL_C_LONG, &prod.productId, sizeof(int), &ind);
            SQLGetData(stmt, 2, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); prod.productCode = buffer;
            SQLGetData(stmt, 3, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); prod.productName = buffer;
            SQLGetData(stmt, 4, SQL_C_LONG, &prod.categoryId, sizeof(int), &ind);
            SQLGetData(stmt, 5, SQL_C_LONG, &prod.supplierId, sizeof(int), &ind);
            SQLGetData(stmt, 6, SQL_C_DOUBLE, &prod.unitPrice, sizeof(double), &ind);
            SQLGetData(stmt, 7, SQL_C_LONG, &prod.stockQuantity, sizeof(int), &ind);
            SQLGetData(stmt, 8, SQL_C_LONG, &prod.reorderLevel, sizeof(int), &ind);
            SQLGetData(stmt, 9, SQL_C_LONG, &prod.discontinued, sizeof(bool), &ind);

            products.push_back(prod);
        }
    }

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return products;
}

bool ProductRepository::update(const Product& product) {
    std::wstring query = L"UPDATE Products SET ProductCode = ?, ProductName = ?, CategoryID = ?, "
        L"SupplierID = ?, UnitPrice = ?, StockQuantity = ?, ReorderLevel = ?, "
        L"Discontinued = ? WHERE ProductID = ?";

    SQLHSTMT stmt = m_db->prepareStatement(query);

    SQLWCHAR code[21], name[101];
    wcscpy_s(code, 21, product.productCode.c_str());
    wcscpy_s(name, 101, product.productName.c_str());

    int prodId = product.productId;

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, code, 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, name, 0, NULL);
    SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (void*)&product.categoryId, 0, NULL);
    SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (void*)&product.supplierId, 0, NULL);
    SQLBindParameter(stmt, 5, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 12, 2, (void*)&product.unitPrice, 0, NULL);
    SQLBindParameter(stmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (void*)&product.stockQuantity, 0, NULL);
    SQLBindParameter(stmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (void*)&product.reorderLevel, 0, NULL);
    SQLBindParameter(stmt, 8, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, (void*)&product.discontinued, 0, NULL);
    SQLBindParameter(stmt, 9, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &prodId, 0, NULL);

    SQLRETURN ret = SQLExecute(stmt);

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

bool ProductRepository::remove(int id) {
    std::wstring query = L"UPDATE Products SET Discontinued = 1 WHERE ProductID = ?";
    SQLHSTMT stmt = m_db->prepareStatement(query);

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &id, 0, NULL);

    SQLRETURN ret = SQLExecute(stmt);

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

bool ProductRepository::checkAvailability(int productId, int quantity) {
    std::wstring query = L"SELECT dbo.fn_CheckProductAvailability(?, ?)";
    SQLHSTMT stmt = m_db->prepareStatement(query);

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &productId, 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &quantity, 0, NULL);

    int result = 0;

    if (SQLExecute(stmt) == SQL_SUCCESS || SQLExecute(stmt) == SQL_SUCCESS_WITH_INFO) {
        if (SQLFetch(stmt) == SQL_SUCCESS) {
            SQLGetData(stmt, 1, SQL_C_LONG, &result, sizeof(int), NULL);
        }
    }

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return result == 1;
}

// OrderRepository implementation

int OrderRepository::createOrder(const std::wstring& customerName, const std::wstring& customerPhone,
    const std::wstring& customerEmail, int employeeId,
    const std::vector<OrderItem>& items, std::wstring& errorMessage) {
    if (!Validator::validatePhone(customerPhone)) {
        errorMessage = L"Некорректный номер телефона";
        return -1;
    }
    if (!customerEmail.empty() && !Validator::validateEmail(customerEmail)) {
        errorMessage = L"Некорректный email";
        return -1;
    }
    if (items.empty()) {
        errorMessage = L"Заказ не может быть пустым";
        return -1;
    }

    std::wstring query = L"{CALL sp_CreateOrder(?,?,?,?,?,?,?)}";
    SQLHSTMT stmt = m_db->prepareStatement(query);

    SQLWCHAR name[101], phone[21], email[101];
    wcscpy_s(name, 101, customerName.c_str());
    wcscpy_s(phone, 21, customerPhone.c_str());
    wcscpy_s(email, 101, customerEmail.c_str());

    // Формируем JSON для товаров
    std::wstringstream jsonStream;
    jsonStream << L"[";
    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0) jsonStream << L",";
        jsonStream << L"{\"ProductID\":" << items[i].productId
            << L",\"Quantity\":" << items[i].quantity
            << L",\"UnitPrice\":" << items[i].unitPrice << L"}";
    }
    jsonStream << L"]";
    std::wstring itemsJson = jsonStream.str();

    int orderId = -1;
    SQLWCHAR errorMsg[501] = { 0 };
    SQLLEN errorLen = 0;

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, name, 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 20, 0, phone, 0, NULL);
    SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, email, 0, NULL);
    SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &employeeId, 0, NULL);
    SQLBindParameter(stmt, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, itemsJson.length() * 2 + 2, 0,
        (SQLWCHAR*)itemsJson.c_str(), 0, NULL);
    SQLBindParameter(stmt, 6, SQL_PARAM_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &orderId, 0, NULL);
    SQLBindParameter(stmt, 7, SQL_PARAM_OUTPUT, SQL_C_WCHAR, SQL_WVARCHAR, 500, 0, errorMsg, 500, &errorLen);

    SQLRETURN ret = SQLExecute(stmt);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        errorMessage = errorMsg;
    }
    else {
        errorMessage = L"Ошибка при выполнении хранимой процедуры";
        orderId = -1;
    }

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    return orderId;
}

std::vector<std::tuple<int, std::wstring, std::wstring, double, int, double>>
OrderRepository::getTopProducts(const std::wstring& startDate, const std::wstring& endDate, int topCount) {
    std::vector<std::tuple<int, std::wstring, std::wstring, double, int, double>> results;

    std::wstring query = L"{CALL sp_GetTop5Products(?,?,?)}";
    SQLHSTMT stmt = m_db->prepareStatement(query);

    SQLWCHAR fromDate[11], toDate[11];
    wcscpy_s(fromDate, 11, startDate.c_str());
    wcscpy_s(toDate, 11, endDate.c_str());

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, fromDate, 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 10, 0, toDate, 0, NULL);
    SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &topCount, 0, NULL);

    if (SQLExecute(stmt) == SQL_SUCCESS || SQLExecute(stmt) == SQL_SUCCESS_WITH_INFO) {
        while (SQLFetch(stmt) == SQL_SUCCESS) {
            int productId;
            SQLWCHAR productCode[50], productName[100];
            double totalRevenue;
            int ordersCount;
            double avgOrderValue;
            SQLLEN ind;

            SQLGetData(stmt, 1, SQL_C_LONG, &productId, sizeof(int), &ind);
            SQLGetData(stmt, 2, SQL_C_WCHAR, productCode, sizeof(productCode), &ind);
            SQLGetData(stmt, 3, SQL_C_WCHAR, productName, sizeof(productName), &ind);
            SQLGetData(stmt, 4, SQL_C_DOUBLE, &totalRevenue, sizeof(double), &ind);
            SQLGetData(stmt, 5, SQL_C_LONG, &ordersCount, sizeof(int), &ind);
            SQLGetData(stmt, 6, SQL_C_DOUBLE, &avgOrderValue, sizeof(double), &ind);

            results.push_back(std::make_tuple(productId, productCode, productName, totalRevenue, ordersCount, avgOrderValue));
        }
    }

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return results;
}

Order OrderRepository::getById(int id) {
    Order order = {};
    order.orderId = -1;

    std::wstring query = L"SELECT OrderID, OrderNumber, OrderDate, CustomerName, CustomerPhone, "
        L"CustomerEmail, EmployeeID, StatusID, TotalAmount, PaymentStatus, Notes "
        L"FROM Orders WHERE OrderID = ?";

    SQLHSTMT stmt = m_db->prepareStatement(query);
    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &id, 0, NULL);

    if (SQLExecute(stmt) == SQL_SUCCESS || SQLExecute(stmt) == SQL_SUCCESS_WITH_INFO) {
        if (SQLFetch(stmt) == SQL_SUCCESS) {
            SQLWCHAR buffer[256];
            SQLLEN ind;
            SQL_TIMESTAMP_STRUCT ts;

            SQLGetData(stmt, 1, SQL_C_LONG, &order.orderId, sizeof(int), &ind);
            SQLGetData(stmt, 2, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); order.orderNumber = buffer;
            SQLGetData(stmt, 3, SQL_C_TYPE_TIMESTAMP, &ts, sizeof(ts), &ind);
            SQLGetData(stmt, 4, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); order.customerName = buffer;
            SQLGetData(stmt, 5, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); order.customerPhone = buffer;
            SQLGetData(stmt, 6, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); order.customerEmail = buffer;
            SQLGetData(stmt, 7, SQL_C_LONG, &order.employeeId, sizeof(int), &ind);
            SQLGetData(stmt, 8, SQL_C_LONG, &order.statusId, sizeof(int), &ind);
            SQLGetData(stmt, 9, SQL_C_DOUBLE, &order.totalAmount, sizeof(double), &ind);
            SQLGetData(stmt, 10, SQL_C_LONG, &order.paymentStatus, sizeof(bool), &ind);
            SQLGetData(stmt, 11, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); order.notes = buffer;
        }
    }

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return order;
}

std::vector<Order> OrderRepository::getAll() {
    std::vector<Order> orders;
    std::wstring query = L"SELECT OrderID, OrderNumber, OrderDate, CustomerName, CustomerPhone, "
        L"CustomerEmail, EmployeeID, StatusID, TotalAmount, PaymentStatus, Notes "
        L"FROM Orders";

    SQLHSTMT stmt = m_db->prepareStatement(query);

    if (SQLExecute(stmt) == SQL_SUCCESS || SQLExecute(stmt) == SQL_SUCCESS_WITH_INFO) {
        while (SQLFetch(stmt) == SQL_SUCCESS) {
            Order order;
            SQLWCHAR buffer[256];
            SQLLEN ind;
            SQL_TIMESTAMP_STRUCT ts;

            SQLGetData(stmt, 1, SQL_C_LONG, &order.orderId, sizeof(int), &ind);
            SQLGetData(stmt, 2, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); order.orderNumber = buffer;
            SQLGetData(stmt, 3, SQL_C_TYPE_TIMESTAMP, &ts, sizeof(ts), &ind);
            SQLGetData(stmt, 4, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); order.customerName = buffer;
            SQLGetData(stmt, 5, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); order.customerPhone = buffer;
            SQLGetData(stmt, 6, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); order.customerEmail = buffer;
            SQLGetData(stmt, 7, SQL_C_LONG, &order.employeeId, sizeof(int), &ind);
            SQLGetData(stmt, 8, SQL_C_LONG, &order.statusId, sizeof(int), &ind);
            SQLGetData(stmt, 9, SQL_C_DOUBLE, &order.totalAmount, sizeof(double), &ind);
            SQLGetData(stmt, 10, SQL_C_LONG, &order.paymentStatus, sizeof(bool), &ind);
            SQLGetData(stmt, 11, SQL_C_WCHAR, buffer, sizeof(buffer), &ind); order.notes = buffer;

            orders.push_back(order);
        }
    }

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return orders;
}

bool OrderRepository::updateStatus(int orderId, int statusId) {
    std::wstring query = L"UPDATE Orders SET StatusID = ? WHERE OrderID = ?";
    SQLHSTMT stmt = m_db->prepareStatement(query);

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &statusId, 0, NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &orderId, 0, NULL);

    SQLRETURN ret = SQLExecute(stmt);

    SQLCloseCursor(stmt);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}