//
// Created by cytech on 20/03/24.
//

#ifndef DATABASE_REQ_H
#define DATABASE_REQ_H
#include <map>
#include <sqlite3.h>
#include <string>
#include <vector>


class database_req final {
    sqlite3* db{};

public:
    explicit database_req(const std::string& db_name);


    [[nodiscard]] std::map<const std::string, const std::string> get_solution(const std::string& solution_id) const;

    [[nodiscard]] std::map<const std::string, const std::string> get_solution_desc(
        const std::string& solution_id, const std::string& code_lang) const;

    [[nodiscard]] std::vector<std::map<const std::string, const std::string>> get_gain_rex(
        const std::string& solution_id) const;

    [[nodiscard]] std::vector<std::map<const std::string, const std::string>> get_cout_rex(
        const std::string& solution_id) const;

    [[nodiscard]] std::map<const std::string, const std::string> get_rex(const std::string& num_rex) const;

    [[nodiscard]] std::vector<std::map<const std::string, const std::string>> get_all_solutions() const;


    ~database_req();
};


#endif //DATABASE_REQ_H
