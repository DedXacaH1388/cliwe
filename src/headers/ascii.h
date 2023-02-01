#include <vector>
#include <string>

#ifndef ASCII_H
#define ASCII_H

struct weather_codes_s {
    int code;
    std::vector<std::string> ascii_art;
};

extern weather_codes_s weather_codes[];

#endif
