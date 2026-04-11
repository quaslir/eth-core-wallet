#include "async_manager.hpp"
#include "cli.hpp"
#include "config.hpp"
#include "wallet.hpp"
#include <string>


struct TEMP_DATA {
  bytes_data password_for_wallet_unlocking;
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
  void handle_wallet_creation(void);
  void handle_wallet_import(void);
  void handle_wallet_loading(void);
  void apply_choice_from_welcome_message(int choice);
  void apply_choice_from_wallet_ui(int choice);
  void set_callbacks_for_cli(void);
};
