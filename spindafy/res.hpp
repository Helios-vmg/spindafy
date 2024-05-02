#pragma once

#include "image.hpp"

const int spot_count = 4;

class Resources{
	void load_spots(const std::filesystem::path &path);
	void load_masks(const std::filesystem::path &path);
public:
	Image base;
	Image mask;
	Image mask_simple;
	Image spots[spot_count];
	Image spots_inverted[spot_count];
	Image masks_simple[4];
	Point mask_bounding_box_corner;
	Point mask_bounding_box_size;
	Resources(const std::filesystem::path &base);
};

extern const Point spot_offsets[4];
