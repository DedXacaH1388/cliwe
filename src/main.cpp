#include <cstdio>
#include <vector>
#include <string>
#include <cstdlib>
#include <sstream>
#include <exception>

#include <fmt/format.h>

#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

#include <boost/json/src.hpp>
#include <boost/json/array.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/object.hpp>
#include <boost/json/serialize.hpp>
#include <boost/throw_exception.hpp>
#include <boost/json/string_view.hpp>
#include <boost/json/system_error.hpp>
#include <boost/json/stream_parser.hpp>
#include <boost/exception/exception.hpp>

#include "headers/ascii.h"

#define API_KEY "PUT_YOUR_API_KEY_HERE"

namespace json = boost::json;
using std::string;
using std::vector;

struct Weather_s {
  long id;
  string main_w;
  string descr;
};

struct Main_s {
  double temp;
  double feels_like;
  long pressure;
  long humidity;
};

struct Wind_s {
  double speed;
  long deg;
  double gust;
};

struct Snow_s {
  double one_h;
};

struct Rain_s {
  double one_h;
};

struct Full_data {
  Weather_s weather;
  Main_s main_s;
  long vis;
  Wind_s wind;
  Snow_s snow;
  Rain_s rain;
  string name;
};

string colorize(string text, string color_code) {
  return fmt::format("\033[{}m{}\033[0m", color_code, text);
}

string draw_weather(double temp) {
  std::map<double, int> color_map = {
    {-15, 21}, {-12, 27}, {-9, 33}, {-6, 39}, {-3, 45},
    {0, 51}, {2, 50}, {4, 49}, {6, 48}, {8, 47},
    {10, 46}, {13, 82}, {16, 118}, {19, 154}, {22, 190},
    {25, 226}, {28, 220}, {31, 214}, {34, 208}, {37, 202},
  };
  
  for (auto color : color_map) {
    if (temp < color.first) {
      return colorize(fmt::format("{}", temp),
                      fmt::format("38;5;{}", color.second));
    }
  }
  return colorize(fmt::format("{}", temp), "38;5;196");
}

string draw_weather(int temp) {
  std::map<double, int> color_map = {
    {-15, 21}, {-12, 27}, {-9, 33}, {-6, 39}, {-3, 45},
    {0, 51}, {2, 50}, {4, 49}, {6, 48}, {8, 47},
    {10, 46}, {13, 82}, {16, 118}, {19, 154}, {22, 190},
    {25, 226}, {28, 220}, {31, 214}, {34, 208}, {37, 202},
  };
  
  for (auto color : color_map) {
    if (temp < color.first) {
      return colorize(fmt::format("{}", temp),
                      fmt::format("38;5;{}", color.second));
    }
  }
  return colorize(fmt::format("{}", temp), "38;5;196");
}

string draw_wind(double wind_speed) {
  std::map<double, int> color_map = {
    {3, 241}, {6, 242}, {9, 243}, {12, 246}, {15, 250},
    {19, 253}, {23, 214}, {27, 208}, {31, 202}, {-1, 196}
  };

  for (auto color : color_map) {
    if (wind_speed < color.first) {
      return colorize(fmt::format("{}", wind_speed),
                      fmt::format("38;5;{}", color.second));
    }
  }
  return colorize(fmt::format("{}", wind_speed), "38;5;196");
}

void print_weather(json::value weather_json) {
  try {
    boost::json::object *weather_obj = weather_json.if_object();
    boost::json::array *weather_arr = weather_json.at("weather").if_array();
    
    if (!weather_obj) {
      printf("\033[0;31mError 0x01.\n"
             "\033[0;33mNot an object.\033[0m");
      exit(EXIT_FAILURE);
    }
    if (!weather_arr) {
      printf("\033[0;31mError 0x02.\n"
             "\033[0;33mNot an array.\033[0m");
      exit(EXIT_FAILURE);
    }
    
    Weather_s weather;
    Main_s main_d;
    Wind_s wind;
    
    try {
      weather = {
        weather_arr->at(0).at("id").as_int64(),
        (string)weather_arr->at(0).at("main").as_string().c_str(),
        (string)weather_arr->at(0).at("description").as_string().c_str()
      };
    } catch (std::exception &e) {
      printf("\033[0;31mError 0x03.\n"
             "\033[0;36mError in weather array. \033[0;33mError message: %s\033[0m\n", e.what());
      exit(EXIT_FAILURE);
    }

    try {
      double feels_like = .0;
      if (weather_obj->at("main").at("feels_like").is_double()) {
        feels_like = weather_obj->at("main").at("feels_like").as_double();
      } else {
        feels_like = (double)weather_obj->at("main").at("feels_like").as_int64();
      }
      main_d = {
        weather_obj->at("main").at("temp").as_double(),
        feels_like,
        weather_obj->at("main").at("pressure").as_int64(),
        weather_obj->at("main").at("humidity").as_int64()
      };
    } catch (std::exception &e) {
      printf("\033[0;31mError 0x04.\n"
             "\033[0;36mError in weather.main part. \033[0;33mError message: %s\033[0m\n", e.what());
      exit(EXIT_FAILURE);
    }

    try {
      wind = {
        weather_obj->at("wind").at("speed").as_double(),
        weather_obj->at("wind").at("deg").as_int64(),
        weather_obj->at("wind").at("gust").as_double()
      };
    } catch (std::exception &e) {
      printf("\033[0;31mError 0x05.\n"
             "\033[0;36mError in wind part. \033[0;33mError message: %s\033[0m\n", e.what());
      exit(EXIT_FAILURE);
    }

    Snow_s snow;
    snow.one_h = .0;
    try {
      if (auto p = weather_obj->if_contains("snow"))
        snow.one_h = p->at("1h").as_double();
    } catch (std::exception &e) {
      printf("\033[0;31mError 0x04.\n"
             "\033[0;36mError in snow structure. \033[0;33mError message: %s\033[0m\n", e.what());
      exit(EXIT_FAILURE);
    }

    Rain_s rain;
    rain.one_h = .0;
    try {
      if (auto p = weather_obj->if_contains("rain"))
        rain.one_h = p->at("1h").as_double();
    } catch (std::exception &e) {
      printf("\033[0;31mError 0x05.\n"
             "\033[0;36mError in rain structure. \033[0;33mError message: %s\033[0m\n", e.what());
      exit(EXIT_FAILURE);
    }

    Full_data out_data;
    try {
      out_data = {
        weather,
        main_d,
        weather_obj->at("visibility").as_int64(),
        wind,
        snow,
        rain,
        (string)weather_obj->at("name").as_string().c_str()
      };
    } catch (std::exception &e) {
      printf("\033[0;31mError 0x06.\n"
             "\033[0;36mError in out_data structure. \033[0;33mError message: %s\033[0m\n", e.what());
      exit(EXIT_FAILURE);
    }
    
    vector<string> ascii_art;
    try {
      for (auto code : weather_codes_map) {
        if (code.first == out_data.weather.id) {
          ascii_art = code.second;
        }
      }
    } catch (std::exception &e) {
      printf("\033[0;31mError 0x07\n"
             "\033[0;36mError in ascii_art. \033[0;33mError message: %s\033[0m\n", e.what());
      exit(EXIT_FAILURE);
    }
    
    double one_h = .0;
    try {
      if (out_data.snow.one_h > .0) {
        one_h = out_data.snow.one_h;
      } else if (out_data.rain.one_h > .0) {
        one_h = out_data.rain.one_h;
      } else {
        one_h = .0;
      }
    } catch (std::exception &e) {
      one_h = .1;
    }

    long deg = 0l;
    string wind_direction = "";
    try {
      deg = out_data.wind.deg;
      wind_direction = "";
      if (((deg >= 337l) || (deg < 22l)) && (deg >= 0l)) {
        wind_direction = "↓";
      } else if ((deg >= 22l) && (deg < 67l)) {
        wind_direction = "↙";
      } else if ((deg >= 67l) && (deg < 112l)) {
        wind_direction = "←";
      } else if ((deg >= 112l) && (deg < 157l)) {
        wind_direction = "↖";
      } else if ((deg >= 157l) && (deg < 202l)) {
        wind_direction = "↑";
      } else if ((deg >= 202l) && (deg < 247l)) {
        wind_direction = "↗";
      } else if ((deg >= 247l) && (deg < 292l)) {
        wind_direction = "→";
      } else if ((deg >= 292l) && (deg < 337l)) {
        wind_direction = "↘";
      } else {
        wind_direction = "?";
      }
    } catch (std::exception &e) {
      printf("\033[0;31mError 0x08.\n"
             "\033[0;36mError in wind direction checker. \033[0;33mError message: %s\033[0m\n", e.what());
      exit(EXIT_FAILURE);
    }

    string format_string[] = {"", "", "", "", ""};
    try {
      format_string[0] = fmt::format("{} {}\r\n",
                           ascii_art[0], out_data.weather.descr);
      format_string[1] = fmt::format("{} {}({}) °C\r\n",
                           ascii_art[1], draw_weather(out_data.main_s.temp), draw_weather((int)out_data.main_s.feels_like));
      format_string[2] = fmt::format("{} {} {}\r\n",
                           ascii_art[2], wind_direction, draw_wind(out_data.wind.speed));
      format_string[3] = fmt::format("{} {} km\r\n",
                           ascii_art[3], (double)out_data.vis/1000);
      format_string[4] = fmt::format("{} {}\r\n",
                           ascii_art[4], one_h);
    } catch (std::exception &e) {
      printf("\033[0;31mError 0x09.\n"
             "\033[0;36mError in print function. \033[0;33mError message: %s\033[0m\n", e.what());
      exit(EXIT_FAILURE);
    }
    
    try {
      for (int i = 0; i < 5; i++)
        std::cout << format_string[i];
    } catch (std::exception &e) {
      printf("\033[0;31mError 0x0A.\n"
             "\033[0;36mError in out. \033[0;33mError message: %s\033[0m", e.what());
      exit(EXIT_FAILURE);
    }
  } catch (std::exception &e) {
    printf("\033[0;31mError 0x0B.\n"
           "\033[0;36mError in print_weather function. \033[0;33mError message: %s\033[0m\n", e.what());
    exit(EXIT_FAILURE);
  }
}

int main() {
  json::value weather_json;
  json::error_code ec;
  std::stringstream tmp;

  string url = fmt::format("https://api.openweathermap.org/data/2.5/weather"
      "?q=Molodechno"
      "&units=metric"
      "&appid={}", API_KEY);

  try {
    curlpp::Cleanup cleaner;
    curlpp::Easy request;

    request.setOpt(new curlpp::options::Url(url));
    request.setOpt(curlpp::Options::WriteStream(&tmp));
    request.perform();

    string tmpp = tmp.str().c_str();

    weather_json = json::parse(tmpp, ec);

    print_weather(weather_json);
  }
  catch ( curlpp::LogicError & e ) {
    printf("\033[0;31mError 0x0C.\n"
           "\033[0;36mLogic error. \033[0;33mError message: %s\n\033[0m\n",
           e.what());
  }
  catch ( curlpp::RuntimeError & e ) {
    printf("\033[0;31mError 0x0D.\n"
           "\033[0;36mRuntime error. \033[0;33mError message: %s\n\033[0m\n",
           e.what());
  } catch (std::exception &e) {
    printf("\033[0;31mError 0x0E.\n"
           "\033[0;36mUnknown exception. \033[0;33mError message: %s\n\033[0m\n",
           e.what());
  }
  return EXIT_SUCCESS;
}
