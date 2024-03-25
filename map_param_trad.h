//
// Created by cytech on 20/03/24.
//

#ifndef MAP_PARAM_TRAD_H
#define MAP_PARAM_TRAD_H
#include <string>
#include <cpprest/json.h>

class map_param_trad {
    static constexpr auto filename = "./map_params.json";
    static web::json::value map_param;
    static bool is_init;

    map_param_trad() = default;


    static void build();

public:
    static std::string trad(const std::string& category, int index);
};

#endif //MAP_PARAM_TRAD_H
