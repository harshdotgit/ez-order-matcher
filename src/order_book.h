#pragma once

#include "binance.h"
#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <vector>

enum side { BUY, SELL };

struct fill {
  double price, qty;
};

struct order {
  side type;
  double price, qty;
  uint64_t seq;
  bool placed;
};

struct order_book {
  // map<price, order(or orders sorted by arrival time - ASC for asks and DESC
  // for bids)>
  std::map<double, std::list<order>, std::greater<double>> bids;
  std::map<double, std::list<order>, std::less<double>> asks;
  uint64_t next_seq = 0;
};

void seed_from_snapshot(order_book &book, std::list<level> &bidLevels,
                        std::list<level> &askLevels,
                        std::vector<order> &my_open_orders);
std::vector<order> collect_mine(order_book &book);
std::vector<fill> submit_market(order_book &book, side type, double qty);
std::vector<fill> submit_limit(order_book &book, side type, double price,
                               double qty);
