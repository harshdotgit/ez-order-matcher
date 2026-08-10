#pragma once

#include "order_book.h"
#include <string>
#include <vector>

void init_screen();
void reset_screen();
void draw(order_book &book, const std::vector<std::string> &fills_log);
