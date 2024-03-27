//
// Created by cytech on 19/03/24.
//

#include "restapp.h"

#include <charconv>
#include <iostream>
#include <set>
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

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
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

value get_model_prediction(const string& input) {
    // model/balance_sheet.py
    const auto command = popen(string(".venv/bin/python test.py \"" + input + "\"").c_str(), "r");


    stringstream ss;
    char c = static_cast<char>(fgetc(command));
    while (c != EOF) {
        ss << c;
        c = static_cast<char>(fgetc(command));
    }

    pclose(command);

    const string s = ss.str();

#ifdef MY_DEBUG
    cout << "Recieving json from model: " << s << endl;
#endif


    return value::parse(s);
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
    auto _ = get_model_prediction(a["search"]);
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
    vector<pair<string, value>> solution_desc_map;
    std::map<string, value> solution_desc_map_fr;
    std::map<string, value> solution_desc_map_en;
    std::map<string, value> solution_desc_map_es;
    for (auto ret_sol_map = req.get_solution_desc(sol_num, "2"); const auto& [key, v]: ret_sol_map) {
        solution_desc_map_fr.emplace(key, v);
    }
    for (auto ret_sol_map = req.get_solution_desc(sol_num, "3"); const auto& [key, v]: ret_sol_map) {
        solution_desc_map_en.emplace(key, v);
    }
    for (auto ret_sol_map = req.get_solution_desc(sol_num, "4"); const auto& [key, v]: ret_sol_map) {
        solution_desc_map_es.emplace(key, v);
    }
    if (sol_details.contains("codeparentsolution")) {
        const auto code_parent_sol = sol_details.at("codeparentsolution");
#ifdef MY_DEBUG
        cout << "Collecting data for parent solution no " << code_parent_sol << endl;
#endif

        for (auto ret_parent_sol_map = req.get_solution_desc(code_parent_sol, "2"); const auto& [key, desc]:
             ret_parent_sol_map) {
            if (!solution_desc_map_fr.contains(key) || solution_desc_map_fr[key] == value("<P>&nbsp;</P>") ||
                solution_desc_map_fr[key] ==
                value("&nbsp;")) {
                solution_desc_map_fr.emplace(key, desc);
            }
        }
        for (auto ret_parent_sol_map = req.get_solution_desc(code_parent_sol, "3"); const auto& [key, desc]:
             ret_parent_sol_map) {
            if (!solution_desc_map_en.contains(key) || solution_desc_map_en[key] == value("<P>&nbsp;</P>") ||
                solution_desc_map_en[key] ==
                value("&nbsp;")) {
                solution_desc_map_en.emplace(key, desc);
            }
        }
        for (auto ret_parent_sol_map = req.get_solution_desc(code_parent_sol, "4"); const auto& [key, desc]:
             ret_parent_sol_map) {
            if (!solution_desc_map_es.contains(key) || solution_desc_map_es[key] == value("<P>&nbsp;</P>") ||
                solution_desc_map_es[key] ==
                value("&nbsp;")) {
                solution_desc_map_es.emplace(key, desc);
            }
        }
    }
    solution_desc_map.emplace_back(
        "fr", value::object(
            vector<std::pair<string, value>>(solution_desc_map_fr.begin(),
                                             solution_desc_map_fr.end()))
    );
    solution_desc_map.emplace_back(
        "en", value::object(
            vector<std::pair<string, value>>(solution_desc_map_en.begin(),
                                             solution_desc_map_en.end()))
    );
    solution_desc_map.emplace_back(
        "es", value::object(
            vector<std::pair<string, value>>(solution_desc_map_es.begin(),
                                             solution_desc_map_es.end()))
    );


    value v = value::object(solution_desc_map);
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

    stringstream tblrexstream, tblgainrexstream, tblcoutrexstream, textsolmodelstream;
    tblrexstream <<
            R"("numrex","codereference","codepublic","codemonnaie","codetauxmonnaie","gainfinancierrex","gainfinancierperioderex","energierex","codeuniteenergie","codeperiodeenergie","codeenergierex","gesrex","ratiogainrex","trirex","capexrex","capexperioderex","opexrex","codeTechno1","codeTechno2","codeTechno3","codetravaux","codereseau","codelicense","availablelangue")"
            << endl;
    tblgainrexstream <<
            R"("numgainrex","codesolution","coderex","gainfinanciergainrex","codemonnaiegainrex","codeperiodeeconomie","energiegainrex","uniteenergiegainrex","codeperiodeenergie","gesgainrex","minigainrex","maxigainrex","moyengainrex","reelgainrex","trireelgainrex","trimingainrex","trimaxgainrex","codelicense")"
            << endl;
    tblcoutrexstream <<
            R"("numcoutrex","codesolution","coderex","minicoutrex","maxicoutrex","reelcoutrex","codemonnaiecoutrex","codeunitecoutrex","codedifficulte","codelicense")"
            << endl;
    textsolmodelstream << R"("codelangue","indexdictionnaire","codeappelobjet","traductiondictionnaire")" << endl;
    std::set<string> tblrexids, tblgainrexids, tblcoutrexids;

    for (const auto solutions = req.get_all_solutions();
         const auto& solution: solutions) {
        if (solution.contains("validsolution") && solution.at("validsolution") == "1") {
            auto solution_map = do_work("2", req, solution);
            values.emplace_back(
                value::object(std::vector<std::pair<std::string, value>>(solution_map.begin(), solution_map.end())));


            for (const auto& [key, v]: solution_map) {
                if (key == "cout_rex") {
                    for (const auto cout_rexs = v.as_array(); const auto& cout_rex: cout_rexs) {
                        if (cout_rex.has_string_field("numcoutrex") && !tblcoutrexids.contains(
                                cout_rex.at("numcoutrex").as_string())) {
                            tblcoutrexids.emplace(cout_rex.at("numcoutrex").as_string());
                            tblcoutrexstream <<
                                    "\"" << (cout_rex.has_string_field("numcoutrex")
                                         ? cout_rex.at("numcoutrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (cout_rex.has_string_field("codesolution")
                                         ? cout_rex.at("codesolution").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (cout_rex.has_string_field("coderex") ? cout_rex.at("coderex").as_string() : "NULL") << "\""
                                    << "," <<
                                    "\"" << (cout_rex.has_string_field("minicoutrex")
                                         ? cout_rex.at("minicoutrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (cout_rex.has_string_field("maxicoutrex")
                                         ? cout_rex.at("maxicoutrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (cout_rex.has_string_field("reelcoutrex")
                                         ? cout_rex.at("reelcoutrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (cout_rex.has_string_field("codemonnaiecoutrex")
                                         ? cout_rex.at("codemonnaiecoutrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (cout_rex.has_string_field("codeunitecoutrex")
                                         ? cout_rex.at("codeunitecoutrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (cout_rex.has_string_field("codedifficulte")
                                         ? cout_rex.at("codedifficulte").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (cout_rex.has_string_field("codelicense")
                                         ? cout_rex.at("codelicense").as_string()
                                         : "NULL") << "\"" << endl;
                        }
                    }
                } else if (key == "gain_rex") {
                    for (const auto gain_rexs = v.as_array(); const auto& gain_rex: gain_rexs) {
                        if (gain_rex.has_string_field("numgainrex") && !tblgainrexids.contains(
                                gain_rex.at("numgainrex").as_string())) {
                            tblgainrexids.emplace(gain_rex.at("numgainrex").as_string());
                            tblgainrexstream <<
                                    "\"" << (gain_rex.has_string_field("numgainrex")
                                         ? gain_rex.at("numgainrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("codesolution")
                                         ? gain_rex.at("codesolution").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("coderex") ? gain_rex.at("coderex").as_string() : "NULL") << "\""
                                    << "," <<
                                    "\"" << (gain_rex.has_string_field("gainfinanciergainrex")
                                         ? gain_rex.at("gainfinanciergainrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("codemonnaiegainrex")
                                         ? gain_rex.at("codemonnaiegainrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("codeperiodeeconomie")
                                         ? gain_rex.at("codeperiodeeconomie").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("energiegainrex")
                                         ? gain_rex.at("energiegainrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("uniteenergiegainrex")
                                         ? gain_rex.at("uniteenergiegainrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("codeperiodeenergie")
                                         ? gain_rex.at("codeperiodeenergie").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("gesgainrex")
                                         ? gain_rex.at("gesgainrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("minigainrex")
                                         ? gain_rex.at("minigainrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("maxigainrex")
                                         ? gain_rex.at("maxigainrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("moyengainrex")
                                         ? gain_rex.at("moyengainrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("reelgainrex")
                                         ? gain_rex.at("reelgainrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("trireelgainrex")
                                         ? gain_rex.at("trireelgainrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("trimingainrex")
                                         ? gain_rex.at("trimingainrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("trimaxgainrex")
                                         ? gain_rex.at("trimaxgainrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (gain_rex.has_string_field("codelicense")
                                         ? gain_rex.at("codelicense").as_string()
                                         : "NULL") << "\"" << endl;
                        }
                    }
                } else if (key == "rex") {
                    for (const auto rexs = v.as_array(); const auto& rex: rexs) {
                        if (rex.has_string_field("numrex") && !tblrexids.contains(rex.at("numrex").as_string())) {
                            tblrexids.emplace(rex.at("numrex").as_string());
                            tblrexstream <<
                                    "\"" << (rex.has_string_field("numrex") ? rex.at("numrex").as_string() : "NULL") << "\"" << "," <<
                                    "\"" << (rex.has_string_field("codereference")
                                         ? rex.at("codereference").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (rex.has_string_field("codepublic") ? rex.at("codepublic").as_string() : "NULL") << "\"" <<
                                    "," <<
                                    "\"" << (rex.has_string_field("codemonnaie") ? rex.at("codemonnaie").as_string() : "NULL") << "\""
                                    << "," <<
                                    "\"" << (rex.has_string_field("codetauxmonnaie") ? rex.at("codetauxmonnaie").as_string() : "NULL") << "\"" << "," <<
                                    "\"" << (rex.has_string_field("gainfinancierrex")
                                         ? rex.at("gainfinancierrex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (rex.has_string_field("gainfinancierperioderex")
                                         ? rex.at("gainfinancierperioderex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (rex.has_string_field("energierex") ? rex.at("energierex").as_string() : "NULL") << "\"" <<
                                    "," <<
                                    "\"" << (rex.has_string_field("codeuniteenergie")
                                         ? rex.at("codeuniteenergie").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (rex.has_string_field("codeperiodeenergie")
                                         ? rex.at("codeperiodeenergie").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (rex.has_string_field("codeenergierex")
                                         ? rex.at("codeenergierex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (rex.has_string_field("gesrex") ? rex.at("gesrex").as_string() : "NULL") << "\"" << "," <<
                                    "\"" << (rex.has_string_field("ratiogainrex") ? rex.at("ratiogainrex").as_string() : "NULL") << "\""
                                    << "," <<
                                    "\"" << (rex.has_string_field("trirex") ? rex.at("trirex").as_string() : "NULL") << "\"" << "," <<
                                    "\"" << (rex.has_string_field("capexrex") ? rex.at("capexrex").as_string() : "NULL") << "\"" << ","
                                    <<
                                    "\"" << (rex.has_string_field("capexperioderex")
                                         ? rex.at("capexperioderex").as_string()
                                         : "NULL") << "\"" << "," <<
                                    "\"" << (rex.has_string_field("opexrex") ? rex.at("opexrex").as_string() : "NULL") << "\"" << "," <<
                                    "\"" << (rex.has_string_field("codeTechno1") ? rex.at("codeTechno1").as_string() : "NULL") << "\""
                                    << "," <<
                                    "\"" << (rex.has_string_field("codeTechno2") ? rex.at("codeTechno2").as_string() : "NULL") << "\""
                                    << "," <<
                                    "\"" << (rex.has_string_field("codeTechno3") ? rex.at("codeTechno3").as_string() : "NULL") << "\""
                                    << "," <<
                                    "\"" << (rex.has_string_field("codetravaux") ? rex.at("codetravaux").as_string() : "NULL") << "\""
                                    << "," <<
                                    "\"" << (rex.has_string_field("codereseau") ? rex.at("codereseau").as_string() : "NULL") << "\"" <<
                                    "," <<
                                    "\"" << (rex.has_string_field("codelicense") ? rex.at("codelicense").as_string() : "NULL") << "\""
                                    << "," <<
                                    "\"" << (rex.has_string_field("availablelangue")
                                         ? rex.at("availablelangue").as_string()
                                         : "NULL") << "\"" << endl;
                        }
                    }
                } else if (key == "solution_descriptions") {
                    for (const auto langs = v.as_object(); const auto& [code, descs]: langs) {
                        string int_code = code == "fr" ? "2" : code == "en" ? "3" : "4";
                        for (const auto desc_objs = descs.as_object(); const auto& [index, d]: desc_objs) {
                            string c(d.as_string());
                            replaceAll(c, "\"", "'");
                            replaceAll(c, "\n", " ");
                            textsolmodelstream << int_code << "," << index << "," << solution.at("numsolution") << ",\"" << c << "\"" <<
                                    endl;
                        }
                    }
                }
            }
        }
    }

    stringstream tblmonnaiestream;
    tblmonnaiestream << R"("nummonnaie","shortmonnaie")" << endl;

    for (const auto map_monnaie = req.get_tbl_monaie(); const auto& monnaie: map_monnaie) {
        tblmonnaiestream << monnaie.at("nummonnaie") << "," <<
                (monnaie.contains("shortmonnaie") ? monnaie.at("shortmonnaie") : "NULL") << endl;
    }


#ifdef MY_DEBUG
    cout << "Writing to files." << endl;
#endif


    std::ofstream tblcoutrex("./model/tblcoutrex.csv");
    tblcoutrex << tblcoutrexstream.str();
    std::ofstream tblgainrex("./model/tblgainrex.csv");
    tblgainrex << tblgainrexstream.str();
    std::ofstream tblrex("./model/tblrex.csv");
    tblrex << tblrexstream.str();
    std::ofstream textsolmodel("./model/textSolModel.csv");
    textsolmodel << textsolmodelstream.str();
    std::ofstream tblmonnaie("./model/tblmonnaie.csv");
    tblmonnaie << tblmonnaiestream.str();


    tblcoutrex.close();
    tblgainrex.close();
    tblrex.close();
    textsolmodel.close();
    tblmonnaie.close();
}

restapp::~restapp() = default;
