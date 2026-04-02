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
    bytes_data master_node;
    long long index = 0;

    int get_number_of_bits(void) const;
    public:
    bytes_data prepare_mnemonic(int strength);
    void finalize_from_mnemonic(std::vector<uint8_t> &mnemonic);
    bytes_data get_eth_address(void) const;
    bytes_data get_private_key(void) const;
    long long getIndex(void) const;
};