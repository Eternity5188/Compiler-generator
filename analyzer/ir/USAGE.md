# IR Module Usage

## 0) Preferred Input Source

Use teammates' already integrated lexical+syntax output as the default input path.

Recommended flow:
- Existing pipeline produces parser AST.
- Convert parser AST through `ir::SyntaxASTAdapter::convert(parser_root)`.
- Generate IR via `ir::IRGenerator`.

## 1) Build IR AST manually or via adapter

You can either:
- Construct `ir::ASTNode` directly.
- Convert parser AST through `ir::SyntaxASTAdapter::convert(parser_root)`.

## 2) Generate IR

```cpp
ir::IRGenerator generator;
const std::vector<ir::Quadruple>& quads = generator.generate(ir_root.get());
```

## 3) Check semantic errors

```cpp
if (generator.errors().has_error()) {
    generator.errors().print_all(std::cerr);
}
```

## Notes

- This folder is intentionally independent from `syntax_parser.h` to avoid coupling.
- Upstream lexical/syntax outputs are treated as source of truth; `SyntaxASTAdapter` preserves upstream node type in `source_type` and only does minimal normalization for IR categories.
