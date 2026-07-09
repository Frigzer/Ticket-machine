#include <gtest/gtest.h>

#include "ticket_server.hpp"

#include <atomic>
#include <barrier>
#include <chrono>
#include <thread>
#include <vector>

TEST(TicketServerConcurrencyTests, OnlyOneClientCanReserveLastTicket) {
    auto now = std::chrono::steady_clock::now();
    TicketServer server(
        {Ticket{.id = 1, .price = 350, .type = "normal", .status = TicketStatus::Available, .owner = std::nullopt}},
        CoinInventory({{200, 1}, {100, 5}, {50, 2}}), std::chrono::seconds(60), [&now] { return now; });

    constexpr int thread_count = 10;
    std::atomic<int> success_count{0};
    std::barrier start_line(thread_count);
    std::vector<std::thread> threads;

    threads.reserve(thread_count);
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&server, &success_count, &start_line] {
            start_line.arrive_and_wait();
            const auto reservation = server.reserveTicket("normal");
            if (reservation.has_value()) {
                ++success_count;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(success_count.load(), 1);

    const auto availability = server.getAvailableTickets();
    ASSERT_EQ(availability.size(), 1U);
    EXPECT_EQ(availability[0].available_count, 0);
}