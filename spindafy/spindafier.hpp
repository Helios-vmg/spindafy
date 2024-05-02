#pragma once

#include "res.hpp"
#include <optional>
#include <random>

class Spindafier{
	u32 black_cases[4];
	u32 white_case;
	static std::optional<bool> get_color(const Image &);
public:
	Resources res;
	Spindafier(const std::filesystem::path &base);
	Image spindafy(const Image &, std::mt19937 * = nullptr) const;
};
