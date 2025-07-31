#include "common.h"

std::vector<std::string> split(std::string& s, const std::string& delimiter)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);

    return tokens;
}

std::vector<std::string> split(std::string& s, const char delimiter)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + 1);
    }
    tokens.push_back(s);
    return tokens;
}

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

static bool _is_base64(unsigned char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
        || (c >= '0' && c <= '9') || c == '+' || c == '/';
}

std::string base64_encode(const std::string& input)
{
    std::string output;
    int val = 0, valb = -6;
    size_t len = input.length();

    for (size_t i = 0; i < len; ++i) {
        unsigned char c = input[i];
        val             = (val << 8) + c;
        valb += 8;

        while (valb >= 0) {
            output.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }

    if (valb > -6) {
        output.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }

    while (output.size() % 4) {
        output.push_back('=');
    }

    return output;
}

std::string base64_decode(const std::string& input)
{
    std::string output;
    std::vector<unsigned char> T(256, 0xFF);

    for (size_t i = 0; i < base64_chars.size(); ++i) {
        T[base64_chars[i]] = i;
    }

    int val = 0, valb = -8;
    size_t len = input.length();

    for (size_t i = 0; i < len; ++i) {
        unsigned char c = input[i];

        if (_is_base64(c)) {
            val = (val << 6) + T[c];
            valb += 6;

            if (valb >= 0) {
                output.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
    }

    return output;
}
