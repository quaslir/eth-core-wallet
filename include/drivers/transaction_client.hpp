
#include <cstdint>
#include <functional>
#include <future>
#include <tuple>
#include <utility>
#include "core/secure_bytes_data.hpp"
#include "core/uint256.hpp"


struct RawTx {
    uint64_t nonce;
  uint64_t gas_price;
  uint64_t gas_limit;
  bytes_data to;
  Uint256 value;
  bytes_data data;
  uint64_t v;
  bytes_data r;
  bytes_data s;
  bytes_data private_key;
};


class TransactionManager {
    public:
                static std::optional<uint64_t> get_nonce(const secure_string& eth_addr, const std::string & url);
        std::future<std::string> send(RawTx& params);
        std::function<std::string(void)> form_url;
    private:

        std::tuple<bytes_data, bytes_data, int> sign_transaction(const bytes_data& hash, const bytes_data& key);
        std::string make_request(const bytes_data& data) const;

};
