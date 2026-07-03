#include "drivers/tokens_metadata_client.hpp"
#include "api/http.hpp"
#include "core/assets.hpp"
#include "core/supported_networks.hpp"
#include "utils/tech_utils.hpp"
#include <exception>
#include <optional>
#include <string>

std::optional<Asset> TokensMetadata::fetch_token_metadata_by_contract_addr(
    const std::string &contract_addr) const {
  auto call = [this,
               &contract_addr](const std::string &selector) -> std::string {
    json j;
    j["jsonrpc"] = "2.0";
    j["id"] = 1;
    j["method"] = "eth_call";

    json params;
    params["to"] = contract_addr;
    params["data"] = selector;

    j["params"] = json::array({params, "latest"});

    std::string buffer = http::post_request(form_url(), j.dump());
    json res = json::parse(buffer);

    if (res.contains("error"))
      return "";
    return res.at("result").get<std::string>();
  };
  Asset new_asset{};
  // decimals
  std::string decimals_hex = call("0x313ce567");
  if (decimals_hex.empty())
    return std::nullopt;
  new_asset.decimals = std::stoul(decimals_hex, nullptr, 16);
  // symbol
  std::string symbol_hex = call("0x95d89b41");
  std::string decoded_symbol = tech_utils::decode_abi_string(symbol_hex);
  if (!decoded_symbol.empty())
    new_asset.symbol = decoded_symbol;

  // name

  std::string name_hex = call("0x06fdde03");
  std::string decoded_name = tech_utils::decode_abi_string(name_hex);
  if (!decoded_name.empty())
    new_asset.name = decoded_name;
  new_asset.contract_address = contract_addr;
  new_asset.chain_id = get_chain_id();
  new_asset.is_native = false;
  new_asset.id =
      fetch_coingecko_id(new_asset.chain_id, new_asset.contract_address);
  return new_asset;
}

std::string
TokensMetadata::fetch_coingecko_id(uint64_t chain_id,
                                   const std::string &contract_addr) const {
  try {
    std::string chain_name = networks::get_coingecko_platform_id(chain_id);
    std::string url = "https://api.coingecko.com/api/v3/coins/" + chain_name +
                      "/contract/" + contract_addr;

    std::string buffer = http::get_request(url);

    json j = json::parse(buffer);
    return j["id"].get<std::string>();
  } catch (const std::exception &err) {
    return "";
  }
}
