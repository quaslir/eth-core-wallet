#pragma once
#include <string>
#include <stdint.h>
struct Config {
    bool user_entropy = false;
    int bit_length = 256;
    bool use_passphrase = false;
    std::string derivation_path = "m/44'/60'/0'/0/0";

};