#include "ui/ui.hpp"
#include "config/configuration.hpp"
int main(void) {
    if(!Configuration::get_instance().init("config.json")) {
        Configuration::get_instance().create_configuration("config.json");
        std::cerr << "[FATAL] Failed to load config.json Exiting." << std::endl;
        return 1;
    }
  UserInterface ui;
  ui.load();
  return 0;
}
