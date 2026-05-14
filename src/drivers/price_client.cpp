#include "drivers/price_client.hpp"
#include "api/http.hpp"
#include "api/json.hpp"
#include "config/configuration.hpp"
#include <map>
#include <string>

std::map<std::string, double>
price_manager::request_prices(const std::vector<std::string> &symbols) {
  std::map<std::string, double> result;
  if (symbols.empty())
    return result;

  try {
    std::string fsyms;

    for (size_t i = 0; i < symbols.size(); i++) {
      if (i > 0)
        fsyms += ',';
      fsyms += symbols[i];
    }

    const std::string url =
        MIN_URL_BATCH + fsyms + "&tsyms=USD&api_key=" + MIN_API;
    std::string buffer = http::get_request(url);
    json j = json::parse(buffer);

    for (const auto &symbol : symbols) {
      if (j.contains(symbol) && j[symbol].contains("USD")) {
        result[symbol] = j[symbol]["USD"].get<double>();

      } else
        result[symbol] = NAN;
    }
  } catch (const std::exception &err) {
    for (const auto &symbol : symbols) {
      result[symbol] = NAN;
    }
  }

  return result;
}
