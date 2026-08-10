#include "binance.h"
#include "display.h"
#include "ipc.h"
#include "order_book.h"
#include <atomic>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <fcntl.h>
#include <iomanip>
#include <mutex>
#include <poll.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

order_book book;
std::mutex book_mutex;
std::vector<std::string> fills_log;
std::atomic<bool> running{true};
volatile sig_atomic_t g_stop{0};

void on_signal(int) { g_stop = 1; }

void poller_loop() {
  while (running) {
    depth d{fetch_depth("BTCUSDT", 10)};
    {
      std::lock_guard<std::mutex> g(book_mutex);
      std::vector<order> mine{collect_mine(book)};
      seed_from_snapshot(book, d.bids, d.asks, mine);
      draw(book, fills_log);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

std::string summarize(const std::string &desc, double requested,
                      const std::vector<fill> &fills, bool limit) {
  std::ostringstream out{};
  out << desc << std::fixed << std::setprecision(2);
  double filled{0};
  if (fills.empty()) {
    out << " -> none";
  } else {
    out << " ->";
    for (const auto &f : fills) {
      out << " " << f.qty << "@" << f.price;
      filled += f.qty;
    }
  }
  double leftover{requested - filled};
  if (leftover > 0)
    out << " (" << leftover << (limit ? " rested)" : " unfilled)");
  return out.str();
}

void handle_order(const std::string &line) {
  std::istringstream ss{line};
  std::string action, kind;
  ss >> action >> kind;
  if (action != "buy" && action != "sell")
    return;
  side type{action == "buy" ? BUY : SELL};

  std::ostringstream desc{};
  desc << (type == BUY ? "BUY " : "SELL ") << std::fixed << std::setprecision(2);

  std::vector<fill> fills{};
  double qty{0};
  if (kind == "market") {
    ss >> qty;
    desc << "MKT " << qty;
    std::lock_guard<std::mutex> g(book_mutex);
    fills = submit_market(book, type, qty);
    fills_log.push_back(summarize(desc.str(), qty, fills, false));
  } else if (kind == "limit") {
    double price;
    ss >> price >> qty;
    desc << "LIM " << price << " " << qty;
    std::lock_guard<std::mutex> g(book_mutex);
    fills = submit_limit(book, type, price, qty);
    fills_log.push_back(summarize(desc.str(), qty, fills, true));
  } else {
    return;
  }

  std::lock_guard<std::mutex> g(book_mutex);
  if (fills_log.size() > 5)
    fills_log.erase(fills_log.begin());
  draw(book, fills_log);
}

int main() {
  signal(SIGINT, on_signal);
  signal(SIGTERM, on_signal);

  unlink(FIFO_PATH);
  if (mkfifo(FIFO_PATH, 0666) < 0 && errno != EEXIST) {
    std::cerr << "mkfifo failed\n";
    return 1;
  }
  int fd{open(FIFO_PATH, O_RDWR | O_NONBLOCK)};
  if (fd < 0) {
    std::cerr << "open fifo failed\n";
    return 1;
  }

  init_screen();
  std::thread poller(poller_loop);

  std::string acc{};
  char buf[512];
  struct pollfd pfd {
    fd, POLLIN, 0
  };
  while (!g_stop) {
    int r{poll(&pfd, 1, 200)};
    if (r > 0 && (pfd.revents & POLLIN)) {
      ssize_t n{read(fd, buf, sizeof(buf))};
      if (n > 0) {
        acc.append(buf, n);
        size_t pos;
        while ((pos = acc.find('\n')) != std::string::npos) {
          handle_order(acc.substr(0, pos));
          acc.erase(0, pos + 1);
        }
      }
    }
  }

  running = false;
  poller.join();
  close(fd);
  unlink(FIFO_PATH);
  reset_screen();
  return 0;
}
