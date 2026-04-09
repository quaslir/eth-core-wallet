#pragma once
#include <future>
#include "blockchain_client.hpp"
#include <string>
class AsyncBalanceManager {
private:

BlockchainClient block_client;
std::future<std::string> worker;
bool updating = false;
std::string current_balance = "0";

public:
void request_balance(const std::string& addr);
void update(void);
std::string get_balance(void) const;
};