#include "display.h"
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

static const char *RESET{"\033[0m"};
static const char *RED{"\033[31m"};
static const char *GREEN{"\033[32m"};
static const char *YELLOW{"\033[1;33m"};

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
                    const std::list<order> &queue, const char *base) {
  double qty{level_qty(queue)};
  const char *color{level_mine(queue) ? YELLOW : base};
  out << "\033[" << row << ";1H\033[2K" << color << std::fixed
      << std::setprecision(2) << std::setw(12) << price << std::setw(12) << qty
      << RESET;
}

static void blank_at(std::ostringstream &out, int row) {
  out << "\033[" << row << ";1H\033[2K";
}

void init_screen() { std::cout << "\033[2J\033[H\033[?25l" << std::flush; }

void reset_screen() { std::cout << "\033[?25h\033[2J\033[H" << std::flush; }

void draw(order_book &book, const std::vector<std::string> &fills_log) {
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

  std::ostringstream out{};
  out << "\033[1;1H\033[2K    ORDER BOOK : BTC/USDT ";
  blank_at(out, 2);

  for (int row = 3; row <= 12; row++) {
    int idx{12 - row};
    if (idx < static_cast<int>(asks.size()))
      line_at(out, row, asks[idx].first, *asks[idx].second, RED);
    else
      blank_at(out, row);
  }

  blank_at(out, 13);

  for (int row = 14; row <= 23; row++) {
    int idx{row - 14};
    if (idx < static_cast<int>(bids.size()))
      line_at(out, row, bids[idx].first, *bids[idx].second, GREEN);
    else
      blank_at(out, row);
  }

  blank_at(out, 24);
  out << "\033[25;1H\033[2K         LAST 5 FILLS       ";
  int row{26};
  for (auto it = fills_log.rbegin(); it != fills_log.rend() && row <= 30;
       ++it, ++row)
    out << "\033[" << row << ";1H\033[2K   " << *it;
  for (; row <= 30; row++)
    blank_at(out, row);

  std::cout << out.str() << std::flush;
}
