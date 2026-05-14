#include <map>
#include <string>
#include <vector>
namespace price_manager {
std::map<std::string, double>
request_prices(const std::vector<std::string> &symbols);
}
