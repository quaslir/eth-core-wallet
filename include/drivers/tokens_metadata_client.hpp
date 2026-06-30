

#include "core/assets.hpp"
#include <functional>
#include <optional>
class TokensMetadata {
        public:
           std::function<std::string(void)> form_url;
         std::optional<Asset> fetch_token_metadata_by_contract_addr(const std::string& contract_addr);
    private:


    int fetch_decimals(const std::string& contract_addr);
};
