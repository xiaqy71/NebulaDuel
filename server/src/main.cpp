#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

auto main() -> int {
    auto console = spdlog::stdout_color_mt("server");

    spdlog::get("server")->info("Nebula Duel server starting...");

    return 0;
}
