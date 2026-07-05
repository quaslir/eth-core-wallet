#pragma once
#include "api/json.hpp"
#include <exception>
#include <fstream>
#include <string>

class Configuration {
private:
  std::string COINGECKO_API{};
  std::string ALCHEMY_API_KEY{};
  std::string ETHERSCAN_API_KEY{};

  Configuration() = default;
  Configuration(const Configuration &) = delete;
  Configuration &operator=(const Configuration) = delete;
  Configuration(Configuration &&) = delete;
  Configuration &operator=(Configuration &&) = delete;

public:
  static constexpr auto MIN_Y = 3;

  static constexpr auto BALANCE_TIMEOUT = 10000;
  static constexpr auto TRANSACTION_TIMEOUT = 60000;
  static constexpr auto GWEI_TIMEOUT = 10000;
  static constexpr auto FULL_UPDATE_TIMEOUT = 10000;
  static constexpr auto TX_STATUS_UPDATE_TIMEOUT = 3000;

  static constexpr auto WEI_TO_ETH = "1000000000000000000";
  static constexpr auto WEI_TO_GWEI = "1000000000";

  inline bool init(const std::string &filename) {
    try {
      std::ifstream file(filename);
      if (!file.is_open())
        return false;

      json j;
      file >> j;
      COINGECKO_API = j["coingecko_key"].get<std::string>();
      ALCHEMY_API_KEY = j["alchemy_key"].get<std::string>();
      ETHERSCAN_API_KEY = j["etherscan_key"].get<std::string>();
      return true;
    } catch (const std::exception &err) {
      return false;
    }
  }

  inline static Configuration &get_instance(void) {
    static Configuration instance;
    return instance;
  }

  inline const std::string &get_coingecko_api(void) const {
    return COINGECKO_API;
  }

  inline const std::string &get_alchemy_api(void) const {
    return ALCHEMY_API_KEY;
  }
  inline const std::string &get_etherscan_api(void) const {
    return ETHERSCAN_API_KEY;
  }

  inline void create_configuration(const std::string &filename) const {
    try {
      std::ofstream file(filename);
      if (!file.is_open())
        return;
      json j;
      j["coingecko_key"] = "YOUR COINGECKO API KEY";
      j["alchemy_key"] = "YOUR ALCHEMY API KEY";
      j["etherscan_key"] = "YOUR ETHERSCAN API KEY";
      file << j.dump();
    } catch (const std::exception &err) {
      return;
    }
  }
};
