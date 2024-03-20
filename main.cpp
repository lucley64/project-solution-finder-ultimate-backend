#include <charconv>
#include <string>
#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>

#include "restapp.h"

using std::string, std::cout, std::endl, std::errc, std::cerr;


/**
 * Main function, Opens the port and waits for requests.
 *
 */
int main(const int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <sqlite_dbpath> [option]" << endl;
        exit(EXIT_FAILURE);
    }


    const string database_name = argv[1];
    restapp app(
        "file:///home/cytech/Travail/S4/Projects/project-solution-finder-ultimate/project-solution-finder-ultimate-backend/plateforme.db");

    if (argc > 2 && string(argv[2]) == "--export") {
        app.export_data();
        if (argc > 3 && string(argv[2]) == "--noexec") {
            exit(EXIT_SUCCESS);
        }
    }
    app.start();
    int input = 0;
    while (input == 0) {
        input = std::cin.get();
    }
}
