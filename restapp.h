//
// Created by cytech on 19/03/24.
//

#ifndef RESTAPP_H
#define RESTAPP_H
#include <cpprest/http_listener.h>

#include "database_req.h"


class restapp final {
    static constexpr auto address = "http://localhost:8080/";

    bool is_running = false;

    web::http::experimental::listener::http_listener listener;

    std::string database_name;


    void handle_get(const web::http::http_request& request) const;

public:
    explicit restapp(std::string database_name);

    void start();

    void stop();

    void export_data() const;

    ~restapp();
};


#endif //RESTAPP_H
