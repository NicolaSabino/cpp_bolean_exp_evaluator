#include <iostream>
#include <cassert>
#include "ini_handler.h"

constexpr const char* CONFIG_FILE = "test_file/config.ini";
constexpr const char* NOT_EXISTING_FILE = "foo.ini";
constexpr const char* MALFORMED_FILE = "test_file/malformed.ini";


int main() {
    // Load the resource file
    const auto loadResult = load_resource(CONFIG_FILE);
    assert(loadResult == 0);

    const auto notExistingFile = load_resource(NOT_EXISTING_FILE);
    assert(notExistingFile == 1);

    const auto malformedFile = load_resource(MALFORMED_FILE);
    assert(notExistingFile == 1);

    return 0;
}
