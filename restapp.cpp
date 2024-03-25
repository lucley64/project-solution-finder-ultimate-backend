//
// Created by cytech on 19/03/24.
//

#include "restapp.h"

#include <charconv>
#include <iostream>
#include <utility>

#include "database_req.h"
#include "map_param_trad.h"

using web::http::http_request, web::http::status_codes, web::http::methods, web::http::http_response;
using web::http::experimental::listener::http_listener;
using web::json::value, web::json::array;
using std::vector, std::string, std::map, std::cout, std::endl, std::pair, std::from_chars, std::errc, std::cerr,
        std::stringstream;
using concurrency::streams::istream;

/**
 * Reads the body of a request and returns it as a string
 * @param stream The input stream.
 * @return The raw body.
 */
string read_body_to_end(const istream& stream) {
    auto stream_buffer = stream.streambuf();
    string ret;

    while (!stream_buffer.is_eof()) {
        ret += static_cast<char>(stream_buffer.nextc().get());
    }

    return ret;
}

/**
 * Parse a raw formdata and returns a map from this data.
 * @param input The raw form input.
 * @return A map of the data.
 */
map<string, string> parse_form_input(const string& input) {
    string s = input;
    const string delim = "\r\n-";
    const string name_delim = "name=\"";
    map<string, string> ret;
    size_t name_start;
    while ((name_start = s.find(name_delim)) != string::npos) {
        s.erase(0, name_start + name_delim.size());
        if (const size_t name_end = s.find('"'); name_end != string::npos) {
            const string name = s.substr(0, name_end);
            s.erase(0, name_end + 1);
            if (const size_t end = s.find(delim); end != string::npos) {
                const string content = s.substr(0, end);
                ret.emplace(name, content);
            }
        }
    }

    return ret;
}

/**
 * Gets the data from a form and awnser with a json of the data.
 * @param request The content of the request.
 */
void handle_post(const http_request& request) {
    // auto test = request.body().streambuf();

#ifdef MY_DEBUG
    cout << "Recieving post request" << endl;
#endif
    const auto fd = read_body_to_end(request.body());
    cout << fd << endl;
    auto a = parse_form_input(fd);
    const value json_rep = value::array(
        {
            value::object({{"nb_sol", 12}, {"financial_cost", .5}, {"financial_gain", .5}}),
            value::object({{"nb_sol", 8}, {"financial_cost", .5}, {"financial_gain", .5}}),
            value::object({{"nb_sol", 22}, {"financial_cost", .5}, {"financial_gain", .5}}),
            value::object({{"nb_sol", 42}, {"financial_cost", .5}, {"financial_gain", .5}}),
            value::object({{"nb_sol", 136}, {"financial_cost", .5}, {"financial_gain", .5}}),
        });

    http_response response{status_codes::OK};

    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.set_body(json_rep);

    auto req = request.reply(response);
}

vector<string> parse_uri(const string& uri) {
    vector<string> ret;
    string s = uri;
    constexpr char delim = '/';
    size_t pos;
    while ((pos = s.find(delim)) != string::npos) {
        if (const string token = s.substr(0, pos); !token.empty()) {
            ret.emplace_back(token);
        }
        s.erase(0, pos + 1);
    }
    if (!s.empty()) {
        ret.emplace_back(s);
    }
    return ret;
}

std::map<string, value> do_work(const string& code_lang, const database_req& req,
                                const std::map<const std::string, const std::string>& sol_details) {
    const string& sol_num = sol_details.at("numsolution");
#ifdef MY_DEBUG
    cout << "Collecting data for solution no " << sol_num << endl;
#endif
    std::map<string, value> solution_map;
    solution_map.emplace("solution_details", value::object(
                             vector<std::pair<string, value>>(sol_details.begin(),
                                                              sol_details.end())));
    std::map<string, value> solution_desc_map;
    for (auto ret_sol_map = req.get_solution_desc(sol_num, code_lang); const auto& [key, v]: ret_sol_map) {
        solution_desc_map.emplace(key, v);
    }
    if (sol_details.contains("codeparentsolution")) {
        const auto code_parent_sol = sol_details.at("codeparentsolution");
#ifdef MY_DEBUG
        cout << "Collecting data for parent solution no " << code_parent_sol << endl;
#endif

        for (auto ret_parent_sol_map = req.get_solution_desc(code_parent_sol, code_lang); const auto& [key, desc]:
             ret_parent_sol_map) {
            if (!solution_desc_map.contains(key) || solution_desc_map[key] == value("<P>&nbsp;</P>") ||
                solution_desc_map[key] ==
                value("&nbsp;")) {
                solution_desc_map.emplace(key, desc);
            }
        }
    }
    value v = value::object(
        vector<std::pair<string, value>>(solution_desc_map.begin(),
                                         solution_desc_map.end()));
    solution_map.emplace("solution_descriptions", v);

    const auto gains_rex = req.get_gain_rex(sol_num);
    std::vector<std::string> codes_rex;
    std::vector<value> values_gains_rex;
    for (auto gain_rex: gains_rex) {
        if (std::find(codes_rex.begin(), codes_rex.end(), gain_rex["coderex"]) == codes_rex.end()) {
            codes_rex.push_back(gain_rex["coderex"]);
        }
        std::vector<std::pair<std::string, value>> vector_gain_rex;

        vector_gain_rex.reserve(gain_rex.size());
        for (const auto& [key, v]: gain_rex) {
            vector_gain_rex.emplace_back(key, value(v));
        }

        values_gains_rex.push_back(value::object(vector_gain_rex));
    }
    solution_map.emplace("gain_rex", value::array(values_gains_rex));

    const auto couts_rex = req.get_cout_rex(sol_num);
    std::vector<value> values_couts_rex;
    for (auto cout_rex: couts_rex) {
        if (std::find(codes_rex.begin(), codes_rex.end(), cout_rex["coderex"]) == codes_rex.end()) {
            codes_rex.push_back(cout_rex["coderex"]);
        }
        std::vector<std::pair<std::string, value>> vector_cout_rex;

        vector_cout_rex.reserve(cout_rex.size());
        for (const auto& [key, v]: cout_rex) {
            vector_cout_rex.emplace_back(key, v);
        }

        values_couts_rex.push_back(value::object(vector_cout_rex));
    }
    solution_map.emplace("cout_rex", value::array(values_couts_rex));

    std::vector<value> rexes;
    for (const auto& code_rex: codes_rex) {
        auto rex = req.get_rex(code_rex);
        std::vector<std::pair<std::string, value>> vector_pairs_rex;
        vector_pairs_rex.reserve(rex.size());
        for (const auto& [k, v]: rex) {
            vector_pairs_rex.emplace_back(k, v);
        }
        rexes.push_back(value::object(vector_pairs_rex));
    }
    solution_map.emplace("rex", value::array(rexes));
    return solution_map;
}

void restapp::handle_get(const http_request& request) const {
#ifdef MY_DEBUG
    cout << "Reciving get request." << endl;
#endif
    const auto uri = request.relative_uri();
    const auto parsed_uri = parse_uri(uri.path());
    auto code = status_codes::OK;
    value reply_value = value::object();

    if (parsed_uri.size() >= 2) {
        const string& sol_num = parsed_uri[0];
        const string& code_lang = parsed_uri[1];

#ifdef MY_DEBUG
        cout << "Uri parsed with at least 2 arguments." << endl;
#endif

        stringstream sql_solution_details;
        sql_solution_details << "SELECT * FROM tblsolution WHERE numsolution = " << sol_num;
        string str_sql_solution_details = sql_solution_details.str();

        database_req req(database_name);
        auto sol_details = req.get_solution(sol_num);

        auto solution_map = do_work(code_lang, req, sol_details);

        reply_value = value::object(vector<std::pair<string, value>>(solution_map.begin(), solution_map.end()));
    }

    http_response response(code);
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.set_body(reply_value);
    auto _ = request.reply(response);
}

restapp::restapp(std::string database_name): database_name(std::move(database_name)) {
#ifdef MY_DEBUG
    cout << "Initializing app." << endl;
#endif
    listener = http_listener(address);
    listener.support(methods::GET, [this](const http_request& request) { handle_get(request); });
    listener.support(methods::POST, handle_post);
}

void restapp::start() {
#ifdef MY_DEBUG
    cout << "Starting app." << endl;
#endif
    std::cout << "Server listening on http://localhost:8080" << std::endl;
    listener.open();
}

void restapp::stop() {
#ifdef MY_DEBUG
    cout << "Stopping app." << endl;
#endif
    listener.close();
}

void restapp::export_data() const {
#ifdef MY_DEBUG
    cout << "Eporting data to ./solutions.json." << endl;
#endif

    const database_req req(database_name);

    std::vector<value> values;

    for (const auto solutions = req.get_all_solutions();
         const auto& solution: solutions) {
        if (solution.contains("validsolution") && solution.at("validsolution") == "1") {
            auto solution_map = do_work("2", req, solution);
            values.emplace_back(
                value::object(std::vector<std::pair<std::string, value>>(solution_map.begin(), solution_map.end())));
        }
    }

    value json_value = value::array(values);

#ifdef MY_DEBUG
    cout << "Writing to file." << endl;
#endif

    std::ofstream file("./solutions.json");

    file << json_value.serialize();

    file.close();
}

restapp::~restapp() = default;
