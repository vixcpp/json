#include <vix/json/json.hpp>
#include <iostream>

int main()
{
    using namespace Vix::json;

    auto j = loads(R"({"a":1,"b":[10,20]})");
    dump_file("out.json", j, 2); // écriture atomique
    auto j2 = load_file("out.json");
    std::cout << dumps(j2, 2) << "\n";
}
