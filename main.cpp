#include <string>
#include <vector>
#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>

using web::http::http_request, web::http::status_codes, web::http::methods;
using web::http::experimental::listener::http_listener;
using web::json::value, web::json::array;
using std::vector, std::string, std::map, std::cout, std::endl, std::pair;
using concurrency::streams::istream;

string read_body_to_end(const istream& stream) {
    auto stream_buffer = stream.streambuf();
    string ret;

    while (!stream_buffer.is_eof()) {
        ret += static_cast<char>(stream_buffer.nextc().get());
    }

    return ret;

}

map<string, string> parse_form_input(const string& input) {
    string s = input;
    const string delim = "-----formdata-undici-";
    const string name_delim = "name=\"";
    map<string, string> ret;

    size_t pos = s.find(delim);
    while (pos != string::npos) {
        s.erase(0,  pos + delim.size() + 14);
        pos = s.find(delim);
        if (pos != string::npos) {
            string tok = s.substr(0, pos);
            if (const size_t name_start = tok.find(name_delim); name_start != string::npos) {
                tok.erase(0, name_start + name_delim.size());
                if (const size_t name_end = tok.find('"'); name_end != string::npos) {
                    const string name = tok.substr(0, name_end);
                    tok.erase(0, name_end + 1);
                    const string content = tok.substr(0, tok.length() - 1);
                    ret.emplace(name, content);
                }
            }
        }
    }

    return ret;
}

void get_items(const http_request& request) {

    // auto test = request.body().streambuf();


    const auto fd = read_body_to_end(request.body());
    cout << fd << endl;
    auto a = parse_form_input(fd);
    const value response = value::object();

    auto req = request.reply(status_codes::OK, response);
}

[[noreturn]] int main() {
    http_listener listener("http://localhost:8080");
    listener.support(methods::POST, get_items);
    const auto req = listener.open();

    std::cout << "Server listening on http://localhost:8080" << std::endl;

    while (true) {
    }

}
