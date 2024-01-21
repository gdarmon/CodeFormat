// if / else / else if
// for / while / do while
// switch / case / default
// range based for loop
// empty loop
// ctor/dtor then the rest of the code
// public/protected/private members - in this order
// template <typename T> class/ template function
// globals
// nested namespaces
// nested classes
// enum class
// virtual / private/public/protected/static/const functions
// include different types of headers
// private/public/protected/static/const/volatile/atomic members
// class inheritance
// initialization list
// macros

#include "main.h"
#include <atomic>
#include <cmath>
#include <iostream>
#include <vector>

namespace MyNamespace
{

enum OLD_ENUM
{
  FIRST_VALUE,
  SECOND_VALUE,
  THIRD_VALUE
};

enum class MY_ENUM_CLASS
{
  VALUE1,
  VALUE2,
  VALUE3
};

#define MAX_VALUE 100

class BaseClass
{
public:
  BaseClass()
  {
    std::cout << "BaseClass constructor" << std::endl;
  }

  virtual ~BaseClass()
  {
    std::cout << "BaseClass destructor" << std::endl;
  }

  virtual void virtualFunction() const
  {
    std::cout << "BaseClass virtualFunction()" << std::endl;
  }

  void publicFunction()
  {
    std::cout << "BaseClass publicFunction()" << std::endl;
    privateFunction();
    protectedFunction();
  }

  static const int _staticConstMember = 100;
  std::atomic<int> atomicMember = ATOMIC_VAR_INIT(0);

protected:
  static void protectedFunction()
  {
    std::cout << "BaseClass protectedFunction()" << std::endl;
  }

private:
  static void privateFunction()
  {
    std::cout << "BaseClass privateFunction()" << std::endl;
  }
};

class DerivedClass : public BaseClass
{
public:
  DerivedClass()
  {
    std::cout << "DerivedClass constructor" << std::endl;
  }

  ~DerivedClass() override
  {
    std::cout << "DerivedClass destructor" << std::endl;
  }

  void virtualFunction() const override
  {
    std::cout << "DerivedClass overrideFunction()" << std::endl;
  }

  static void staticFunction()
  {
    std::cout << "DerivedClass staticFunction()" << std::endl;
  }

  void useBaseClassFunctionalities()
  {
    virtualFunction();
    publicFunction();
    protectedFunction();
  }

  volatile double volatileMember = 2.71;

protected:
  float protectedMember = 3.14F;

private:
  int _privateMember = 10;
};

void printValues(const std::vector<int>& values)
{
  for (auto val : values)
  {
    if (val % 2 == 0)
    {
      std::cout << "Even value: " << val << std::endl;
    }
    else if (val % 5 == 0)
    {
      std::cout << "Value divisible by 5: " << val << std::endl;
    }
    else
    {
      std::cout << "Odd value: " << val << std::endl;
    }
  }
}

void performWhileLoop(int count)
{
  int i = 0;
  while (i < count)
  {
    std::cout << "While loop iteration: " << i + 1 << std::endl;
    ++i;
  }
}

void performDoWhileLoop(int count)
{
  int i = 0;
  do
  {
    std::cout << "Do-while loop iteration: " << i + 1 << std::endl;
    ++i;
  }
  while (i < count);
}

void performSwitchCase(int value)
{
  switch (value)
  {
  case 1:
    std::cout << "Switch case: Value is 1" << std::endl;
    break;
  case 2:
    std::cout << "Switch case: Value is 2" << std::endl;
    break;
  case 3:
    std::cout << "Switch case: Value is 3" << std::endl;
    break;
  default:
    std::cout << "Switch case: Value is not 1, 2, or 3" << std::endl;
    break;
  }
}

void emptyLoopExample(int count)
{
  for (;;)
  {
    // This is an empty for loop with no initialization, condition, or increment sections
    if (count <= 0)
    {
      break;
    }
    --count;
  }
}

void emptyLoop()
{
  // This is an empty loop with no statements inside its body
}
} // namespace MyNamespace

auto main() -> int
{
  double squareRoot = std::sqrt(25.0);
  std::cout << "Square root of 25: " << squareRoot << std::endl;

  MyNamespace::printValues({ 1, 2, 3, 4, 5, 10, 15 });

  std::cout << "MAX_VALUE: " << MAX_VALUE << std::endl;

  MyNamespace::DerivedClass derivedObj;
  derivedObj.virtualFunction();
  derivedObj.useBaseClassFunctionalities();
  MyNamespace::DerivedClass::staticFunction();

  MyNamespace::performWhileLoop(5);
  MyNamespace::performDoWhileLoop(3);

  MyNamespace::performSwitchCase(2);

  MyNamespace::emptyLoopExample(3); // Using the empty for loop

  MyNamespace::emptyLoop(); // Using the empty loop

  return 0;
}
