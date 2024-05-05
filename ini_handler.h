/**
 * @file ini_handler.h
 * @brief Header file containing declaration of library functionalities.
 */

#ifndef LIB_INI_HANDLER_H
#define LIB_INI_HANDLER_H

#include <string>

/**
 * @brief Load and parse an INI resource file located in the Linux file system.
 *
 * This function loads and parses an INI resource file located in the Linux file system.
 * It reads the contents of the file and stores it in volatile memory for further processing.
 *
 * @param path The path to the INI resource file.
 * @return 0 if successful.
 * @return 1 if there are read errors.
 * @return 255 for generic errors.
 */
unsigned short load_resource(const std::string& path);

/**
 * @brief Retrieve the value of a key from the loaded INI resource.
 *
 * This function retrieves the value associated with a given key from the INI resource
 * that has been previously loaded using the load_resource function.
 *
 * @param key The key whose value is to be retrieved.
 * @param value A reference to a string where the retrieved value will be stored.
 * @return 0 if successful.
 * @return 3 if the key is missing.
 * @return 4 if a resource file has not been loaded yet.
 * @return 255 for generic errors.
 */
unsigned short get_value(const std::string& key, std::string& value);

/**
 * @brief Store or update the value of a key in the loaded INI resource and the INI file.
 *
 * This function stores or updates the value associated with a given key in the loaded INI resource
 * and also updates the corresponding entry in the INI file on the filesystem.
 *
 * @param key The key whose value is to be stored or updated.
 * @param value The value to be stored.
 * @return 0 if successful
 * @return 4 if a resource file has not been loaded yet
 * @return 255 for generic errors.
 */
unsigned short set_value(const std::string& key, const std::string& value);


#endif // LIB_INI_HANDLER_H
