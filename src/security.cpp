#include "security.hpp"
#include <iomanip>
#include "wallet.hpp"
#include "cli.hpp"
using json = nlohmann::json;
namespace security_manager {
bool first_time_save(const Wallet& wallet, const std::string& filename) {
bytes_data password = cli::read_and_confirm_password();
json j;
j["current_address"] = wallet.get_eth_address();
j["master_node"] = wallet.get_master_node();

std::ofstream file(filename);

if(file.is_open()) {
  file << std::setw(4) << j << std::endl;
  return true;
}

return false;
}

bool load_wallet(Wallet& wallet,const  std::string&filename) {
  std::ifstream file(filename);

  if(!file.is_open()) {
    return false;
  }
  try {
  json j;
  file >> j;

  wallet.set_eth_address(j["current_address"]);
  wallet.set_master_node(j["master_node"]);
  } catch(const std::exception& err) {
    std::cerr << err.what() << std::endl;

  }
  return true;
}
} // namespace security_manager