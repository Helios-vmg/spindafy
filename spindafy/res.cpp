#include "res.hpp"
#include <cassert>
#include <string>

extern const Point spot_offsets[4] = {
	{8, 6},
	{32, 7},
	{14, 24},
	{26, 25},
};

Resources::Resources(const std::filesystem::path &base){
	auto path = base;
	this->base = Image(path / "spinda_base.png");
	this->mask = Image(path / "spinda_mask.png");
	this->mask_simple = Image(path / "spinda_mask_simple.png");
	assert(this->base.width() == this->mask.width());
	assert(this->base.height() == this->mask.height());
	this->load_spots(path);
	this->load_masks(path);

	int x0 = -1;
	int y0 = -1;
	int x1 = -1;
	int y1 = -1;

	for (int y = 0; y < this->mask.height(); y++){
		for (int x = 0; x < this->mask.width(); x++){
			auto p = this->mask.get(x, y);
			if (!p.a)
				continue;
#define CHECK(d, n, op) if (d##n < 0 || d op d##n) d##n = d
			CHECK(x, 0, <);
			CHECK(x, 1, >);
			CHECK(y, 0, <);
			CHECK(y, 1, >);
		}
	}
	this->mask_bounding_box_corner = { x0, y0 };
	this->mask_bounding_box_size = Point(x1 + 1, y1 + 1) - this->mask_bounding_box_corner;
}

void Resources::load_spots(const std::filesystem::path &path){
	for (int i = 0; i < spot_count; i++){
		auto &inv = this->spots_inverted[i];
		inv = this->spots[i] = path / ("spot_" + std::to_string(i + 1) + ".png");
		for (int y = 0; y < inv.height(); y++){
			for (int x = 0; x < inv.width(); x++){
				auto pixel = inv.get(x, y);
				pixel.r ^= 0xFF;
				pixel.g ^= 0xFF;
				pixel.b ^= 0xFF;
				inv.set(x, y) = pixel;
			}
		}
	}
}

void Resources::load_masks(const std::filesystem::path &path){
	for (int i = 0; i < 4; i++)
		this->masks_simple[i] = path / ("spinda_mask_simple_" + std::to_string(i) + ".png");
}
