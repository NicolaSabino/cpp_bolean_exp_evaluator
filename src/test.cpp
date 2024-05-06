#include <iostream>
#include <cassert>
#include "ini_handler.h"

constexpr const char *CONFIG_FILE = "test_file/config.ini";
constexpr const char *NOT_EXISTING_FILE = "foo.ini";

int main()
{
    std::string value;
    unsigned short result;

    result = get_value("Dummy", value);
    assert(result == 4);

    result = set_value("Dummy", value);
    assert(result == 4);

    result = load_resource(NOT_EXISTING_FILE);
    assert(result == 1);

    result = load_resource(CONFIG_FILE);
    assert(result == 0);

    result = get_value("Advanced.key5.subsection", value);
    assert(result == 0);
    assert(value == "value5");

    result = get_value("Not.existing.key", value);
    assert(result == 3);

    result = set_value("Not.existing.key", "foo");
    delete_value("Not.existing.key");

    return 0;
}
