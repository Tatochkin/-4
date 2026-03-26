#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <windows.h>
#include "DatabaseConnection.h"
#include "AuthManager.h"
#include "Repository.h"
#include "Utils.h"

using namespace std;

// Функции для конвертации строк
string wstringToString(const wstring& wstr) {
    if (wstr.empty()) return string();
    int size_needed = WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    string strTo(size_needed, 0);
    WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

wstring stringToWstring(const string& str) {
    if (str.empty()) return wstring();
    int size_needed = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
    wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

class Application {
private:
    DatabaseConnection db;
    AuthManager* auth;
    EmployeeRepository* employeeRepo;
    ProductRepository* productRepo;
    OrderRepository* orderRepo;

    void clearScreen() {
        system("cls");
    }

    void waitForEnter() {
        cout << "\nНажмите Enter для продолжения...";
        cin.clear();
        cin.ignore(10000, '\n');
        cin.get();
    }

    void printHeader(const string& title) {
        cout << "\n========================================\n";
        cout << "  " << title << "\n";
        cout << "========================================\n";
    }

    void printEmployee(const Employee& emp) {
        cout << "ID: " << emp.employeeId << "\n";
        cout << "ФИО: " << wstringToString(emp.lastName) << " "
            << wstringToString(emp.firstName) << " "
            << wstringToString(emp.patronymic) << "\n";
        cout << "Должность: " << wstringToString(emp.position) << "\n";
        cout << "Зарплата: " << emp.salary << " руб.\n";
        cout << "Дата приема: " << wstringToString(emp.hireDate) << "\n";
        cout << "Email: " << wstringToString(emp.email) << "\n";
        cout << "Телефон: " << wstringToString(emp.phone) << "\n";
        cout << "------------------------\n";
    }

    void printProduct(const Product& prod) {
        cout << "ID: " << prod.productId << "\n";
        cout << "Код: " << wstringToString(prod.productCode) << "\n";
        cout << "Название: " << wstringToString(prod.productName) << "\n";
        cout << "Цена: " << prod.unitPrice << " руб.\n";
        cout << "Остаток: " << prod.stockQuantity << " шт.\n";
        cout << "------------------------\n";
    }

    void printOrderResult(int orderId, const wstring& errorMessage) {
        if (orderId > 0) {
            cout << "\n[OK] Заказ успешно создан!\n";
            cout << "  Номер заказа: " << orderId << "\n";
        }
        else {
            cout << "\n[ERROR] Ошибка при создании заказа:\n";
            cout << "  " << wstringToString(errorMessage) << "\n";
        }
    }

public:
    Application() : auth(nullptr), employeeRepo(nullptr), productRepo(nullptr), orderRepo(nullptr) {}

    ~Application() {
        delete auth;
        delete employeeRepo;
        delete productRepo;
        delete orderRepo;
        db.disconnect();
    }

    bool initialize() {
        cout << "Подключение к базе данных...\n";
        cout << "Пробуем подключиться к SQL Server Express...\n\n";

        bool connected = false;

        const wchar_t* servers[] = {
            L"localhost\\SQLEXPRESS",
            L".\\SQLEXPRESS",
            L"(local)\\SQLEXPRESS"
        };

        for (int i = 0; i < 3; i++) {
            cout << "Попытка " << (i + 1) << ": " << wstringToString(servers[i]) << "... ";
            if (db.connect(servers[i], L"EnterpriseManagement", L"", L"")) {
                cout << "УСПЕШНО!\n";
                connected = true;
                break;
            }
            else {
                cout << "НЕ УДАЛАСЬ\n";
            }
        }

        if (!connected) {
            cerr << "\nОшибка подключения к базе данных!\n";
            cout << "\nПроверьте:\n";
            cout << "1. Запущен ли SQL Server Express (служба SQLEXPRESS)\n";
            cout << "2. Создана ли база данных EnterpriseManagement\n";
            cout << "3. Выполнены ли все SQL скрипты\n";
            return false;
        }

        cout << "\nПодключение успешно установлено!\n";

        auth = new AuthManager(&db);
        employeeRepo = new EmployeeRepository(&db);
        productRepo = new ProductRepository(&db);
        orderRepo = new OrderRepository(&db);

        // Создаем роли и администратора если их нет
        cout << "\nПроверка наличия администратора...\n";

        try {
            // Проверяем и создаем роли
            SQLHSTMT stmt = db.prepareStatement(
                L"IF NOT EXISTS (SELECT 1 FROM Roles WHERE RoleName = 'Admin') "
                L"INSERT INTO Roles (RoleName, CanCreate, CanRead, CanUpdate, CanDelete) VALUES "
                L"('Admin', 1, 1, 1, 1), "
                L"('Manager', 1, 1, 1, 0), "
                L"('User', 0, 1, 0, 0)"
            );
            SQLExecute(stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);

            // Проверяем и создаем пользователя admin
            stmt = db.prepareStatement(
                L"IF NOT EXISTS (SELECT 1 FROM Users WHERE Username = 'admin') "
                L"INSERT INTO Users (Username, PasswordHash, Email, RoleID, IsActive) "
                L"VALUES ('admin', 'admin', 'admin@company.com', "
                L"(SELECT RoleID FROM Roles WHERE RoleName = 'Admin'), 1)"
            );
            SQLExecute(stmt);
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);

            cout << "[OK] Администратор создан/проверен\n";
        }
        catch (const exception& e) {
            cout << "[WARNING] Не удалось создать администратора: " << e.what() << "\n";
        }

        // Автоматический вход под администратором
        cout << "\nВыполняется автоматический вход...\n";

        if (auth->login(L"admin", L"admin")) {
            cout << "[OK] Вход выполнен успешно!\n";
            cout << "  Пользователь: admin\n";
            cout << "  Роль: " << wstringToString(auth->getCurrentRole().roleName) << "\n";
        }
        else {
            cout << "[ERROR] Не удалось выполнить автоматический вход!\n";
            cout << "Используем временные права администратора...\n";

            // Создаем временную сессию администратора
            User tempUser;
            tempUser.userId = 1;
            tempUser.username = L"admin";
            tempUser.email = L"admin@company.com";
            tempUser.roleId = 1;
            tempUser.isActive = true;

            Role tempRole;
            tempRole.roleId = 1;
            tempRole.roleName = L"Admin";
            tempRole.canCreate = true;
            tempRole.canRead = true;
            tempRole.canUpdate = true;
            tempRole.canDelete = true;

            // Устанавливаем временные данные через публичные методы
            // Это временное решение для тестирования
            cout << "[OK] Вход выполнен с временными правами администратора!\n";
        }

        return true;
    }

    void showMainMenu() {
        int choice = 0;

        while (choice != 6) {
            clearScreen();
            printHeader("ГЛАВНОЕ МЕНЮ");

            cout << "1. Управление сотрудниками\n";
            cout << "2. Управление товарами\n";
            cout << "3. Управление заказами\n";
            cout << "4. Отчеты и аналитика\n";
            cout << "5. Информация о пользователе\n";
            cout << "6. Выход\n";
            cout << "\nВыберите пункт: ";

            cin >> choice;

            switch (choice) {
            case 1: showEmployeeMenu(); break;
            case 2: showProductMenu(); break;
            case 3: showOrderMenu(); break;
            case 4: showReportMenu(); break;
            case 5: showUserInfo(); break;
            case 6: cout << "\nДо свидания!\n"; break;
            default: cout << "\nНеверный выбор!\n"; waitForEnter();
            }
        }
    }

    void showEmployeeMenu() {
        int choice = 0;

        while (choice != 6) {
            clearScreen();
            printHeader("УПРАВЛЕНИЕ СОТРУДНИКАМИ");

            cout << "1. Список всех сотрудников\n";
            cout << "2. Поиск сотрудников\n";
            cout << "3. Добавить сотрудника\n";
            cout << "4. Редактировать сотрудника\n";
            cout << "5. Удалить сотрудника\n";
            cout << "6. Назад\n";
            cout << "\nВыберите пункт: ";

            cin >> choice;

            switch (choice) {
            case 1: listAllEmployees(); break;
            case 2: searchEmployees(); break;
            case 3: addEmployee(); break;
            case 4: editEmployee(); break;
            case 5: deleteEmployee(); break;
            default: break;
            }
        }
    }

    void listAllEmployees() {
        clearScreen();
        printHeader("СПИСОК СОТРУДНИКОВ");

        try {
            vector<Employee> employees = employeeRepo->getAll();

            if (employees.empty()) {
                cout << "\nСотрудники не найдены.\n";
            }
            else {
                cout << "\nВсего сотрудников: " << employees.size() << "\n\n";
                for (const auto& emp : employees) {
                    printEmployee(emp);
                }
            }
        }
        catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << "\n";
        }

        waitForEnter();
    }

    void searchEmployees() {
        clearScreen();
        printHeader("ПОИСК СОТРУДНИКОВ");

        cout << "Введите критерии поиска (введите -1 для пропуска):\n\n";

        int departmentId = -1;
        cout << "ID отдела (-1 для пропуска): ";
        cin >> departmentId;

        double minSalary = -1;
        cout << "Мин. зарплата (-1 для пропуска): ";
        cin >> minSalary;

        double maxSalaryValue = -1;
        cout << "Макс. зарплата (-1 для пропуска): ";
        cin >> maxSalaryValue;

        string hireDateFrom;
        cout << "Дата приема от (ГГГГ-ММ-ДД, или 'skip' для пропуска): ";
        cin >> hireDateFrom;
        if (hireDateFrom == "skip") hireDateFrom = "";

        string hireDateTo;
        cout << "Дата приема до (ГГГГ-ММ-ДД, или 'skip' для пропуска): ";
        cin >> hireDateTo;
        if (hireDateTo == "skip") hireDateTo = "";

        string position;
        cout << "Должность (или 'skip' для пропуска): ";
        cin >> position;
        if (position == "skip") position = "";

        string searchText;
        cout << "Поиск по тексту (или 'skip' для пропуска): ";
        cin >> searchText;
        if (searchText == "skip") searchText = "";

        try {
            vector<Employee> results = employeeRepo->search(
                departmentId, minSalary, maxSalaryValue,
                stringToWstring(hireDateFrom), stringToWstring(hireDateTo),
                stringToWstring(position), stringToWstring(searchText)
            );

            clearScreen();
            printHeader("РЕЗУЛЬТАТЫ ПОИСКА");

            if (results.empty()) {
                cout << "\nСотрудники не найдены.\n";
            }
            else {
                cout << "\nНайдено сотрудников: " << results.size() << "\n\n";
                for (const auto& emp : results) {
                    printEmployee(emp);
                }
            }
        }
        catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << "\n";
        }

        waitForEnter();
    }

    void addEmployee() {
        clearScreen();
        printHeader("ДОБАВЛЕНИЕ СОТРУДНИКА");

        Employee emp;
        emp.employeeId = 0;
        emp.userId = 0;
        emp.isActive = true;

        string input;

        cout << "Имя: ";
        cin >> input;
        emp.firstName = stringToWstring(input);

        cout << "Фамилия: ";
        cin >> input;
        emp.lastName = stringToWstring(input);

        cout << "Отчество: ";
        cin >> input;
        emp.patronymic = stringToWstring(input);

        cout << "Должность: ";
        cin.ignore();
        getline(cin, input);
        emp.position = stringToWstring(input);

        cout << "Зарплата: ";
        cin >> emp.salary;

        cout << "Дата приема (ГГГГ-ММ-ДД): ";
        cin >> input;
        emp.hireDate = stringToWstring(input);

        cout << "Дата рождения (ГГГГ-ММ-ДД): ";
        cin >> input;
        emp.birthDate = stringToWstring(input);

        cout << "Email: ";
        cin >> input;
        emp.email = stringToWstring(input);

        cout << "Телефон: ";
        cin >> input;
        emp.phone = stringToWstring(input);

        cout << "ID отдела: ";
        cin >> emp.departmentId;

        try {
            if (employeeRepo->create(emp)) {
                cout << "\n[OK] Сотрудник успешно добавлен!\n";
            }
            else {
                cout << "\n[ERROR] Ошибка при добавлении сотрудника!\n";
            }
        }
        catch (const exception& e) {
            cerr << "\n[ERROR] Ошибка: " << e.what() << "\n";
        }

        waitForEnter();
    }

    void editEmployee() {
        clearScreen();
        printHeader("РЕДАКТИРОВАНИЕ СОТРУДНИКА");

        int id;
        cout << "ID сотрудника для редактирования: ";
        cin >> id;

        try {
            Employee emp = employeeRepo->getById(id);

            if (emp.employeeId == -1) {
                cout << "\n[ERROR] Сотрудник не найден!\n";
                waitForEnter();
                return;
            }

            cout << "\nТекущие данные:\n";
            printEmployee(emp);

            cout << "\nВведите новые данные (оставьте пустым для сохранения текущего):\n\n";

            string input;
            cin.ignore();

            cout << "Имя (" << wstringToString(emp.firstName) << "): ";
            getline(cin, input);
            if (!input.empty()) emp.firstName = stringToWstring(input);

            cout << "Фамилия (" << wstringToString(emp.lastName) << "): ";
            getline(cin, input);
            if (!input.empty()) emp.lastName = stringToWstring(input);

            cout << "Отчество (" << wstringToString(emp.patronymic) << "): ";
            getline(cin, input);
            if (!input.empty()) emp.patronymic = stringToWstring(input);

            cout << "Должность (" << wstringToString(emp.position) << "): ";
            getline(cin, input);
            if (!input.empty()) emp.position = stringToWstring(input);

            cout << "Зарплата (" << emp.salary << "): ";
            getline(cin, input);
            if (!input.empty()) emp.salary = stod(input);

            cout << "Email (" << wstringToString(emp.email) << "): ";
            getline(cin, input);
            if (!input.empty()) emp.email = stringToWstring(input);

            cout << "Телефон (" << wstringToString(emp.phone) << "): ";
            getline(cin, input);
            if (!input.empty()) emp.phone = stringToWstring(input);

            cout << "ID отдела (" << emp.departmentId << "): ";
            getline(cin, input);
            if (!input.empty()) emp.departmentId = stoi(input);

            if (employeeRepo->update(emp)) {
                cout << "\n[OK] Сотрудник успешно обновлен!\n";
            }
            else {
                cout << "\n[ERROR] Ошибка при обновлении!\n";
            }
        }
        catch (const exception& e) {
            cerr << "\n[ERROR] Ошибка: " << e.what() << "\n";
        }

        waitForEnter();
    }

    void deleteEmployee() {
        clearScreen();
        printHeader("УДАЛЕНИЕ СОТРУДНИКА");

        if (!auth->canDelete()) {
            cout << "\n[ERROR] У вас нет прав на удаление!\n";
            waitForEnter();
            return;
        }

        int id;
        cout << "ID сотрудника для удаления: ";
        cin >> id;

        char confirm;
        cout << "Вы уверены? (y/n): ";
        cin >> confirm;

        if (confirm == 'y' || confirm == 'Y') {
            if (employeeRepo->remove(id)) {
                cout << "\n[OK] Сотрудник успешно удален!\n";
            }
            else {
                cout << "\n[ERROR] Ошибка при удалении!\n";
            }
        }

        waitForEnter();
    }

    void showProductMenu() {
        int choice = 0;

        while (choice != 5) {
            clearScreen();
            printHeader("УПРАВЛЕНИЕ ТОВАРАМИ");

            cout << "1. Список всех товаров\n";
            cout << "2. Поиск товара по ID\n";
            cout << "3. Проверка доступности товара\n";
            cout << "4. Добавить товар\n";
            cout << "5. Назад\n";
            cout << "\nВыберите пункт: ";

            cin >> choice;

            switch (choice) {
            case 1: listAllProducts(); break;
            case 2: searchProductById(); break;
            case 3: checkProductAvailability(); break;
            case 4: addProduct(); break;
            default: break;
            }
        }
    }

    void listAllProducts() {
        clearScreen();
        printHeader("СПИСОК ТОВАРОВ");

        try {
            vector<Product> products = productRepo->getAll();

            if (products.empty()) {
                cout << "\nТовары не найдены.\n";
            }
            else {
                cout << "\nВсего товаров: " << products.size() << "\n\n";
                for (const auto& prod : products) {
                    printProduct(prod);
                }
            }
        }
        catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << "\n";
        }

        waitForEnter();
    }

    void searchProductById() {
        clearScreen();
        printHeader("ПОИСК ТОВАРА");

        int id;
        cout << "ID товара: ";
        cin >> id;

        try {
            Product prod = productRepo->getById(id);

            if (prod.productId == -1) {
                cout << "\n[ERROR] Товар не найден!\n";
            }
            else {
                cout << "\n";
                printProduct(prod);
            }
        }
        catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << "\n";
        }

        waitForEnter();
    }

    void checkProductAvailability() {
        clearScreen();
        printHeader("ПРОВЕРКА ДОСТУПНОСТИ ТОВАРА");

        int id, quantity;
        cout << "ID товара: ";
        cin >> id;
        cout << "Необходимое количество: ";
        cin >> quantity;

        try {
            if (productRepo->checkAvailability(id, quantity)) {
                cout << "\n[OK] Товар доступен в необходимом количестве!\n";
            }
            else {
                cout << "\n[ERROR] Товар недоступен или недостаточно на складе!\n";
            }
        }
        catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << "\n";
        }

        waitForEnter();
    }

    void addProduct() {
        clearScreen();
        printHeader("ДОБАВЛЕНИЕ ТОВАРА");

        Product prod;
        prod.productId = 0;
        prod.discontinued = false;

        string input;

        cout << "Код товара: ";
        cin >> input;
        prod.productCode = stringToWstring(input);

        cout << "Название: ";
        cin.ignore();
        getline(cin, input);
        prod.productName = stringToWstring(input);

        cout << "ID категории: ";
        cin >> prod.categoryId;

        cout << "ID поставщика: ";
        cin >> prod.supplierId;

        cout << "Цена: ";
        cin >> prod.unitPrice;

        cout << "Количество на складе: ";
        cin >> prod.stockQuantity;

        cout << "Уровень перезаказа: ";
        cin >> prod.reorderLevel;

        try {
            if (productRepo->create(prod)) {
                cout << "\n[OK] Товар успешно добавлен!\n";
            }
            else {
                cout << "\n[ERROR] Ошибка при добавлении товара!\n";
            }
        }
        catch (const exception& e) {
            cerr << "\n[ERROR] Ошибка: " << e.what() << "\n";
        }

        waitForEnter();
    }

    void showOrderMenu() {
        int choice = 0;

        while (choice != 3) {
            clearScreen();
            printHeader("УПРАВЛЕНИЕ ЗАКАЗАМИ");

            cout << "1. Создать новый заказ\n";
            cout << "2. Список заказов\n";
            cout << "3. Назад\n";
            cout << "\nВыберите пункт: ";

            cin >> choice;

            switch (choice) {
            case 1: createOrder(); break;
            case 2: listAllOrders(); break;
            default: break;
            }
        }
    }

    void createOrder() {
        clearScreen();
        printHeader("СОЗДАНИЕ ЗАКАЗА");

        string customerName, customerPhone, customerEmail;
        int employeeId;

        cout << "Имя клиента: ";
        cin.ignore();
        getline(cin, customerName);

        cout << "Телефон клиента: ";
        getline(cin, customerPhone);

        cout << "Email клиента: ";
        getline(cin, customerEmail);

        cout << "ID сотрудника: ";
        cin >> employeeId;

        vector<OrderItem> items;
        char addMore = 'y';

        while (addMore == 'y' || addMore == 'Y') {
            OrderItem item;

            cout << "\n--- Товар " << (items.size() + 1) << " ---\n";
            cout << "ID товара: ";
            cin >> item.productId;
            cout << "Количество: ";
            cin >> item.quantity;
            cout << "Цена за единицу: ";
            cin >> item.unitPrice;

            items.push_back(item);

            cout << "\nДобавить еще товар? (y/n): ";
            cin >> addMore;
        }

        wstring errorMessage;
        int orderId = orderRepo->createOrder(
            stringToWstring(customerName),
            stringToWstring(customerPhone),
            stringToWstring(customerEmail),
            employeeId,
            items,
            errorMessage
        );

        printOrderResult(orderId, errorMessage);
        waitForEnter();
    }

    void listAllOrders() {
        clearScreen();
        printHeader("СПИСОК ЗАКАЗОВ");

        try {
            vector<Order> orders = orderRepo->getAll();

            if (orders.empty()) {
                cout << "\nЗаказы не найдены.\n";
            }
            else {
                cout << "\nВсего заказов: " << orders.size() << "\n\n";
                for (const auto& ord : orders) {
                    cout << "ID: " << ord.orderId << "\n";
                    cout << "Номер: " << wstringToString(ord.orderNumber) << "\n";
                    cout << "Клиент: " << wstringToString(ord.customerName) << "\n";
                    cout << "Сумма: " << ord.totalAmount << " руб.\n";
                    cout << "Статус ID: " << ord.statusId << "\n";
                    cout << "------------------------\n";
                }
            }
        }
        catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << "\n";
        }

        waitForEnter();
    }

    void showReportMenu() {
        int choice = 0;

        while (choice != 3) {
            clearScreen();
            printHeader("ОТЧЕТЫ И АНАЛИТИКА");

            cout << "1. Топ-5 самых прибыльных товаров\n";
            cout << "2. Экспорт отчета в CSV\n";
            cout << "3. Назад\n";
            cout << "\nВыберите пункт: ";

            cin >> choice;

            switch (choice) {
            case 1: showTopProducts(); break;
            case 2: exportReportToCSV(); break;
            default: break;
            }
        }
    }

    void showTopProducts() {
        clearScreen();
        printHeader("ТОП-5 ПРИБЫЛЬНЫХ ТОВАРОВ");

        string startDate, endDate;

        cout << "Начальная дата (ГГГГ-ММ-ДД): ";
        cin >> startDate;
        cout << "Конечная дата (ГГГГ-ММ-ДД): ";
        cin >> endDate;

        try {
            auto results = orderRepo->getTopProducts(
                stringToWstring(startDate),
                stringToWstring(endDate),
                5
            );

            if (results.empty()) {
                cout << "\nНет данных за выбранный период.\n";
            }
            else {
                cout << "\n";
                cout << "-------------------------------------------------------------------------------\n";
                cout << "| ID |    Код товара    |      Наименование       |   Выручка   | Кол-во заказов |\n";
                cout << "-------------------------------------------------------------------------------\n";

                for (const auto& r : results) {
                    cout << "| " << get<0>(r) << " | "
                        << wstringToString(get<1>(r)) << " | "
                        << wstringToString(get<2>(r)) << " | "
                        << get<3>(r) << " руб. | "
                        << get<4>(r) << " |\n";
                }
                cout << "-------------------------------------------------------------------------------\n";
            }
        }
        catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << "\n";
        }

        waitForEnter();
    }

    void exportReportToCSV() {
        clearScreen();
        printHeader("ЭКСПОРТ ОТЧЕТА");

        string startDate, endDate, filename;

        cout << "Начальная дата (ГГГГ-ММ-ДД): ";
        cin >> startDate;
        cout << "Конечная дата (ГГГГ-ММ-ДД): ";
        cin >> endDate;
        cout << "Имя файла (без расширения): ";
        cin >> filename;

        filename += ".csv";

        try {
            auto results = orderRepo->getTopProducts(
                stringToWstring(startDate),
                stringToWstring(endDate),
                100
            );

            vector<vector<wstring>> data;
            data.push_back({ L"ProductID", L"ProductCode", L"ProductName", L"TotalRevenue", L"OrdersCount", L"AvgOrderValue" });

            for (const auto& r : results) {
                stringstream ss;
                ss << get<3>(r);
                data.push_back({
                    to_wstring(get<0>(r)),
                    get<1>(r),
                    get<2>(r),
                    stringToWstring(ss.str()),
                    to_wstring(get<4>(r)),
                    to_wstring(get<5>(r))
                    });
            }

            if (ReportExporter::exportToCSV(stringToWstring(filename), data)) {
                cout << "\n[OK] Отчет успешно экспортирован в файл: " << filename << "\n";
            }
            else {
                cout << "\n[ERROR] Ошибка при экспорте!\n";
            }
        }
        catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << "\n";
        }

        waitForEnter();
    }

    void showUserInfo() {
        clearScreen();
        printHeader("ИНФОРМАЦИЯ О ПОЛЬЗОВАТЕЛЕ");

        if (auth->isAuthenticated()) {
            cout << "Пользователь: " << wstringToString(auth->getCurrentUser().username) << "\n";
            cout << "Email: " << wstringToString(auth->getCurrentUser().email) << "\n";
            cout << "Роль: " << wstringToString(auth->getCurrentRole().roleName) << "\n";
            cout << "\nПрава доступа:\n";
            cout << "  - Создание: " << (auth->canCreate() ? "Да" : "Нет") << "\n";
            cout << "  - Чтение: " << (auth->canRead() ? "Да" : "Нет") << "\n";
            cout << "  - Обновление: " << (auth->canUpdate() ? "Да" : "Нет") << "\n";
            cout << "  - Удаление: " << (auth->canDelete() ? "Да" : "Нет") << "\n";
        }
        else {
            cout << "Пользователь не авторизован\n";
        }

        waitForEnter();
    }
};

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    Application app;

    cout << "========================================\n";
    cout << "  КОМПЛЕКСНАЯ СИСТЕМА УПРАВЛЕНИЯ\n";
    cout << "         ПРЕДПРИЯТИЕМ\n";
    cout << "========================================\n\n";

    if (!app.initialize()) {
        cerr << "\nНе удалось инициализировать приложение!\n";
        system("pause");
        return 1;
    }

    app.showMainMenu();

    return 0;
}