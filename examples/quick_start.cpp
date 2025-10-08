#include <vix/json/json.hpp>
#include <iostream>

int main()
{
    using namespace Vix::json;

    auto j = o(
        "message", "Hello",
        "count", 3,
        "arr", a(1, 2, 3));

    std::cout << dumps(j, 2) << "\n";
}
