#include "async_manager.hpp"
#include "cli.hpp"
#include "config.hpp"
#include "json.hpp"
#include "wallet.hpp"

struct TEMP_DATA {
  bytes_data password_for_wallet_unlocking;
  bytes_data mnemonic, passphrase; // for import only
};

class UserInterface {
public:
  void load(void);

private:
  Wallet wallet;
  Config config;
  AsyncBalanceManager balance_manager;
  TEMP_DATA temp;
  CLI cli;
  void apply_choice_from_wallet_ui(int choice);
  void set_callbacks_for_cli(void);
};
