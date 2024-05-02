#include "spot.hpp"

SpotConfiguration::SpotConfiguration(u32 personality){
	for (int i = 0; i < spot_count; i++){
		u8 x = personality & 0x0F;
		personality >>= 4;
		u8 y = personality & 0x0F;
		personality >>= 4;
		spots[i] = Point(x, y);
	}
}

Image SpotConfiguration::full_render(const Resources &res) const{
	auto mask = Image::allocate_same_size(res.mask);
	for (int i = 0; i < spot_count; i++){
		auto position = this->spots[i] + spot_offsets[i];
		mask.alpha_blend(res.spots[i], position);
	}
	auto spots_final = res.mask;
	spots_final.multiply_alpha_from(mask);

	auto ret = res.base;
	ret.alpha_blend(spots_final);
	return ret;
}

Image SpotConfiguration::minimal_render(const Resources &res, size_t mask_index) const{
	auto mask = Image::allocate_same_size(res.mask);
	mask.fill({ 255, 255, 255 });
	auto ret = mask;
	for (int i = 0; i < spot_count; i++){
		auto position = this->spots[i] + spot_offsets[i];
		mask.alpha_blend(res.spots_inverted[i], position);
	}
	mask.multiply_alpha_from(res.masks_simple[mask_index]);
	ret.alpha_blend(mask);
	return ret;
}

Image SpotConfiguration::merge_all(const Resources &res){
	auto ret = Image::allocate_same_size(res.mask);
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 256; j++){
			SpotConfiguration sc(j << (i * 8));
			auto position = sc.spots[i] + spot_offsets[i];
			ret.alpha_blend(res.spots[i], position);
		}
	}
	ret.multiply_alpha_from(res.mask);
	return ret;
}
