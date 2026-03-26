#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <regex>
#include <vector>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <fstream>

class Validator {
public:
    static bool validateEmail(const std::wstring& email) {
        std::wregex pattern(LR"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        return std::regex_match(email, pattern);
    }

    static bool validatePhone(const std::wstring& phone) {
        std::wregex pattern(LR"((\+?\d{1,3}[\s\-]?)?\(?\d{3}\)?[\s\-]?\d{3}[\s\-]?\d{2}[\s\-]?\d{2})");
        return std::regex_match(phone, pattern);
    }

    static bool validateDate(const std::wstring& date) {
        std::wregex pattern(LR"(\d{4}-\d{2}-\d{2})");
        return std::regex_match(date, pattern);
    }

    static bool validatePositiveNumber(double value) {
        return value > 0;
    }

    static bool validateSalary(double salary) {
        return salary > 0 && salary <= 1000000;
    }

    static bool validateName(const std::wstring& name) {
        std::wregex pattern(LR"([A-Za-zА-Яа-яЁё\s\-]{2,50})");
        return std::regex_match(name, pattern);
    }

    static bool validateProductCode(const std::wstring& code) {
        std::wregex pattern(LR"([A-Z0-9\-]{3,20})");
        return std::regex_match(code, pattern);
    }

    static bool validateTaxNumber(const std::wstring& taxNumber) {
        std::wregex pattern(LR"(\d{10,12})");
        return std::regex_match(taxNumber, pattern);
    }

    static bool validateIntRange(int value, int min, int max) {
        return value >= min && value <= max;
    }

    static bool validateNotEmpty(const std::wstring& str) {
        return !str.empty();
    }
};

class DateHelper {
public:
    static std::wstring getCurrentDate() {
        time_t now = time(nullptr);
        struct tm tm_info;
        localtime_s(&tm_info, &now);
        wchar_t buffer[11];
        wcsftime(buffer, 11, L"%Y-%m-%d", &tm_info);
        return std::wstring(buffer);
    }

    static std::wstring getCurrentDateTime() {
        time_t now = time(nullptr);
        struct tm tm_info;
        localtime_s(&tm_info, &now);
        wchar_t buffer[20];
        wcsftime(buffer, 20, L"%Y-%m-%d %H:%M:%S", &tm_info);
        return std::wstring(buffer);
    }

    static std::wstring formatDateTime(time_t time) {
        struct tm tm_info;
        localtime_s(&tm_info, &time);
        wchar_t buffer[20];
        wcsftime(buffer, 20, L"%Y-%m-%d %H:%M:%S", &tm_info);
        return std::wstring(buffer);
    }

    static std::wstring formatDate(time_t time) {
        struct tm tm_info;
        localtime_s(&tm_info, &time);
        wchar_t buffer[11];
        wcsftime(buffer, 11, L"%Y-%m-%d", &tm_info);
        return std::wstring(buffer);
    }

    static time_t parseDate(const std::wstring& date) {
        struct tm tm = { 0 };
        int year, month, day;
        swscanf_s(date.c_str(), L"%d-%d-%d", &year, &month, &day);
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        return mktime(&tm);
    }

    static bool isDateInRange(const std::wstring& date, const std::wstring& startDate, const std::wstring& endDate) {
        time_t t = parseDate(date);
        time_t start = parseDate(startDate);
        time_t end = parseDate(endDate);
        return t >= start && t <= end;
    }
};

class ReportExporter {
public:
    static bool exportToCSV(const std::wstring& filename, const std::vector<std::vector<std::wstring>>& data) {
        std::wofstream file(filename);
        if (!file.is_open()) return false;

        for (const auto& row : data) {
            for (size_t i = 0; i < row.size(); ++i) {
                std::wstring escaped = row[i];
                // Экранирование кавычек
                size_t pos = 0;
                while ((pos = escaped.find(L'"', pos)) != std::wstring::npos) {
                    escaped.replace(pos, 1, L"\"\"");
                    pos += 2;
                }
                file << L"\"" << escaped << L"\"";
                if (i < row.size() - 1) file << L",";
            }
            file << std::endl;
        }

        file.close();
        return true;
    }

    static bool exportToTXT(const std::wstring& filename, const std::wstring& content) {
        std::wofstream file(filename);
        if (!file.is_open()) return false;

        file << content;
        file.close();
        return true;
    }

    static bool exportToTXT(const std::wstring& filename, const std::vector<std::wstring>& lines) {
        std::wofstream file(filename);
        if (!file.is_open()) return false;

        for (const auto& line : lines) {
            file << line << std::endl;
        }

        file.close();
        return true;
    }
};

class StringHelper {
public:
    static std::wstring trim(const std::wstring& str) {
        size_t start = str.find_first_not_of(L" \t\n\r");
        if (start == std::wstring::npos) return L"";
        size_t end = str.find_last_not_of(L" \t\n\r");
        return str.substr(start, end - start + 1);
    }

    static std::wstring toUpper(const std::wstring& str) {
        std::wstring result = str;
        for (auto& c : result) {
            c = towupper(c);
        }
        return result;
    }

    static std::wstring toLower(const std::wstring& str) {
        std::wstring result = str;
        for (auto& c : result) {
            c = towlower(c);
        }
        return result;
    }

    static std::vector<std::wstring> split(const std::wstring& str, wchar_t delimiter) {
        std::vector<std::wstring> tokens;
        std::wstringstream ss(str);
        std::wstring token;
        while (std::getline(ss, token, delimiter)) {
            if (!token.empty()) {
                tokens.push_back(token);
            }
        }
        return tokens;
    }

    static std::wstring join(const std::vector<std::wstring>& tokens, const std::wstring& delimiter) {
        std::wstring result;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (i > 0) result += delimiter;
            result += tokens[i];
        }
        return result;
    }

    static bool startsWith(const std::wstring& str, const std::wstring& prefix) {
        if (str.length() < prefix.length()) return false;
        return str.compare(0, prefix.length(), prefix) == 0;
    }

    static bool endsWith(const std::wstring& str, const std::wstring& suffix) {
        if (str.length() < suffix.length()) return false;
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    }
};

#endif