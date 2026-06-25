#include "drivers/rpc_bridge.hpp"
#include "api/http.hpp"
#include "fmt/core.h"
#include <chrono>
#include <cstdint>
#include <exception>
#include <httplib.h>
#include <mutex>
#include <string>


void RpcBridge::start(int port) {
    server.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
                {"Access-Control-Allow-Methods", "POST, OPTIONS"},
                {"Access-Control-Allow-Headers", "Content-Type"}
    });
    server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    server.Post("/", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            std::string origin = req.get_header_value("Origin");
            json body = json::parse(req.body);
            json response = handle_rpc(body, origin);
            res.set_content(response.dump(), "application/json");

        }

        catch(const std::exception& err) {
            json error;
            error["jsonrpc"] = "2.0";
            error["error"] = {
                {
                    "code", -32700
                },
                {
                    "message", err.what()
                }
            };

            res.set_content(error.dump(), "application/json");
        }
    });

    running = true;
    server_th = std::thread([this, port] {
        server.listen("127.0.0.1", port);
    });
}


void RpcBridge::stop(void) {
    if(running) {
        server.stop();
        if(server_th.joinable()) server_th.join();
        running = false;
    }
}

void RpcBridge::resolve(uint64_t id, json result) {
    std::lock_guard<std::mutex> lock(pending_mutex);
    auto it = pending.find(id);
    if(it != pending.end()) {
        it->second->result = std::move(result);
        pending_cv.notify_all();
    }
}

void RpcBridge::reject(uint64_t id, const std::string& error) {
    std::lock_guard<std::mutex> lock(pending_mutex);
    auto it = pending.find(id);
    if(it != pending.end()) {
        it->second->error = error;
        pending_cv.notify_all();
    }
}

std::shared_ptr<DappRequest> RpcBridge::submit_and_wait(DappRequestType type, const json& params, const std::string& origin) {
    auto req = std::make_shared<DappRequest>();
    req->id = next_id++;
    req->type = type;
    req->params = params;
    req->origin = origin;

    {
        std::lock_guard<std::mutex> lock(pending_mutex);
        pending[req->id] = req;
    }

    if(on_new_request) on_new_request(req);

    std::unique_lock<std::mutex> lock(pending_mutex);
    pending_cv.wait_for(lock, std::chrono::minutes(2), [&req] {
        return !req->result.is_null() || !req->error.empty();
    });

    pending.erase(req->id);
    return req;
}


json RpcBridge::handle_rpc(const json& body, const std::string& origin) {
    std::string method = body.value("method", "");
    json params = body.value("params", json::array());

    json res;
    res["jsonrpc"] = "2.0";
    res["id"] = body.value("id", 1);


    if(method == "eth_chainId") {
        res["result"] = fmt::format("0x{:x}", get_chain_id());
        return res;
    }
    else if(method == "net_version") {
        res["result"] = std::to_string(get_chain_id());
        return res;
    }
    else if(method == "eth_requestAccounts") {
        auto req = submit_and_wait(DappRequestType::RequestAccounts, params, origin);
        if(!req->error.empty()) {
            res["error"] = {
        {
            "code", 4401
        },
        {
            "message", req->error
        }
            };
            return res;
        }

        res["result"] = json::array({std::string{get_current_address()}});
        return res;
    }

    else if(method == "eth_accounts") {
        res["result"] = json::array({std::string{get_current_address()}});
        return res;
    }

    else if(method == "eth_requestAccounts") {
        auto req = submit_and_wait(DappRequestType::SendTransaction, params, origin);
        if(!req->error.empty()) {
            res["error"] = {
        {
            "code", 4001
        },
        {
            "message", req->error
        }
            };
            return res;
        }
        res["result"] = req->result;
        return res;
    }

    else if(method == "personal_sign" || method == "eth_signTypedData_v4") {
        auto type = method == "personal_sign" ? DappRequestType::PersonalSign : DappRequestType::SignTypedData;
        auto req = submit_and_wait(type, params, origin);
        if(!req->error.empty()) {
            res["error"] = {
        {
            "code", 4001
        },
        {
            "message", req->error
        }
            };
            return res;
        }
        res["result"] = req->result;
        return res;
    }

    std::string raw = http::post_request(form_url(), body.dump());
    return json::parse(raw);
}


bool RpcBridge::is_running(void) const {
    return running;
}
