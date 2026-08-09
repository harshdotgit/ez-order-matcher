#pragma once

#include <iostream>
#include <list>
#include <string.h>

struct level {
  double price, quantity;
};

struct depth {
  std::list<level> bids;
  std::list<level> asks;
};

depth fetch_depth(const std::string &symbol, int limit);
