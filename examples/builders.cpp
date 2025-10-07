#include <vix/json/json.hpp>
#include <iostream>

int main()
{
    using namespace Vix::json;
    auto user = o("id", 42, "name", "Ada", "tags", a("pro", "admin"));
    auto conf = kv({{"host", "localhost"}, {"port", 8080}});
    std::cout << dumps(user) << "\n"
              << dumps(conf) << "'n";
}