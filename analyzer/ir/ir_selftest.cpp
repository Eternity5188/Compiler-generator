#include "ir_generator.h"

#include <iostream>
#include <memory>

namespace {

std::unique_ptr<ir::ASTNode> build_demo_ast() {
    std::unique_ptr<ir::ASTNode> program(new ir::ASTNode(ir::ASTNodeType::PROGRAM));

    // int a;
    std::unique_ptr<ir::ASTNode> decl(new ir::ASTNode(ir::ASTNodeType::DECL, "a"));
    decl->data_type = ir::DataType::INT;

    // a = 1 + 2;
    std::unique_ptr<ir::ASTNode> assign(new ir::ASTNode(ir::ASTNodeType::ASSIGN));
    std::unique_ptr<ir::ASTNode> lhs(new ir::ASTNode(ir::ASTNodeType::IDENT, "a"));

    std::unique_ptr<ir::ASTNode> add(new ir::ASTNode(ir::ASTNodeType::BINOP));
    add->op = ir::OpType::ADD;
    std::unique_ptr<ir::ASTNode> one(new ir::ASTNode(ir::ASTNodeType::INT_LITERAL, "1"));
    one->data_type = ir::DataType::INT;
    std::unique_ptr<ir::ASTNode> two(new ir::ASTNode(ir::ASTNodeType::INT_LITERAL, "2"));
    two->data_type = ir::DataType::INT;

    add->add_child(std::move(one));
    add->add_child(std::move(two));
    assign->add_child(std::move(lhs));
    assign->add_child(std::move(add));

    // return a;
    std::unique_ptr<ir::ASTNode> ret(new ir::ASTNode(ir::ASTNodeType::RETURN_STMT));
    ret->add_child(std::unique_ptr<ir::ASTNode>(new ir::ASTNode(ir::ASTNodeType::IDENT, "a")));

    program->add_child(std::move(decl));
    program->add_child(std::move(assign));
    program->add_child(std::move(ret));

    return program;
}

void print_quads(const std::vector<ir::Quadruple>& quads) {
    for (std::size_t i = 0; i < quads.size(); ++i) {
        const ir::Quadruple& q = quads[i];
        std::cout << i << ": (" << q.op << ", " << q.arg1 << ", " << q.arg2 << ", " << q.result << ")\n";
    }
}

}  // namespace

int main() {
    std::unique_ptr<ir::ASTNode> root = build_demo_ast();

    ir::IRGenerator generator;
    const std::vector<ir::Quadruple>& quads = generator.generate(root.get());

    if (generator.errors().has_error()) {
        generator.errors().print_all(std::cerr);
        return 1;
    }

    print_quads(quads);
    return 0;
}
