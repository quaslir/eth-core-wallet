#include "security.hpp"

namespace security_manager {
    void save_to_file(bytes_data & data) {
        std::ofstream target_file("data.bin");

        target_file.write(reinterpret_cast<const char *>(data.data()), data.size());

        target_file.close();
    }
}