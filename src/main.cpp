#include <vector>
#include <string>
#include <cstdlib>
#include <sstream>
#include <exception>

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

void print_weather(json::value weather_json) {
  try {
    boost::json::object *weather_obj = weather_json.if_object();
    boost::json::array *weather_arr = weather_json.at("weather").if_array();

    if (!weather_obj) {
      printf("Error 0x01.\n"
             "Not an object.");
      exit(EXIT_FAILURE);
    }
    if (!weather_arr) {
      printf("Error 0x02.\n"
             "Not an array.");
      exit(EXIT_FAILURE);
    }
    
    Weather_s weather;
    Main_s main_d;
    Wind_s wind;
    
    try {
      weather = {
        weather_arr->at(0).at("id").as_int64(),
        weather_arr->at(0).at("main").as_string().c_str(),
        weather_arr->at(0).at("description").as_string().c_str()
      };
    } catch (std::exception &e) {
      printf("Error 0x03.\n"
             "Error in weather array. Error message: %s", e.what());
      exit(EXIT_FAILURE);
    }

    try {
      main_d = {
        weather_obj->at("main").at("temp").as_double(),
        weather_obj->at("main").at("feels_like").as_double(),
        weather_obj->at("main").at("pressure").as_int64(),
        weather_obj->at("main").at("humidity").as_int64()
      };
    } catch (std::exception &e) {
      printf("Error 0x04.\n"
             "Error in weather.main part. Error message: %s", e.what());
      exit(EXIT_FAILURE);
    }

    try {
      wind = {
        weather_obj->at("wind").at("speed").as_double(),
        weather_obj->at("wind").at("deg").as_int64(),
        weather_obj->at("wind").at("gust").as_double()
      };
    } catch (std::exception &e) {
      printf("Error 0x05.\n"
             "Error in wind part. Error message: %s", e.what());
      exit(EXIT_FAILURE);
    }

    Snow_s snow;
    snow.one_h = .0;
    try {
      if (auto p = weather_obj->if_contains("snow"))
        snow.one_h = p->at("1h").as_double();
    } catch (std::exception &e) {
      printf("Error 0x04.\n"
             "Error in snow structure. Error message: %s", e.what());
      exit(EXIT_FAILURE);
    }

    Rain_s rain;
    rain.one_h = .0;
    try {
      if (auto p = weather_obj->if_contains("rain"))
        rain.one_h = p->at("1h").as_double();
    } catch (std::exception &e) {
      printf("Error 0x05.\n"
             "Error in rain structure. Error message: %s", e.what());
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
        weather_obj->at("name").as_string().c_str()
      };
    } catch (std::exception &e) {
      printf("Error 0x06.\n"
             "Error in out_data structure. Error message: %s", e.what());
      exit(EXIT_FAILURE);
    }

    vector<string> ascii_art;
    try {
      for (int i = 0; i < 20; i++) {
        if (weather_codes[i].code == out_data.weather.id) {
          ascii_art = weather_codes[i].ascii_art;
        }
      }
    } catch (std::exception &e) {
      printf("Error 0x07\n"
             "Error in ascii_art. Error message: %s", e.what());
      exit(EXIT_FAILURE);
    }

    try {
      printf("%s %s\n"
             "%s %.1lf(%.1lf) °C\n"
             "%s %s %.1lf\n"
             "%s %.1lf km\n"
             "%s %.1lf\n",
             ascii_art[0].c_str(), out_data.weather.descr.c_str(),
             ascii_art[1].c_str(), out_data.main_s.temp, out_data.main_s.feels_like,
             ascii_art[2].c_str(), "↓", out_data.wind.speed,
             ascii_art[3].c_str(), (double)out_data.vis/1000,
             ascii_art[4].c_str(), (out_data.snow.one_h < out_data.rain.one_h) 
             ? out_data.rain.one_h : out_data.snow.one_h);
    } catch (std::exception &e) {
      printf("Error 0x08.\n"
             "Error in print function. Error message: %s", e.what());
      exit(EXIT_FAILURE);
    }
  } catch (std::exception &e) {
    printf("Error 0x09.\n"
           "Error in json object. Error message: %s", e.what());
    exit(EXIT_FAILURE);
  }
}

int main() {
  json::value weather_json;
  json::error_code ec;
  std::stringstream tmp;

  string url = "https://api.openweathermap.org/data/2.5/weather"
      "?q=Molodechno"
      "&units=metric"
      "&appid=";

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
    std::cout << e.what() << std::endl;
  }
  catch ( curlpp::RuntimeError & e ) {
    std::cout << e.what() << std::endl;
  }

  return EXIT_SUCCESS;
}
