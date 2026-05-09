#include "drivers/manager.hpp"
#include <chrono>

bool Manager::get_status(void) const {return this->updating;}
bool Manager::get_error(void) const {return this->error;}
bool Manager::can_request(void) const {
    if(updating) return false;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now - last_update_time);
    return elapsed >= timer_interval;
}
