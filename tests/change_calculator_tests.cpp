#include <gtest/gtest.h>

#include "change_calculator.hpp"

TEST(ChangeCalculatorTests, ReturnsEmptyChangeForZeroAmount) {
    const CoinInventory inventory({{200, 3}, {100, 2}});

    const auto result = ChangeCalculator::computeMinimalCoinChange(0, inventory);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->total, 0);
    EXPECT_TRUE(result->coins.empty());
}

TEST(ChangeCalculatorTests, RejectsNegativeChangeAmount) {
    const CoinInventory inventory({{200, 3}, {100, 2}});

    const auto result = ChangeCalculator::computeMinimalCoinChange(-1, inventory);

    EXPECT_FALSE(result.has_value());
}

TEST(ChangeCalculatorTests, UsesMinimalNumberOfCoins) {
    const CoinInventory inventory({{100, 5}, {50, 5}, {20, 5}, {10, 5}});

    const auto result = ChangeCalculator::computeMinimalCoinChange(200, inventory);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->total, 200);
    ASSERT_EQ(result->coins.size(), 1U);
    EXPECT_EQ(result->coins.at(100), 2);
}

TEST(ChangeCalculatorTests, RespectsLimitedCoinCounts) {
    const CoinInventory inventory({{100, 1}, {50, 10}});

    const auto result = ChangeCalculator::computeMinimalCoinChange(200, inventory);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->total, 200);
    EXPECT_EQ(result->coins.at(100), 1);
    EXPECT_EQ(result->coins.at(50), 2);
}

TEST(ChangeCalculatorTests, ReturnsNulloptWhenExactChangeCannotBeMade) {
    const CoinInventory inventory({{200, 10}});

    const auto result = ChangeCalculator::computeMinimalCoinChange(220, inventory);

    EXPECT_FALSE(result.has_value());
}