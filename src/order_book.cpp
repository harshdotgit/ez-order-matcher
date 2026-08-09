#include "order_book.h"

void mkt_buy(order_book &book, double &qty, std::vector<fill> &fills) {
  double take;
  auto entry{std::next(book.asks.begin())};

  while (!book.asks.empty() && qty > 0) {
    // its ok because we likely wont have to tread through many asks.
    // iterator is a copy but points to the same map (book.asks)

    while (!entry->second.empty() && qty > 0) {
      take = std::min(qty, entry->second.front().qty);
      fills.push_back(fill{entry->second.front().price, take});
      entry->second.front().qty -= take;
      qty -= take;
      if (entry->second.front().qty <= 0)
        entry->second.pop_front();
    }
    if (entry->second.empty()) {
      entry = book.asks.erase(entry);
    }
  }
}
void mkt_sell(order_book &book, double &qty, std::vector<fill> &fills) {
  double take;
  auto entry{std::next(book.bids.begin())};

  while (!book.asks.empty() && qty > 0) {
    // its ok because we likely wont have to tread through many asks.
    // iterator is a copy but points to the same map (book.asks)

    while (!entry->second.empty() && qty > 0) {
      take = std::min(qty, entry->second.front().qty);
      fills.push_back(fill{entry->second.front().price, take});
      entry->second.front().qty -= take;
      qty -= take;
      if (entry->second.front().qty <= 0)
        entry->second.pop_front();
    }
    if (entry->second.empty()) {
      entry = book.bids.erase(entry);
    }
  }
}

void limit_buy(order_book &book, double &qty, std::vector<fill> &fills,
               double price) {
  double take;
  auto entry{std::next(book.asks.begin())};
  while (!book.asks.empty() && qty > 0) {
    // its ok because we likely wont have to tread through many asks.
    // iterator is a copy but points to the same map (book.asks)
    if (entry->first <= price) {
      while (!entry->second.empty() && qty > 0) {
        take = std::min(qty, entry->second.front().qty);
        fills.push_back(fill{entry->second.front().price, take});
        entry->second.front().qty -= take;
        qty -= take;
        if (entry->second.front().qty <= 0)
          entry->second.pop_front();
      }
      if (entry->second.empty()) {
        entry = book.bids.erase(entry);
      }
    }
  }
  // place leftover qty into bids
}

void limit_sell(order_book &book, double &qty, std::vector<fill> &fills,
                double price) {}

std::vector<fill> submit_market(order_book &book, side type, double qty) {
  // DONE: figoure out opposite = (type == Buy) ? book.asks : book.bids
  // Can just use book.asks / bids at runtime
  //
  std::vector<fill> fills{};

  if (qty > 0 && (type == BUY || type == SELL)) {
    if (type == BUY) {
      mkt_buy(book, qty, fills);
    } else if (type == SELL) {
      mkt_sell(book, qty, fills);
    }
  }

  if (qty)
    std::cout << qty << " ORDERS COULD NOT BE FILLED\n";

  return fills;
}

std::vector<fill> submit_limit(order_book &book, side type, double price,
                               double qty) {
  // DONE: figoure out opposite = (type == Buy) ? book.asks : book.bids
  // Can just use book.asks / bids at runtime
  //
  std::vector<fill> fills{};

  if (qty > 0 && (type == BUY || type == SELL)) {
    if (type == BUY) {
      limit_buy(book, qty, fills, price);
    } else if (type == SELL) {
      limit_sell(book, qty, fills, price);
    }
  }

  if (qty) {
    std::cout << qty << " ORDERS COULD NOT BE FILLED\n"
              << qty << " ORDERS ADDED BACK TO THE BOOK\n";
  }

  return fills;
}

void seed_from_snapshot(order_book &book,
                        std::map<double, std::list<order>>
                            &my_open_orders) // bidLevels, //askLevels)
{
  book.asks.clear();
  book.bids.clear();

  // TODO: push bidLevels into book.bids
  // TODO: push askLevels into book.asks

  size_t my_orders_size{my_open_orders.size()};

  for (const auto &[price, o] : my_open_orders) {
    if (o.front().type == BUY)
      book.bids[price] = o;
    else if (o.front().type == SELL)
      book.asks[price] = o;
  }

  my_open_orders.clear();
}
