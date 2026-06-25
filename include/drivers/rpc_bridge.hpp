#pragma once
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <httplib.h>
#include <memory>
#include <thread>
#include <unordered_map>
#include "api/json.hpp"
#include "core/secure_bytes_data.hpp"
enum class DappRequestType {
    SendTransaction,
    PersonalSign,
    SignTypedData,
    RequestAccounts
};


struct DappRequest {
    uint64_t id;
    DappRequestType type;
    json params;
    std::string origin;
    json result;
    std::string error;
};



class RpcBridge {
    public :
        std::function<std::string(void)> form_url;
        std::function<secure_string(void)> get_current_address;
        std::function<uint32_t(void)> get_chain_id;


        std::function<void(std::shared_ptr<DappRequest>)> on_new_request;

        void start(int port = 8989);
        void stop(void);
        bool is_running(void) const;

        void resolve(uint64_t id, json result);
        void reject(uint64_t id, const std::string& error);


    private:
        httplib::Server server;
        std::thread server_th;
        std::atomic<bool> running{false};
        std::atomic<uint64_t>next_id{1};
        std::mutex pending_mutex;
        std::condition_variable pending_cv;
        std::unordered_map<uint64_t, std::shared_ptr<DappRequest>> pending;


        json handle_rpc(const json& body, const std::string& origin);
        std::shared_ptr<DappRequest> submit_and_wait(DappRequestType type, const json& params, const std::string& origin);
};
