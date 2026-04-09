#include "async_manager.hpp"
#include "config.hpp"
#include "wallet.hpp"
#include <string>
enum State_t { MAIN_MENU, SEED_GENERATION, SEED_IMPORT, EXIT, WALLET_UI };

class UserInterface {
public:
  void load(void);

private:
  Wallet wallet;
  Config config;
  AsyncBalanceManager balance_manager;
  State_t state = MAIN_MENU;
  void handle_wallet_creation(void);
  void handle_wallet_import(void);
  void handle_wallet_loading(void);
  void apply_choice_from_welcome_message(int choice);
  void apply_choice_from_wallet_ui(int choice);
  bool handle_seed_generation_config(void);
};