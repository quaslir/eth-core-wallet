#include "blockchain_client.hpp"
#include "http.hpp"
#include <exception>
#include <iostream>
std::string BlockchainClient::get_balance(const std::string &eth_addr) const {

  try {
    AlchemyJSON alchm("2.0", "eth_getBalance", {eth_addr, "latest"}, 1);

    std::string data = alchm.to_string();

    std::string buffer = http::post_request(
        "https://eth-sepolia.g.alchemy.com/v2/MkveNSvN4rHOvLoZK8dE3", data);

    if (buffer.empty())
      return "";

    alchm.parse(buffer);

    Uint256 uint256_t(alchm.get_result(), 1);

    return uint256_t.from_wei_to_eth();

  } catch (const std::exception &err) { // handle in the future
    std::cerr << err.what() << std::endl;
    return "";
  }
}
