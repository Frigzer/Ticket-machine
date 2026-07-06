#pragma once

#include "coin_inventory.hpp"
#include "models.hpp"

#include <optional>

class ChangeCalculator {
public:
	static std::optional< ChangeResult > computeMinimalCoinChange( Money amount, const CoinInventory& inventory );
};