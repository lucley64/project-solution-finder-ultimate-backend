//
// Created by cytech on 20/03/24.
//
#include "map_param_trad.h"

#include <filesystem>
#include <string>

bool map_param_trad::is_init = false;
web::json::value map_param_trad::map_param = web::json::value();

void map_param_trad::build() {

#ifdef  MY_DEBUG
    std::cout << std::filesystem::current_path().string() << std::endl;
#endif
    std::ifstream file(filename);

    std::string line;
    std::stringstream ss;
    while (std::getline(file, line)) {
        ss << line;
    }

    // std::cout << ss.str() << std::endl;

    map_param = web::json::value::parse(ss);


    is_init = true;
}

std::string map_param_trad::trad(const std::string& category, const int index) {
    if (!is_init) {
        build();
    }

    web::json::value v = map_param.at(category);

    const web::json::value r = v.at(index - 1);

    return r.as_string();
}
