#pragma once

#include <ostream>
#include <string>
#include <vector>

namespace ir {

enum class ErrorType {
    UNDECLARED,
    REDEFINED,
    TYPE_MISMATCH,
    NOT_LVALUE,
    PARAM_TYPE_MISMATCH,
    PARAM_COUNT_MISMATCH,
    BREAK_OUTSIDE_LOOP,
    CONTINUE_OUTSIDE_LOOP,
    VOID_VARIABLE,
    VOID_RETURN_MISMATCH,
    VOID_EXPRESSION_VALUE,
};

class SemanticError {
public:
    SemanticError(ErrorType type, std::string message, int line = 0, int col = 0)
        : type_(type), message_(std::move(message)), line_(line), col_(col) {}

    ErrorType type() const { return type_; }
    const std::string& message() const { return message_; }
    int line() const { return line_; }
    int col() const { return col_; }

private:
    ErrorType type_;
    std::string message_;
    int line_;
    int col_;
};

class ErrorHandler {
public:
    void add_error(ErrorType type, const std::string& message, int line = 0, int col = 0) {
        errors_.emplace_back(type, message, line, col);
    }

    bool has_error() const { return !errors_.empty(); }
    const std::vector<SemanticError>& errors() const { return errors_; }

    void clear() { errors_.clear(); }

    void print_all(std::ostream& os) const {
        for (const auto& err : errors_) {
            os << "[SemanticError] line " << err.line() << ", col " << err.col() << ": "
               << err.message() << '\n';
        }
    }

private:
    std::vector<SemanticError> errors_;
};

}  // namespace ir
