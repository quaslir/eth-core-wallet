#include "drivers/tokens_metadata_client.hpp"
#include "api/http.hpp"
#include <exception>
#include <string>



    int TokensMetadata::fetch_decimals(const std::string& contract_addr) {
        try {
            json params;
            params["to"] = contract_addr;
            params["data"] = "0x313ce567";
            json j;
            j["jsonrpc"] = "2.0";
            j["method"] = "eth_call";
            j["params"] = json::array({params});
            j["id"] = 1;

            std::string buffer = http::post_request(form_url(), j.dump());
            json res = json::parse(buffer);
            if(res.contains("result")) {
                return std::stoull(res.at("result").get<std::string>(), nullptr, 16);
            }

            return -1;
        } catch(const std::exception& err) {
            return -1;
        }
    }
