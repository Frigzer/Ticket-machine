#pragma once

#include "coin_inventory.hpp"
#include "models.hpp"

#include <filesystem>
#include <vector>

struct ServerSeedData {
	std::vector< Ticket > tickets;
	CoinInventory cashbox;
};

class ServerSeedDataLoader {
public:
	[[nodiscard]] static ServerSeedData loadFromFile( const std::filesystem::path& file_path );
};