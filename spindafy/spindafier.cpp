#include "spindafier.hpp"
#include "spot.hpp"
#include <exception>
#include <set>
#include <string>

namespace{
u64 compute_difference_energy(const Image &a, const Image &b);
u32 find_best_fit(const Resources &res, const Image &src, size_t mask_index);
u32 find_random_fit(const Resources &res, const Image &src, std::mt19937 &rng);
std::set<float>::const_iterator get_iterator(const std::set<float> &set, size_t i);
std::pair<float, bool> compute_pseudo_median(Image &img);
void apply_threshold(Image &img);
}

Spindafier::Spindafier(const std::filesystem::path &base): res(base){
	Image img(this->res.mask_bounding_box_size);
	img.fill({ 0, 0, 0 });
	for (int i = 0; i < 4; i++)
		this->black_cases[i] = find_best_fit(this->res, img, i);
	img.fill({ 0xFF, 0xFF, 0xFF });
	this->white_case = find_best_fit(this->res, img, 3);
}

Image Spindafier::spindafy(const Image &src_, std::mt19937 *rng) const{
	auto src = src_;
	apply_threshold(src);

	auto ret = Image::allocate_same_size(src);
	ret.fill({ 0, 0, 0 });

	//Spacing for spinda matrix.
	const int x_increment = 25;
	const int y_increment = 20;

	for (int y = 0; y < src.height(); y += y_increment){
		bool last_y = y + y_increment >= src.width();
		for (int x = 0; x < src.width(); x += x_increment){
			bool last_x = x + x_increment >= src.width();
			size_t mask_index = (size_t)last_x | ((size_t)last_y << 1);

			Image sliced(this->res.mask_bounding_box_size);
			Point p(x, y);
			sliced.blit(src, -p);

			u32 solution;
			auto color = get_color(sliced);
			if (!color){
				if (!rng)
					solution = find_best_fit(this->res, sliced, mask_index);
				else
					solution = find_random_fit(this->res, sliced, *rng);
			}else
				solution = !*color ? this->black_cases[mask_index] : this->white_case;

			ret.alpha_blend(SpotConfiguration(solution).full_render(res), p - this->res.mask_bounding_box_corner);
		}
	}

	return ret;
}

std::optional<bool> Spindafier::get_color(const Image &src){
	bool all_white;
	switch (src.get(0, 0).compute_integer_luma()){
		case 0:
			all_white = false;
			break;
		case 255:
			all_white = true;
			break;
		default:
			return {};
	}
	for (int y = 0; y < src.height(); y++){
		for (int x = 0; x < src.width(); x++){
			switch (src.get(x, y).compute_integer_luma()){
				case 0:
					if (all_white)
						return {};
					break;
				case 255:
					if (!all_white)
						return {};
					break;
				default:
					return {};
			}
		}
	}
	return all_white;
}

namespace{

u64 compute_difference_energy(const Image &a, const Image &b){
	auto [w, h] = a.size();
	if (w != b.width() || h != b.height())
		throw std::exception();

	u64 total_energy = 0;
	for (int y = 0; y < h; y++){
		for (int x = 0; x < w; x++){
			auto a0 = a.get(x, y);
			auto b0 = b.get(x, y);
			auto d = a0.simmetric_difference(b0);
			total_energy += d.compute_integer_luma();
		}
	}
	return total_energy;
}

u32 find_best_fit(const Resources &res, const Image &src, size_t mask_index){
	u32 ret = 0;
	for (int i = 0; i < spot_count; i++){
		int best = -1;
		std::optional<u64> min_diff;
		for (int j = 0; j < 256; j++){
			ret &= ~((u32)0xFF << (i * 8));
			ret |= (u32)j << (i * 8);

			auto rendered = SpotConfiguration(ret).minimal_render(res, mask_index);
			{
				Image rendered_sliced(res.mask_bounding_box_size);
				rendered_sliced.blit(rendered, -res.mask_bounding_box_corner);
				rendered = std::move(rendered_sliced);
			}

			auto diff = compute_difference_energy(src, rendered);

			if (!min_diff || diff < min_diff){
				best = j;
				min_diff = diff;
			}
		}
		ret &= ~((u32)0xFF << (i * 8));
		ret |= (u32)best << (i * 8);
	}
	return ret;
}

u32 find_random_fit(const Resources &res, const Image &src, std::mt19937 &rng){
	u32 ret = 0;
	std::optional<u64> min_diff;
	for (int i = 0; i < 256; i++){
		auto test = rng();
		auto rendered = SpotConfiguration(test).minimal_render(res);
		{
			Image rendered_sliced(res.mask_bounding_box_size);
			rendered_sliced.blit(rendered, -res.mask_bounding_box_corner);
			rendered = std::move(rendered_sliced);
		}
		auto diff = compute_difference_energy(src, rendered);

		if (!min_diff || diff < min_diff){
			ret = test;
			min_diff = diff;
		}
	}
	return ret;
}

std::set<float>::const_iterator get_iterator(const std::set<float> &set, size_t i){
	auto ret = set.begin();
	while (i--)
		++ret;
	return ret;
}

std::pair<float, bool> compute_pseudo_median(Image &img){
	std::set<float> unique_pixels;
	for (int y = 0; y < img.height(); y++)
		for (int x = 0; x < img.width(); x++)
			unique_pixels.insert(img.get(x, y).compute_luma());
	if (unique_pixels.size() < 2)
		return {*unique_pixels.begin(), true};
	float ret;
	auto n = unique_pixels.size() / 2;
	auto it = get_iterator(unique_pixels, n);
	if (unique_pixels.size() % 2)
		ret = *it;
	else{
		ret = *it;
		++it;
		ret = (ret + *it) * 0.5f;
	}
	return {ret, false};
}

void apply_threshold(Image &img){
	auto [threshold, single_color] = compute_pseudo_median(img);
	if (single_color)
		threshold = 0.5;
	static const pixel black(0, 0, 0);
	static const pixel white(255, 255, 255);
	for (int y = 0; y < img.height(); y++)
		for (int x = 0; x < img.width(); x++)
			img.set(x, y) = img.get(x, y).compute_luma() >= threshold ? white : black;
}

}
