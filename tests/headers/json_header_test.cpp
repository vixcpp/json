#include <vix/json/json.hpp>

#if defined(VIX_JSON_ENABLE_LITERALS) && (VIX_JSON_ENABLE_LITERALS == 1)
using namespace vix::json::literals;
#endif

int main()
{
#if defined(VIX_JSON_ENABLE_LITERALS) && (VIX_JSON_ENABLE_LITERALS == 1)
  const auto value = R"({"header":"json"})"_json;
  return value.at("header") == "json" ? 0 : 1;
#else
  return 0;
#endif
}
