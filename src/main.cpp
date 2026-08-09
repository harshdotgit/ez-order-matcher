#include "binance.h"
#include "display.h"
#include "order_book.h"
#include <atomic>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

order_book book;
std::mutex book_mutex;
std::atomic<bool> running{true};

void poller_loop() {
  while (running) {
    depth d{fetch_depth("BTCUSDT", 10)};
    {
      std::lock_guard<std::mutex> g(book_mutex);
      std::vector<order> mine{collect_mine(book)};
      seed_from_snapshot(book, d.bids, d.asks, mine);
      draw(book);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void report(double requested, const std::vector<fill> &fills, bool limit) {
  double filled{0};
  for (size_t i{0}; i < fills.size(); i++) {
    std::cout << (i == 0 ? "filled " : ", ") << std::fixed
              << std::setprecision(2) << fills[i].qty << " @ " << fills[i].price;
    filled += fills[i].qty;
  }
  double leftover{requested - filled};
  if (leftover > 0)
    std::cout << (fills.empty() ? "" : "; ") << std::fixed
              << std::setprecision(2) << leftover
              << (limit ? " rested" : " unfilled");
  std::cout << "\n";
}

int main() {
  init_screen();
  std::thread poller(poller_loop);

  std::string line;
  while (true) {
    {
      std::lock_guard<std::mutex> g(book_mutex);
      std::cout << "> " << std::flush;
    }
    if (!std::getline(std::cin, line))
      break;
    if (line == "quit")
      break;

    std::istringstream ss{line};
    std::string action, kind;
    ss >> action >> kind;
    side type{action == "buy" ? BUY : SELL};

    std::vector<fill> fills{};
    double qty{0};
    bool limit{kind == "limit"};
    if (kind == "market") {
      ss >> qty;
      std::lock_guard<std::mutex> g(book_mutex);
      fills = submit_market(book, type, qty);
    } else if (kind == "limit") {
      double price;
      ss >> price >> qty;
      std::lock_guard<std::mutex> g(book_mutex);
      fills = submit_limit(book, type, price, qty);
    } else {
      continue;
    }

    std::lock_guard<std::mutex> g(book_mutex);
    report(qty, fills, limit);
  }

  running = false;
  poller.join();
  reset_screen();
  return 0;
}
