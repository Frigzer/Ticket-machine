#include <gtest/gtest.h>

#include "coin_inventory.hpp"

#include <stdexcept>

TEST(CoinInventoryTests, RejectsUnsupportedDenominationInConstructor) {
    EXPECT_THROW(CoinInventory({{25, 1}}), std::invalid_argument);
}

TEST(CoinInventoryTests, RemoveCoinsDoesNotPartiallyModifyInventoryOnFailure) {
    CoinInventory inventory({{100, 1}, {50, 1}});

    const bool removed = inventory.removeCoins({{100, 1}, {50, 2}});

    EXPECT_FALSE(removed);
    EXPECT_EQ(inventory.count(100), 1U);
    EXPECT_EQ(inventory.count(50), 1U);
    EXPECT_EQ(inventory.total(), 150);
}