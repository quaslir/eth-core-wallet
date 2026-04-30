#include "api/json.hpp"
#include "utils/tech_utils.hpp"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>
#include <iostream>
using namespace nlohmann;



namespace core_test_utils {
    struct MnemTests {
        bytes_data entropy;
        std::string seed_phrase;
        bytes_data seed;
        bytes_data master_node;
        std::string first_eth_addr;
        bytes_data first_eth_private_key;
    };


    inline std::vector<MnemTests> get_vector_tests(void) {
        std::ifstream file("/Users/karl/Desktop/Coding/C++/eth-core-wallet/build/bip39-tests.json");
        if(!file.is_open()) {
            return std::vector<MnemTests>();
        }

        json j;

        file >> j;

        try {
           std::vector<MnemTests> vector_tests;

           if(j.contains("english") && j["english"].is_array()) {

               for(const auto& item : j["english"]) {
                   MnemTests test;

                   test.entropy = tech_utils::from_hex_to_bytes(item.at("entropy").get<std::string>());
                   test.seed_phrase = item["seed_phrase"].get<std::string>();
                   test.seed = tech_utils::from_hex_to_bytes(item["seed"].get<std::string>());
                   //test.master_node = tech_utils::from_hex_to_bytes(item["root_key"].get<std::string>());

                   test.first_eth_addr = item["first_eth_addr"].get<std::string>();
                   test.first_eth_private_key = tech_utils::from_hex_to_bytes(item["first_eth_private_key"].get<std::string>());

                   vector_tests.push_back(std::move(test));
               }


           }
            return vector_tests;
        } catch(const std::exception& err) {
            std::cerr << err.what();
             return std::vector<MnemTests>();
        }
    }
}
