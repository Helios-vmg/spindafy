#pragma once

#include "point.hpp"
#include "res.hpp"

class SpotConfiguration{
public:
	Point spots[spot_count];
	SpotConfiguration(u32 personality);
	Image full_render(const Resources &res) const;
	Image minimal_render(const Resources &res, size_t mask_index = 3) const;
	void minimal_render2(Image &dst, Image &temp, const Resources &res, size_t mask_index = 3) const;
	static Image merge_all(const Resources &res);
};