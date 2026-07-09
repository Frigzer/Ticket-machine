#include <gtest/gtest.h>

#include "server_seed_data.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

std::filesystem::path writeTempSeedFile(const std::string& file_name, const std::string& contents) {
    const auto path = std::filesystem::temp_directory_path() / file_name;
    std::ofstream output(path);
    output << contents;
    return path;
}

}  // namespace

TEST(ServerSeedDataTests, LoadsValidSeedFile) {
    const auto path = writeTempSeedFile(
        "ticket_machine_valid_seed.json",
        R"json({
            "tickets": [
                { "id": 1, "price": 350, "type": "normal", "status": "available" },
                { "id": 2, "price": 170, "type": "reduced", "status": "sold", "owner": { "first_name": "Jan", "last_name": "Kowalski" } }
            ],
            "cashbox": [
                { "denomination": 200, "count": 2 },
                { "denomination": 100, "count": 1 }
            ]
        })json");

    const auto seed = ServerSeedDataLoader::loadFromFile(path);

    ASSERT_EQ(seed.tickets.size(), 2U);
    EXPECT_EQ(seed.tickets[0].status, TicketStatus::Available);
    EXPECT_EQ(seed.tickets[1].status, TicketStatus::Sold);
    ASSERT_TRUE(seed.tickets[1].owner.has_value());
    EXPECT_EQ(seed.tickets[1].owner->last_name, "Kowalski");
    EXPECT_EQ(seed.cashbox.total(), 500);
}

TEST(ServerSeedDataTests, RejectsDuplicateTicketIds) {
    const auto path = writeTempSeedFile(
        "ticket_machine_duplicate_ids_seed.json",
        R"json({
            "tickets": [
                { "id": 1, "price": 350, "type": "normal" },
                { "id": 1, "price": 170, "type": "reduced" }
            ],
            "cashbox": []
        })json");

    EXPECT_THROW(ServerSeedDataLoader::loadFromFile(path), std::runtime_error);
}