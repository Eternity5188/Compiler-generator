#pragma once

#include <string>

namespace ir {

struct Quadruple {
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;
    int index = -1;
};

}  // namespace ir
