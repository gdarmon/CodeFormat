#TO fix all the code
clang-tidy  -fix-errors -checks=*  main.cpp -- -std=c++11

#how to build
module load bazel
bazel build //:GetFormat
