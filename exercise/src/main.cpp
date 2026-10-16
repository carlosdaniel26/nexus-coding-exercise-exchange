#include <httplib.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

using json = nlohmann::json;

static bool fetch(httplib::Client& client, int n) {
    auto res = client.Get("/orders?n=" + std::to_string(n));
    if (!res || res->status != 200) {
        std::cerr << "feed unavailable at 127.0.0.1:3000\n";
        return false;
    }

    auto messages = json::parse(res->body);
    std::cout << "-- " << messages.size() << " messages --\n";

    for (const auto& message : messages) {
        if (message.contains("New")) {
            const auto& order = message["New"];
            std::cout << order["id"] << " New " << order["symbol"].get<std::string>() << " "
                      << order["side"].get<std::string>() << " " << order["price"] << " x "
                      << order["quantity"] << "\n";
        } else if (message.contains("Cancel")) {
            const auto& cancel = message["Cancel"];
            std::cout << cancel["id"] << " Cancel -> " << cancel["target_id"] << "\n";
        }
    }
    return true;
}

int main(int argc, char** argv) {
    int n = argc > 1 ? std::atoi(argv[1]) : 10;
    int seconds = argc > 2 ? std::atoi(argv[2]) : 0;
    int interval_ms = argc > 3 ? std::atoi(argv[3]) : 200;

    httplib::Client client("127.0.0.1", 3000);

    if (seconds <= 0) {
        return fetch(client, n) ? 0 : 1;
    }

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
    while (std::chrono::steady_clock::now() < deadline) {
        fetch(client, n);
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
    return 0;
}
