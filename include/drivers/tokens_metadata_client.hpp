

#include "core/assets.hpp"
#include <cstdint>
#include <functional>
#include <optional>
class TokensMetadata {
public:
  std::function<std::string(void)> form_url;
  std::function<uint64_t(void)> get_chain_id;
  std::optional<Asset>
  fetch_token_metadata_by_contract_addr(const std::string &contract_addr) const;

private:
  std::string fetch_coingecko_id(uint64_t chain_id,
                                 const std::string &contract_addr) const;
};
