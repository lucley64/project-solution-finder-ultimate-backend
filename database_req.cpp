//
// Created by cytech on 20/03/24.
//

#include "database_req.h"

#include "map_param_trad.h"

database_req::database_req(const std::string& db_name) {
    sqlite3_open(db_name.c_str(), &db);
}

std::map<const std::string, const std::string> database_req::get_solution(const std::string& solution_id) const {
    sqlite3_stmt* stmt;

    std::stringstream sql_solution_details;
    sql_solution_details << "SELECT * FROM tblsolution WHERE numsolution = " << solution_id;
    const std::string str_sql_solution_details = sql_solution_details.str();

    std::map<const std::string, const std::string> ret;

    if (sqlite3_prepare_v2(db, str_sql_solution_details.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const int col_count = sqlite3_column_count(stmt);
            for (int i = 0; i < col_count; i++) {
                const std::string name = sqlite3_column_name(stmt, i);
                if (const auto ret_val = sqlite3_column_text(stmt, i); ret_val != nullptr) {
                    const std::string value = reinterpret_cast<const char *>(ret_val);
                    ret.emplace(name, value);
                }
            }
        }
    }


    return ret;
}

std::map<const std::string, const std::string> database_req::get_solution_desc(
    const std::string& solution_id, const std::string& code_lang) const {
    sqlite3_stmt* stmt;

    std::map<const std::string, const std::string> solution_map;
    std::stringstream sql_solution_descriptions;
    sql_solution_descriptions << "SELECT * " << "FROM tbldictionnaire " << "WHERE typedictionnaire = 'sol'" <<
            " AND codelangue = " <<
            code_lang << " AND codeappelobjet = " << solution_id << ";";

    const std::string str_sql = sql_solution_descriptions.str();

    if (sqlite3_prepare_v2(db, str_sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        int ret_code = 0;
        while ((ret_code = sqlite3_step(stmt)) == SQLITE_ROW) {
            const std::string index = map_param_trad::trad("tblsolution", sqlite3_column_int(stmt, 4));
            const std::string desc = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
            solution_map.emplace(index, desc);
        }
    }

    return solution_map;
}

std::vector<std::map<const std::string, const std::string>> database_req::get_gain_rex(
    const std::string& solution_id) const {
    sqlite3_stmt* stmt;

    std::stringstream sql_solution_details;
    sql_solution_details << "SELECT * FROM tblgainrex WHERE codesolution = " << solution_id;
    const std::string str_sql_solution_details = sql_solution_details.str();

    std::vector<std::map<const std::string, const std::string>> vector_ret;


    if (sqlite3_prepare_v2(db, str_sql_solution_details.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::map<const std::string, const std::string> ret;
            const int col_count = sqlite3_column_count(stmt);
            for (int i = 0; i < col_count; i++) {
                const std::string name = sqlite3_column_name(stmt, i);
                if (const auto ret_val = sqlite3_column_text(stmt, i); ret_val != nullptr) {
                    const std::string value = reinterpret_cast<const char *>(ret_val);
                    ret.emplace(name, value);
                }
            }
            vector_ret.push_back(ret);
        }
    }


    return vector_ret;
}

std::vector<std::map<const std::string, const std::string>> database_req::get_cout_rex(
    const std::string& solution_id) const {
    sqlite3_stmt* stmt;

    std::stringstream sql_solution_details;
    sql_solution_details << "SELECT * FROM tblcoutrex WHERE codesolution = " << solution_id;
    const std::string str_sql_solution_details = sql_solution_details.str();

    std::vector<std::map<const std::string, const std::string>> vector_ret;


    if (sqlite3_prepare_v2(db, str_sql_solution_details.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::map<const std::string, const std::string> ret;
            const int col_count = sqlite3_column_count(stmt);
            for (int i = 0; i < col_count; i++) {
                const std::string name = sqlite3_column_name(stmt, i);
                if (const auto ret_val = sqlite3_column_text(stmt, i); ret_val != nullptr) {
                    const std::string value = reinterpret_cast<const char *>(ret_val);
                    ret.emplace(name, value);
                }
            }
            vector_ret.push_back(ret);
        }
    }


    return vector_ret;
}


std::map<const std::string, const std::string> database_req::get_rex(const std::string& num_rex) const {
    sqlite3_stmt* stmt;

    std::stringstream sql_solution_details;
    sql_solution_details << "SELECT * FROM tblrex WHERE numrex = " << num_rex;
    const std::string str_sql_solution_details = sql_solution_details.str();

    std::map<const std::string, const std::string> ret;

    if (sqlite3_prepare_v2(db, str_sql_solution_details.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const int col_count = sqlite3_column_count(stmt);
            for (int i = 0; i < col_count; i++) {
                const std::string name = sqlite3_column_name(stmt, i);
                if (const auto ret_val = sqlite3_column_text(stmt, i); ret_val != nullptr) {
                    const std::string value = reinterpret_cast<const char *>(ret_val);
                    ret.emplace(name, value);
                }
            }
        }
    }


    return ret;
}

std::vector<std::map<const std::string, const std::string>> database_req::get_all_solutions() const {
    std::vector<std::map<const std::string, const std::string>> ret_value;

    sqlite3_stmt* stmt;

    std::stringstream sql_solution_details;
    sql_solution_details << "SELECT * FROM tblsolution";
    const std::string str_sql_solution_details = sql_solution_details.str();


    if (sqlite3_prepare_v2(db, str_sql_solution_details.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::map<const std::string, const std::string> ret;
            const int col_count = sqlite3_column_count(stmt);
            for (int i = 0; i < col_count; i++) {
                const std::string name = sqlite3_column_name(stmt, i);
                if (const auto ret_val = sqlite3_column_text(stmt, i); ret_val != nullptr) {
                    const std::string value = reinterpret_cast<const char *>(ret_val);
                    ret.emplace(name, value);
                }
            }
            ret_value.push_back(ret);
        }
    }

    return ret_value;
}

std::vector<std::map<const std::string, const std::string>> database_req::get_tbl_monaie() const {
    std::vector<std::map<const std::string, const std::string>> ret_value;

    sqlite3_stmt* stmt;

    std::stringstream sql_monaie;
    sql_monaie << "SELECT * FROM tblmonnaie";
    const std::string str_sql_monaie = sql_monaie.str();


    if (sqlite3_prepare_v2(db, str_sql_monaie.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::map<const std::string, const std::string> ret;
            const int col_count = sqlite3_column_count(stmt);
            for (int i = 0; i < col_count; i++) {
                const std::string name = sqlite3_column_name(stmt, i);
                if (const auto ret_val = sqlite3_column_text(stmt, i); ret_val != nullptr) {
                    const std::string value = reinterpret_cast<const char *>(ret_val);
                    ret.emplace(name, value);
                }
            }
            ret_value.push_back(ret);
        }
    }

    return ret_value;
}


database_req::~database_req() {
    sqlite3_close(db);
}
