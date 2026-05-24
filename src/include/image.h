#ifndef IMAGE_H
#define IMAGE_H

struct image {
	int width;
	int height;
	int channels;
	unsigned char *pixels;
};

int image_load(
	struct image *img,
	const char *filename
);

void image_destroy(
	struct image *img
);

#endif
