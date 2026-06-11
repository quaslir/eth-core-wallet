#include "drivers/price_client.hpp"
#include "api/http.hpp"
#include "api/json.hpp"
#include "config/configuration.hpp"
#include <map>
#include <string>
#include <iostream>
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

    const std::string url = "https://api.coingecko.com/api/v3/simple/price?ids=" + fsyms +
                                    "&vs_currencies=usd&x_cg_demo_api_key=" + COINGECKO_API;
    std::string buffer = http::get_request(url);
    json j = json::parse(buffer);

    for (const auto &symbol : symbols) {
      if (j.contains(symbol) && j[symbol].contains("usd")) {

        result[symbol] = j[symbol]["usd"].get<double>();
      } else
        result[symbol] = NAN;
    }
  } catch (const std::exception &err) {
      std::cerr << err.what();
    for (const auto &symbol : symbols) {
      result[symbol] = NAN;
    }
  }

  return result;
}
