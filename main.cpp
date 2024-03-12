#include <string>
#include <vector>
#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>

using web::http::http_request, web::http::status_codes, web::http::methods, web::http::http_response;
using web::http::experimental::listener::http_listener;
using web::json::value, web::json::array;
using std::vector, std::string, std::map, std::cout, std::endl, std::pair;
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
    size_t name_start = 0;
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
    const value json_rep = value::object({{"aaa", value{"bbb"}}});

    http_response response{status_codes::OK};

    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.set_body(json_rep);

    auto req = request.reply(response);
}

/**
 * Main function, Opens the port and waits for requests.
 *
 */
[[noreturn]] int main() {
    http_listener listener("http://localhost:8080");
    listener.support(methods::POST, get_items);
    listener.support(methods::GET, [](const http_request& request) {
        cout << "No, not yet." << endl;
        cout << request.relative_uri().to_string() << endl;
        auto _ = request.reply(status_codes::NotImplemented);
        //TODO add support to get requests
    });
    const auto req = listener.open();

    std::cout << "Server listening on http://localhost:8080" << std::endl;

    string input;
    while (input.empty()) {
        std::cin >> input;
    }
}
