#include <charconv>
#include <string>
#include <vector>
#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <sqlite3.h>

using web::http::http_request, web::http::status_codes, web::http::methods, web::http::http_response;
using web::http::experimental::listener::http_listener;
using web::json::value, web::json::array;
using std::vector, std::string, std::map, std::cout, std::endl, std::pair, std::from_chars, std::errc, std::cerr;
using concurrency::streams::istream;

volatile char* database_name;

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
void get_items(const http_request& request) {
    // auto test = request.body().streambuf();

    cout << "recived" << endl;
    const auto fd = read_body_to_end(request.body());
    cout << fd << endl;
    auto a = parse_form_input(fd);
    const value json_rep = value::array({value{5}, value{9}, value{6174}});

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

void db_req(const string& name, const string& sol_num, const string& code_lang) {
    sqlite3 *db;
    char* z_err_msg = nullptr;


    const string db_name = string("file://") + name;

    if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
    }
    else {
        cout << "Database opened successfuly." << endl;
    }

    const string sql = "";

    if(const auto data = "Callback function called";
        sqlite3_exec(db, sql.c_str(), nullptr, (void*)data, &z_err_msg) != SQLITE_OK) {
        cerr << "SQL error: " << z_err_msg << endl;
        sqlite3_free(z_err_msg);
        }
    else {
        cout << "Operation done successfuly" << endl;
    }

    sqlite3_close(db);
}

void get_solution(const http_request& request) {
    const auto uri = request.relative_uri();
    const auto parsed_uri = parse_uri(uri.path());
    const string & sol_num = parsed_uri[0];
    const string & code_lang = parsed_uri[1];




    auto _ = request.reply(status_codes::OK);
}

static int callback(void* data, int argc, char** argv, char** az_col_name) {
    cerr << static_cast<const char *>(data) << endl;

    for (int i = 0; i < argc; ++i) {
        cout << az_col_name[i] << " = " << (argv[i] ? argv[i] : nullptr) << "\n";
    }

    cout << endl;

    return 0;
}


/**
 * Main function, Opens the port and waits for requests.
 *
 */
[[noreturn]] int main(const int argc, char** argv) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <sqlite_dbpath>" << endl;
        exit(EXIT_FAILURE);
    }

    database_name = argv[1];


    http_listener listener("http://localhost:8080/");
    listener.support(methods::POST, get_items);
    listener.support(methods::GET, get_solution);
    const auto req = listener.open();

    std::cout << "Server listening on http://localhost:8080" << std::endl;

    string input;
    while (input.empty()) {
        std::cin >> input;
    }
}
