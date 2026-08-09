#include "order_book.h"

void mkt_buy(order_book &book, double &qty, std::vector<fill> &fills) {
  while (!book.asks.empty() && qty > 0) {
    auto entry{book.asks.begin()};
    auto &queue{entry->second};
    while (!queue.empty() && qty > 0) {
      order &o{queue.front()};
      double take{std::min(qty, o.qty)};
      fills.push_back(fill{o.price, take});
      o.qty -= take;
      qty -= take;
      if (o.qty <= 0)
        queue.pop_front();
    }
    if (queue.empty())
      book.asks.erase(entry);
  }
}

void mkt_sell(order_book &book, double &qty, std::vector<fill> &fills) {
  while (!book.bids.empty() && qty > 0) {
    auto entry{book.bids.begin()};
    auto &queue{entry->second};
    while (!queue.empty() && qty > 0) {
      order &o{queue.front()};
      double take{std::min(qty, o.qty)};
      fills.push_back(fill{o.price, take});
      o.qty -= take;
      qty -= take;
      if (o.qty <= 0)
        queue.pop_front();
    }
    if (queue.empty())
      book.bids.erase(entry);
  }
}

void limit_buy(order_book &book, double &qty, std::vector<fill> &fills,
               double price) {
  while (!book.asks.empty() && qty > 0) {
    auto entry{book.asks.begin()};
    if (entry->first > price)
      break;
    auto &queue{entry->second};
    while (!queue.empty() && qty > 0) {
      order &o{queue.front()};
      double take{std::min(qty, o.qty)};
      fills.push_back(fill{o.price, take});
      o.qty -= take;
      qty -= take;
      if (o.qty <= 0)
        queue.pop_front();
    }
    if (queue.empty())
      book.asks.erase(entry);
  }
  if (qty > 0)
    book.bids[price].push_back(order{BUY, price, qty, book.next_seq++, true});
}

void limit_sell(order_book &book, double &qty, std::vector<fill> &fills,
                double price) {
  while (!book.bids.empty() && qty > 0) {
    auto entry{book.bids.begin()};
    if (entry->first < price)
      break;
    auto &queue{entry->second};
    while (!queue.empty() && qty > 0) {
      order &o{queue.front()};
      double take{std::min(qty, o.qty)};
      fills.push_back(fill{o.price, take});
      o.qty -= take;
      qty -= take;
      if (o.qty <= 0)
        queue.pop_front();
    }
    if (queue.empty())
      book.bids.erase(entry);
  }
  if (qty > 0)
    book.asks[price].push_back(order{SELL, price, qty, book.next_seq++, true});
}

std::vector<fill> submit_market(order_book &book, side type, double qty) {
  std::vector<fill> fills{};

  if (qty > 0 && type == BUY)
    mkt_buy(book, qty, fills);
  else if (qty > 0 && type == SELL)
    mkt_sell(book, qty, fills);

  return fills;
}

std::vector<fill> submit_limit(order_book &book, side type, double price,
                               double qty) {
  std::vector<fill> fills{};

  if (qty > 0 && type == BUY)
    limit_buy(book, qty, fills, price);
  else if (qty > 0 && type == SELL)
    limit_sell(book, qty, fills, price);

  return fills;
}

std::vector<order> collect_mine(order_book &book) {
  std::vector<order> mine{};
  for (auto &[price, queue] : book.bids)
    for (auto &o : queue)
      if (o.placed)
        mine.push_back(o);
  for (auto &[price, queue] : book.asks)
    for (auto &o : queue)
      if (o.placed)
        mine.push_back(o);
  return mine;
}

void seed_from_snapshot(order_book &book, std::list<level> &bidLevels,
                        std::list<level> &askLevels,
                        std::vector<order> &my_open_orders) {
  book.bids.clear();
  book.asks.clear();

  for (const auto &l : bidLevels)
    book.bids[l.price].push_back(order{BUY, l.price, l.quantity, 0, false});
  for (const auto &l : askLevels)
    book.asks[l.price].push_back(order{SELL, l.price, l.quantity, 0, false});

  for (const auto &o : my_open_orders) {
    if (o.type == BUY)
      book.bids[o.price].push_back(o);
    else
      book.asks[o.price].push_back(o);
  }
}
