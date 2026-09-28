
#include <iostream> // for input/output
#include <string> // for std::string
#include <vector> // for std::vector (holds 4 octet strings)
#include <algorithm> // for std::count
#include <cctype> // for std::isdigit
#include <sstream> // for std::stringstream (splits "a.b.c.d" into octets)

// Helper function to manually convert a string segment to an integer,
// validating against the 0-255 range and the leading zero rule.
// Returns the value if valid, or -1 if invalid.
int manual_to_int(const std::string& s, int max_val, bool& isValid) {
    // Rejects empty strings and anything longer than 5 digits for port numbers
    if (s.empty() || s.length() > 5) {
        isValid = false;
        return -1;
    }

    // Check for leading zeros rule: only "0" is valid, "00" is not.
    if (s.length() > 1 && s[0] == '0') {
        isValid = false;
        return -1;
    }

    long value = 0;
    // Check each character to ensure digits only, build integer value
    for (char c : s) {
        if (!std::isdigit(c)) {
            isValid = false;
            return -1;
        }
        value = value * 10 + (c - '0'); // Converts char digit to int value
    }

    // Validate the final integer value against maximum possible value
    if (value >= 0 && value <= max_val) {
        isValid = true;
        return static_cast<int>(value); // Type cast to int for return
    } else {
        isValid = false;
        return -1;
    }
}

/**
 * @brief Extracts one valid IPv4 address (optionally with a port) hidden inside a line of arbitrary text,
 *        strictly validating it according to complex rules.
 *
 * @param str The line of text to search within.
 * @param outAddress Reference to store the 32-bit address value on success.
 * @param outPort Reference to store the port number (or -1 if no port present).
 * @return true if a valid IPv4 address/port combination was found and extracted, false otherwise.
 */
bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort) {
    // Implementation based on BUG 2: Scan character by character for tokens
    // where consecutive characters are digits, '.', or ':'

    // Lambda function to validate a token string as a potential IPv4 address with optional port
    auto tryValidateToken = [&](const std::string& token_str) -> bool {


        // Need to split off the port if present
        size_t colon_pos = token_str.find(':');
        std::string address_str = token_str;
        bool has_port = false;
        std::string port_str;

        if (colon_pos != std::string::npos) {
            if (token_str.find(':', colon_pos + 1) != std::string::npos) {
                return false; // Multiple colons found
            }
            address_str = token_str.substr(0, colon_pos);
            port_str = token_str.substr(colon_pos + 1);
            has_port = true;
            if (port_str.empty()) {
                return false; // Port is empty
            }
        }



        // Validate the address part, ensure 3 dots and 4 octets
        size_t dot_count = std::count(address_str.begin(), address_str.end(), '.');
        if (dot_count != 3) {
            return false; // Invalid IPv4 format
        }

        // Split address into octets
        std::vector<std::string> octets;
        std::stringstream ss(address_str);
        std::string octet;
        // getline reads until delimiter ('.') and stores each octet as a string in the vector
        while (std::getline(ss, octet, '.')) {
            octets.push_back(octet);
        }
        if (octets.size() != 4) {
            return false; // Should have exactly 4 octets
        }

        // Validate each octet and convert to integer value
        int octet_values[4];
        for (int i = 0; i < 4; i++) {
            bool valid_octet;
            octet_values[i] = manual_to_int(octets[i], 255, valid_octet);
            if (!valid_octet) {
                return false;
            }
        }

        // Validate the port if present
        int port_value = -1;
        if (has_port) {
            bool valid_port;
            port_value = manual_to_int(port_str, 65535, valid_port);
            if (!valid_port) {
                return false;
            }
        }

        // All validations passed, compute the 32-bit address
        // "<< 24" shifts the first octet to the highest byte
        // or bitwise used to combine all octets into single unsigned long value
        outAddress = (static_cast<unsigned long>(octet_values[0]) << 24) |
                     (static_cast<unsigned long>(octet_values[1]) << 16) |
                     (static_cast<unsigned long>(octet_values[2]) << 8) |
                     (static_cast<unsigned long>(octet_values[3]));
        outPort = port_value;
        return true;
    };


    // Scan the input string character by character to extract potential candidate tokens
    std::string current_token;
    for (char c : str) {
        if (std::isdigit(c) || c == '.' || c == ':') {
            current_token += c;
        } else {
            // End of a token (non-digit, non-dot, non-colon character encountered)
            // Validate current token isn't empty and ensure it meets IPv4 format standards
            if (!current_token.empty() && tryValidateToken(current_token)) {
                return true; // Valid token found + extracted
            }
            current_token.clear();
        }
    }

    // Check the last token after loop ends
    if (!current_token.empty() && tryValidateToken(current_token)) {
        return true;
    }

    // If no valid token was found after scanning the whole line
    return false;
}

int main() {
    std::string input;
    while (true) {
        std::cout << "Enter a string (or 'END' to quit): ";
        if (!std::getline(std::cin, input)) {
            break; // Input ended
        }
        if (input == "END") {
            std::cout << "Program terminated." << std::endl;
            break;
        }
        unsigned long address = 0;
        int port = -1;
        if (!extractIPv4(input, address, port)) {
            std::cout << "Invalid input: no valid IPv4 address found" << std::endl;
        } else {
            unsigned long a = (address >> 24) & 0xFF;
            unsigned long b = (address >> 16) & 0xFF;
            unsigned long c = (address >> 8) & 0xFF;
            unsigned long d = address & 0xFF;

            std::cout << "Extracted IPv4 address: " << a << "." << b << "." << c << "." << d << " (decimal value: " << address << ", port: " << (port == -1 ? "none" : std::to_string(port)) << ")" << std::endl;
        }
    }
    return 0;
}