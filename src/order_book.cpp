#include "order_book.h"

std::vector<fill> submit_market(order_book book, side type, double qty) {
  std::vector<fill> fills{};
  // TODO: figoure out opposite = (type == Buy) ? book.asks : book.bids
}
