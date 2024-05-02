#include "image.hpp"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include <memory>
#include <fstream>
#include <stdexcept>

typedef std::uint32_t u32;

struct Freer{
	void operator()(stbi_uc *p){
		stbi_image_free(p);
	}
};

Image::Image(int w, int h){
	if (w <= 0 || h <= 0)
		return;
	this->w = w;
	this->h = h;
	this->data.resize(this->w * this->h);
}

float pixel::compute_luma() const{
	float r = this->r / 255.f;
	float g = this->g / 255.f;
	float b = this->b / 255.f;
	float a = this->a / 255.f;

	auto ret = (r * .2126f + g * .7152f + b * .0722f) * a;
	if (ret < 0)
		ret = 0;
	else if (ret > 1)
		ret = 1;
	return ret;
}

u8 pixel::compute_integer_luma() const{
	u32 r = this->r;
	u32 g = this->g;
	u32 b = this->b;
	u32 a = this->a;

	return (u8)((r * 2126 + g * 7152 + b * 722) * a / 2'550'000);
}

u8 simmetric_difference(u8 a, u8 b){
	return a >= b ? a - b : b - a;
}

pixel pixel::simmetric_difference(const pixel &other) const{
	pixel ret;
#define SD(x) ret.x = ::simmetric_difference(this->x, other.x)
	SD(r);
	SD(g);
	SD(b);
	ret.a = 0xFF;
	return ret;
}


Image::Image(const std::filesystem::path &path){
	static_assert(sizeof(pixel) == 4, "???");

	std::vector<u8> buffer;
	{
		std::ifstream file(path, std::ios::binary);
		if (!file)
			throw std::runtime_error("file not found: " + path.u8string());
		file.seekg(0, std::ios::end);
		buffer.resize(file.tellg());
		file.seekg(0);
		file.read((char *)buffer.data(), (int)buffer.size());
	}

	int channels;
	auto image = stbi_load_from_memory(buffer.data(), (int)buffer.size(), &this->w, &this->h, &channels, 4);
	if (!image){
		this->w = 0;
		this->h = 0;
		return;
	}
	std::unique_ptr<stbi_uc, Freer> image_p(image);
	this->data.resize(this->w * this->h);
	memcpy(this->data.data(), image, this->w * this->h * 4);
}

Image Image::allocate_same_size(const Image &src){
	return Image(src.w, src.h);
}

pixel Image::get(int x, int y) const{
	if (x < 0 || x >= this->w || y < 0 || y >= this->h)
		return {};
	return this->data[x + y * this->w];
}

pixel &Image::set(int x, int y){
	return this->data[x + y * this->w];
}

void Image::fill(pixel color){
	std::fill(this->data.begin(), this->data.end(), color);
}

void Image::fill_alpha(u8 color){
	for (auto &p : this->data)
		p.a = color;
}

void Image::blit(const Image &src, Point point){
	for (int y = 0; y < src.h; y++){
		auto Y = point.y + y;
		if (Y < 0 || Y >= this->h)
			continue;
		for (int x = 0; x < src.w; x++){
			auto X = point.x + x;
			if (X < 0 || X >= this->w)
				continue;
			this->set(X, Y) = src.get(x, y);
		}
	}
}

//Assumes alpha channels are either 0x00 or 0xFF.
void Image::pseudo_alpha_blend(const Image &src, Point point){
	for (int y = 0; y < src.h; y++){
		auto Y = point.y + y;
		if (Y < 0 || Y >= this->h)
			continue;
		for (int x = 0; x < src.w; x++){
			auto X = point.x + x;
			if (X < 0 || X >= this->w)
				continue;

			auto over = src.get(x, y);
			if (over.a)
				this->set(X, Y) = over;
		}
	}
}

void Image::multiply_alpha_from(const Image &src){
	if (this->w != src.w || this->h != src.h)
		return;
	for (size_t i = 0; i < this->data.size(); i++)
		this->data[i].a = (u8)((u32)this->data[i].a * (u32)src.data[i].a / 255);
}

void Image::save_png(const std::filesystem::path &path) const{
	stbi_write_png(path.u8string().c_str(), this->w, this->h, 4, this->data.data(), this->w * 4);
}

std::tuple<std::vector<float>, int, int> Image::get_luma() const{
	std::vector<float> ret;
	ret.reserve(this->data.size());
	for (auto p : this->data){
		float r = p.r / 255.f;
		float g = p.g / 255.f;
		float b = p.b / 255.f;
		float a = p.a / 255.f;

		r *= a;
		g *= a;
		b *= a;

		auto luma = r * .2126f + g * .7152f + b * .0722f;
		if (luma < 0)
			luma = 0;
		else if (luma > 1)
			luma = 1;

		ret.push_back(luma);
	}

	return { std::move(ret), w, h };
}
