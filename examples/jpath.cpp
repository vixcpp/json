#include <vix/json/json.hpp>
#include <iostream>

int main()
{
    using namespace Vix::json;

    Json j = obj();
    jset(j, "user.langs[2]", "cpp"); // -> [null, null, "cpp"]
    jset(j, "user.profile.name", "Gaspard");
    jset(j, R"(user["display.name"])", "Ada L.");

    if (auto v = jget(j, "user.langs[2]"))
    {
        std::cout << v->get<std::string>() << "\n"; // cpp
    }
    std::cout << dumps(j, 2) << "\n";
}
