from PIL import Image, ImageChops, ImageDraw
from random import randint
import numpy as np

class SpindaConfig:
    sprite_base = Image.open("res/spinda_base.png")
    sprite_mask = Image.open("res/spinda_mask.png")
    spot_masks = [
        Image.open("res/spots/spot_1.png"),
        Image.open("res/spots/spot_2.png"),
        Image.open("res/spots/spot_3.png"),
        Image.open("res/spots/spot_4.png")
    ]
    spot_offsets = [
        (8, 6),
        (32, 7),
        (14, 24),
        (26, 25)
    ]
    def __init__(self):
        self.spots = [
            (0, 0),
            (0, 0),
            (0, 0),
            (0, 0)
        ]

    def __str__(self):
        return f"<SpindaConfig> {self.spots}"
    
    @staticmethod
    def from_personality(pers):
        self = SpindaConfig()
        self.spots[0] = (pers & 0x0000000f, (pers & 0x000000f0) >> 4)
        self.spots[1] = ((pers & 0x00000f00) >> 8, (pers & 0x0000f000) >> 12)
        self.spots[2] = ((pers & 0x000f0000) >> 16, (pers & 0x00f00000) >> 20)
        self.spots[3] = ((pers & 0x0f000000) >> 24, (pers & 0xf0000000) >> 28)
        return self
    
    @staticmethod
    def random():
        return SpindaConfig.from_personality(randint(0, 0x100000000))

    def get_personality(self):
        pers = 0x00000000
        for i, spot in enumerate(self.spots):
            pers = pers | (spot[0] << i*8) | (spot[1] << i*8+4)
        return pers

    def is_spot_n(self, pos, n):
        pos_adjusted = (
            pos[0] - self.spot_offsets[n][0] - self.spots[n][0],
            pos[1] - self.spot_offsets[n][1] - self.spots[n][1]
        )
        mask = self.spot_masks[n]

        # if the position lies outside the spot image: return false
        if pos_adjusted[0] < 0 or pos_adjusted[1] < 0 or pos_adjusted[0] >= mask.width or pos_adjusted[1] >= mask.height:
            return False
        
        # else: return true if the corresponding pixel is white
        mask_pixel = mask.getpixel(pos_adjusted)
        if mask_pixel == (255, 255, 255, 255):
            return True
        return False

    def is_spot(self, pos):
        for n in range(4):
            if self.is_spot_n(pos, n):
                return True
        return False

    def render_pattern(self, only_pattern = False, crop = False):
        size = self.sprite_base.size
        img = Image.new("RGBA", size)

        mask_arr = np.asarray(self.sprite_mask)
        base_arr = np.asarray(self.sprite_base)

        draw = ImageDraw.ImageDraw(img)

        for x in range(size[0]):
            for y in range(size[1]):
                # apply mask
                mask_pixel = tuple(mask_arr[y][x])
                base_pixel = tuple(base_arr[y][x])

                if self.is_spot((x, y)) and mask_pixel[3] != 0:
                    if only_pattern:
                        draw.point((x, y), (255, 255, 255, 255))
                    else:
                        draw.point((x, y), mask_pixel)
                else:
                    if only_pattern:
                        draw.point((x, y), (0, 0, 0, 255))
                    else:
                        draw.point((x, y), base_pixel)

        if crop: img = img.crop((17, 15, 52, 48))

        return img

    def get_difference(self, target):
        target_grayscale = target.convert("L")
        total_diff = 0
        for x in range(target_grayscale.size[0]):
            for y in range(target_grayscale.size[1]):
                result = 1 if self.is_spot((x, y)) else 0
                pixel = target_grayscale.getpixel((x, y))
                total_diff += abs(pixel - result)
        return total_diff

if __name__ == "__main__":
    spin = SpindaConfig.from_personality(0x7a397866)
    print(hex(spin.get_personality()))