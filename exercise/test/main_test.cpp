#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include <string>

#include "exchange.hpp"

using json = nlohmann::json;

TEST(SampleTest, Addition) {
    EXPECT_EQ(2 + 2, 4);
}

TEST(SampleTest, CallsSrcDummy) {
    EXPECT_EQ(dummy(), 42);
}

TEST(OrderMessage, ParsesNewOrder) {
    auto message = json::parse(
        R"({"New":{"id":1,"symbol":"AAPL","side":"buy","price":100,"quantity":5}})");

    ASSERT_TRUE(message.contains("New"));
    const auto& order = message["New"];
    EXPECT_EQ(order["id"], 1);
    EXPECT_EQ(order["symbol"].get<std::string>(), "AAPL");
    EXPECT_EQ(order["side"].get<std::string>(), "buy");
    EXPECT_EQ(order["price"], 100);
    EXPECT_EQ(order["quantity"], 5);
}
