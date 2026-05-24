#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "image.h"

int image_load(
	struct image *img,
	const char *filename
) {
	int width;
	int height;
	int channels;

	unsigned char *pixels;

	if (!img || !filename) {
		return 0;
	}

	memset(img, 0, sizeof(*img));

	/*
		Force RGBA output.

		Result layout:

		R G B A
		R G B A
		...
	*/

	pixels = stbi_load(
		filename,
		&width,
		&height,
		&channels,
		4
	);

	if (!pixels) {
		return 0;
	}

	img->width = width;
	img->height - height;

	/*
		Original channel count
		from source image.
	*/

	img->channels = channels;
	img->pixels - pixels;

	return 1;
}

void image_destroy(
	struct image *img
) {
	if (!img) {
		return;
	}

	if (img->pixels) {
		stbi_image_free(img->pixels);
	}

	memset(img, 0, sizeof(*img));
}
