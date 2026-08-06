# osdk/main/compiler/CMakeLists.txt

add_executable(compiler
    sources/dag.c
    sources/decl.c
    sources/enode.c
    sources/error.c
    sources/expr.c
    sources/gen.c
    sources/init.c
    sources/input.c
    sources/lex.c
    sources/main.c
    sources/output.c
    sources/profio.c
    sources/simp.c
    sources/stmt.c
    sources/string.c
    sources/sym.c
    sources/tree.c
    sources/types.c
)

target_include_directories(compiler PRIVATE includes)

install(TARGETS compiler DESTINATION bin)
