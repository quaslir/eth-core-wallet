#pragma once
#include "config/configuration.hpp"
#include "drivers/manager.hpp"
#include <future>
#include <string>
enum class TxStatus { PENDING, SUCCESS, FAILED, ERROR };

class TxStatusManager : public Manager {
private:
  std::future<std::pair<TxStatus, bool>> worker;
  std::pair<TxStatus, bool> current_tx_status;
  std::string current_tx_hash{};

  std::pair<TxStatus, bool> make_request(void);

public:
  TxStatusManager() : Manager(TX_STATUS_UPDATE_TIMEOUT) {}
  ~TxStatusManager();
  std::function<std::string(void)> form_url;

  void request(const secure_string &) override;
  void update(void) override;
  std::pair<TxStatus, bool> get_current_tx_status(void) const;
  void set_tx_hash(const std::string &tx_hash);
};
