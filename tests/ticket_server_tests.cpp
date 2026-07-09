#include <gtest/gtest.h>

#include "ticket_server.hpp"

namespace {

using Clock = std::chrono::steady_clock;

std::vector<Ticket> makeSampleTickets() {
    return {
        Ticket{.id = 1, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt},
        Ticket{.id = 2, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt},
        Ticket{.id = 3, .price = 170, .type = "reduced", .status = TicketStatus::Available, .owner = std::nullopt},
    };
}

int availableCountForType(const std::vector<TicketAvailability>& availability, const std::string& type) {
    for (const auto& item : availability) {
        if (item.type == type) {
            return item.available_count;
        }
    }
    return -1;
}

}  // namespace

TEST(TicketServerTests, ReservationLifecycleUpdatesAvailability) {
    auto now = Clock::now();
    TicketServer server(
        makeSampleTickets(), CoinInventory({{200, 1}, {100, 5}, {50, 2}}), std::chrono::seconds(60), [&now] { return now; });

    const auto before = server.getAvailableTickets();
    const auto reservation = server.reserveTicket("normal");
    const auto after_reserve = server.getAvailableTickets();

    ASSERT_TRUE(reservation.has_value());
    EXPECT_EQ(availableCountForType(before, "normal"), 2);
    EXPECT_EQ(availableCountForType(after_reserve, "normal"), 1);

    EXPECT_TRUE(server.cancelReservation(reservation->reservation_id));

    const auto after_cancel = server.getAvailableTickets();
    EXPECT_EQ(availableCountForType(after_cancel, "normal"), 2);
}

TEST(TicketServerTests, TimeoutReleasesReservation) {
    auto now = Clock::now();
    TicketServer server(
        {Ticket{.id = 1, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt}},
        CoinInventory({{200, 1}, {100, 5}, {50, 2}}), std::chrono::seconds(60), [&now] { return now; });

    const auto reservation = server.reserveTicket("normal");
    ASSERT_TRUE(reservation.has_value());

    now += std::chrono::seconds(61);

    const auto availability = server.getAvailableTickets();
    ASSERT_EQ(availability.size(), 1U);
    EXPECT_EQ(availability[0].type, "normal");
    EXPECT_EQ(availability[0].available_count, 1);
}

TEST(TicketServerTests, ExpiredReservationReturnsDedicatedErrorAndRefundsInsertedCoins) {
    auto now = Clock::now();
    TicketServer server(
        {Ticket{.id = 1, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt}},
        CoinInventory({{200, 1}, {100, 5}, {50, 2}}), std::chrono::seconds(60), [&now] { return now; });

    const auto reservation = server.reserveTicket("normal");
    ASSERT_TRUE(reservation.has_value());

    now += std::chrono::seconds(61);

    const CoinInventory inserted({{500, 1}});
    const CustomerData customer{.first_name = "Jan", .last_name = "Kowalski"};
    const auto result = server.finalizePurchase(reservation->reservation_id, customer, inserted);

    ASSERT_TRUE(std::holds_alternative<PurchaseFailure>(result));
    const auto& failure = std::get<PurchaseFailure>(result);
    EXPECT_EQ(failure.error, PurchaseError::ReservationExpired);
    EXPECT_EQ(failure.returned_coins.at(500), 1);
}

TEST(TicketServerTests, SuccessfulPurchaseAssignsTicketAndReturnsChange) {
    auto now = Clock::now();
    TicketServer server(
        {Ticket{.id = 1, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt}},
        CoinInventory({{200, 1}, {100, 5}, {50, 2}}), std::chrono::seconds(60), [&now] { return now; });

    const auto reservation = server.reserveTicket("normal");
    ASSERT_TRUE(reservation.has_value());

    const CoinInventory inserted({{500, 1}});
    const CustomerData customer{.first_name = "Jan", .last_name = "Kowalski"};
    const auto result = server.finalizePurchase(reservation->reservation_id, customer, inserted);

    ASSERT_TRUE(std::holds_alternative<PurchaseSuccess>(result));
    const auto& success = std::get<PurchaseSuccess>(result);

    EXPECT_EQ(success.ticket_id, 1);
    EXPECT_EQ(success.customer.first_name, "Jan");
    EXPECT_EQ(success.customer.last_name, "Kowalski");
    EXPECT_EQ(success.paid, 500);
    EXPECT_EQ(success.price, 350);
    EXPECT_EQ(success.change.total, 150);
    EXPECT_EQ(success.change.coins.at(100), 1);
    EXPECT_EQ(success.change.coins.at(50), 1);

    const auto availability = server.getAvailableTickets();
    ASSERT_EQ(availability.size(), 1U);
    EXPECT_EQ(availability[0].available_count, 0);
}

TEST(TicketServerTests, InsufficientFundsRefundsInsertedCoinsAndReleasesReservation) {
    auto now = Clock::now();
    TicketServer server(
        {Ticket{.id = 1, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt}},
        CoinInventory({{200, 1}, {100, 5}, {50, 2}}), std::chrono::seconds(60), [&now] { return now; });

    const auto reservation = server.reserveTicket("normal");
    ASSERT_TRUE(reservation.has_value());

    const CoinInventory inserted({{200, 1}, {100, 1}});
    const CustomerData customer{.first_name = "Jan", .last_name = "Kowalski"};
    const auto result = server.finalizePurchase(reservation->reservation_id, customer, inserted);

    ASSERT_TRUE(std::holds_alternative<PurchaseFailure>(result));
    const auto& failure = std::get<PurchaseFailure>(result);
    EXPECT_EQ(failure.error, PurchaseError::InsufficientFunds);
    EXPECT_EQ(failure.returned_coins.at(200), 1);
    EXPECT_EQ(failure.returned_coins.at(100), 1);

    const auto availability = server.getAvailableTickets();
    ASSERT_EQ(availability.size(), 1U);
    EXPECT_EQ(availability[0].available_count, 1);
}

TEST(TicketServerTests, CannotMakeChangeRefundsInsertedCoinsAndReleasesReservation) {
    auto now = Clock::now();
    TicketServer server(
        {Ticket{.id = 1, .price = 280, .type = "special", .status = TicketStatus::Available, .owner = std::nullopt}},
        CoinInventory({{200, 10}}), std::chrono::seconds(60), [&now] { return now; });

    const auto reservation = server.reserveTicket("special");
    ASSERT_TRUE(reservation.has_value());

    const CoinInventory inserted({{500, 1}});
    const CustomerData customer{.first_name = "Jan", .last_name = "Kowalski"};
    const auto result = server.finalizePurchase(reservation->reservation_id, customer, inserted);

    ASSERT_TRUE(std::holds_alternative<PurchaseFailure>(result));
    const auto& failure = std::get<PurchaseFailure>(result);
    EXPECT_EQ(failure.error, PurchaseError::CannotMakeChange);
    EXPECT_EQ(failure.returned_coins.at(500), 1);

    const auto availability = server.getAvailableTickets();
    ASSERT_EQ(availability.size(), 1U);
    EXPECT_EQ(availability[0].available_count, 1);
}