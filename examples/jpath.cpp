#include <vix/json/json.hpp>
#include <iostream>

int main()
{
    using namespace Vix::json;

    Json j = o();
    jset(j, "user.langs[2]", "cpp"); // array [null, null, "cpp"]
    jset(j, "user.profile.name", "Gaspard");
    if (auto v = jget(j, "user.langs[2]"))
    {
        std::cout << v->get<std::string>() << "\n"; // cpp
    }
    std::cout << dumps(j, 2) << "\n";
}