#include "display.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <utility>

static const char *RESET{"\033[0m"};
static const char *RED{"\033[31m"};
static const char *GREEN{"\033[32m"};
static const char *YELLOW{"\033[1;33m"};

static const int BOOK_ROWS{23};

static int terminal_rows() {
  struct winsize ws{};
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0)
    return ws.ws_row;
  return 40;
}

static double level_qty(const std::list<order> &queue) {
  double sum{0};
  for (const auto &o : queue)
    sum += o.qty;
  return sum;
}

static bool level_mine(const std::list<order> &queue) {
  for (const auto &o : queue)
    if (o.placed)
      return true;
  return false;
}

static void line_at(std::ostringstream &out, int row, double price,
                    const std::list<order> &queue, double max_qty,
                    const char *base) {
  double qty{level_qty(queue)};
  int n{max_qty > 0 ? static_cast<int>(qty / max_qty * 40) : 0};
  const char *color{level_mine(queue) ? YELLOW : base};
  out << "\033[" << row << ";1H\033[2K" << color << std::fixed
      << std::setprecision(2) << std::setw(12) << price << std::setw(12) << qty
      << "  " << std::string(n, '#') << RESET;
}

static void blank_at(std::ostringstream &out, int row) {
  out << "\033[" << row << ";1H\033[2K";
}

void init_screen() {
  int rows{terminal_rows()};
  std::cout << "\033[2J\033[H";
  if (rows > BOOK_ROWS + 1)
    std::cout << "\033[" << (BOOK_ROWS + 1) << ";" << rows << "r";
  std::cout << "\033[" << (BOOK_ROWS + 1) << ";1H" << std::flush;
}

void reset_screen() {
  std::cout << "\033[r\033[" << (BOOK_ROWS + 1) << ";1H\n" << std::flush;
}

void draw(order_book &book) {
  std::vector<std::pair<double, const std::list<order> *>> asks{}, bids{};
  for (auto &[price, queue] : book.asks) {
    asks.push_back({price, &queue});
    if (asks.size() == 10)
      break;
  }
  for (auto &[price, queue] : book.bids) {
    bids.push_back({price, &queue});
    if (bids.size() == 10)
      break;
  }

  double max_qty{0};
  for (auto &[price, queue] : asks)
    max_qty = std::max(max_qty, level_qty(*queue));
  for (auto &[price, queue] : bids)
    max_qty = std::max(max_qty, level_qty(*queue));

  std::ostringstream out{};
  out << "\033[s";
  out << "\033[1;1H\033[2K        ORDER BOOK  --  BTC/USDT";
  blank_at(out, 2);

  for (int row = 3; row <= 12; row++) {
    int idx{12 - row};
    if (idx < static_cast<int>(asks.size()))
      line_at(out, row, asks[idx].first, *asks[idx].second, max_qty, RED);
    else
      blank_at(out, row);
  }

  blank_at(out, 13);
  if (!asks.empty() && !bids.empty())
    out << "        -- spread " << std::fixed << std::setprecision(2)
        << (asks.front().first - bids.front().first) << " --";

  for (int row = 14; row <= 23; row++) {
    int idx{row - 14};
    if (idx < static_cast<int>(bids.size()))
      line_at(out, row, bids[idx].first, *bids[idx].second, max_qty, GREEN);
    else
      blank_at(out, row);
  }

  out << "\033[u";
  std::cout << out.str() << std::flush;
}
