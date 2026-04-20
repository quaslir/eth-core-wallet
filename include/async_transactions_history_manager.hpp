#pragma once
#include "blockchain_client.hpp"
#include <chrono>
#include <future>
#include <vector>
#define TIMER 1000

class AsyncTransactionsHistoryManager {
    private:
    std::future<std::vector<TransactionRecord>> worker;
    bool updating = false;
    std::chrono::steady_clock::time_point last_update_time;
    std::vector<TransactionRecord> current_transactions_history;

    public:

    AsyncTransactionsHistoryManager() : last_update_time(std::chrono::steady_clock::now() - std::chrono::milliseconds(TIMER)) {}

    void request_transactions_data(const std::string& eth_addr);
    void update(void);
    bool get_status(void);
    std::vector<TransactionRecord> get_transactions_history(void) const;
};
