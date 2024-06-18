#TO fix all the code
clang-tidy  -fix-errors -checks=*  main.cpp -- -std=c++11

#how to build \
module load bazel \
bazel build //:GetFormat

#how to load clang-format-16 \
module load llvm/16.0.0

#how to compile commands
bazel run @hedron_compile_commands//:refresh_all --
