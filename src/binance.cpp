#include "binance.h"
#include <cpr/cpr.h>
#include <list>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

depth fetch_depth(const std::string &symbol, int limit) {
  std::string url{"https://api.binance.com/api/v3/depth?symbol=" + symbol +
                  "&limit=" + std::to_string(limit)};
  auto response{cpr::Get(cpr::Url{url})};
  if (response.status_code != 200) {
    std::cerr << "BINANCE REQUEST FAILED\n";
    exit(EXIT_FAILURE);
  }
  auto j = json::parse(response.text);
  depth d{};
  for (const auto &lvl : j["bids"])
    d.bids.push_back(level{std::stod(lvl[0].get<std::string>()),
                           std::stod(lvl[1].get<std::string>())});
  for (const auto &lvl : j["asks"])
    d.asks.push_back(level{std::stod(lvl[0].get<std::string>()),
                           std::stod(lvl[1].get<std::string>())});

  return d;
}
