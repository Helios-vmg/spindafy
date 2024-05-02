#pragma once

#include "types.hpp"
#include "point.hpp"
#include <vector>
#include <filesystem>
#include <tuple>

class pixel{
public:
	u8 r = 0, g = 0, b = 0, a = 0;
	pixel() = default;
	pixel(u8 r, u8 g, u8 b, u8 a = 0xFF): r(r), g(g), b(b), a(a){}
	float compute_luma() const;
	u8 compute_integer_luma() const;
	pixel simmetric_difference(const pixel &other) const;
};

class Image{
	int w = 0, h = 0;
	std::vector<pixel> data;
public:
	Image() = default;
	Image(const std::filesystem::path &);
	Image(const Point &p): Image(p.x, p.y){}
	Image(int w, int h);
	Image(const Image &) = default;
	Image &operator=(const Image &) = default;
	Image(Image &&) = default;
	Image &operator=(Image &&) = default;
	static Image allocate_same_size(const Image &);
	pixel get(int x, int y) const;
	pixel &set(int x, int y);
	void fill(pixel);
	void fill_alpha(u8);
	void blit(const Image &, Point = {});
	void pseudo_alpha_blend(const Image &, Point = {});
	void multiply_alpha_from(const Image &);
	auto width() const{
		return this->w;
	}
	auto height() const{
		return this->h;
	}
	std::tuple<int, int> size() const{
		return { this->w, this->h };
	}
	void save_png(const std::filesystem::path &) const;
	std::tuple<std::vector<float>, int, int> get_luma() const;
};
