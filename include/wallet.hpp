#include "utils.hpp"
#include "security.hpp"
#include "mnemonic.hpp"
#include "derive.hpp"
#include <string>
using bytes_data = std::vector<uint8_t>;
class Wallet {
    private:
    bytes_data priv_key;
    bytes_data eth_address;

    public:
    void generate(int strength = 128);
    bytes_data get_eth_address(void) const;
    bytes_data get_private_key(void) const;
};