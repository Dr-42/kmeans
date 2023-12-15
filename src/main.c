#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#define DR42_TRACE_IMPLEMENTATION
#include "trace.h"

typedef struct color_t {
	unsigned char r, g, b;
} color_t;

typedef struct centroid_t {
	color_t color;
	size_t count;
	color_t *pixels;
} centroid_t;

float distance(const color_t *a, const color_t *b) {
	return sqrt(pow(a->r - b->r, 2) + pow(a->g - b->g, 2) + pow(a->b - b->b, 2));
}

void write_output(const char* prefix, size_t iter, const color_t* colors, size_t num_colors) {
	char filename[256];
	sprintf(filename, "%s_%zu.png", prefix, iter);
	printf("Writing %s\n", filename);
	uint8_t* data = malloc(sizeof(uint8_t) * 256 * 256 * 3 * num_colors);
	for (size_t i = 0; i < num_colors; i++) {
		for (size_t j = 0; j < 256 * 256; j++) {
			data[i * 256 * 256 * 3 + j * 3] = colors[i].r;
			data[i * 256 * 256 * 3 + j * 3 + 1] = colors[i].g;
			data[i * 256 * 256 * 3 + j * 3 + 2] = colors[i].b;
		}
	}
	stbi_write_png(filename, 256, 256 * num_colors, 3, data, 256 * 3);
	free(data);
}

void print_colors(const color_t* colors, size_t num_colors) {
	for (size_t i = 0; i < num_colors; i++) {
		printf("%zu: %d %d %d\n", i, colors[i].r, colors[i].g, colors[i].b);
	}
}

color_t* extract_colors(uint8_t* data, size_t data_size, color_t* out, size_t out_size) {
	if (out == NULL) {
		out = malloc(sizeof(color_t) * out_size);
		for (size_t i = 0; i < out_size; i++) {
			out[i].r = rand() % 256;
			out[i].g = rand() % 256;
			out[i].b = rand() % 256;
		}
	}

	centroid_t* centroids;
	centroids = malloc(sizeof(centroid_t) * out_size);
	memset(centroids, 0, sizeof(centroid_t) * out_size);
	for (size_t i = 0; i < out_size; i++) {
		centroids[i].color = out[i];
	}
	
	for (size_t i = 0; i < data_size; i += 3) {
		color_t pixel = {data[i], data[i + 1], data[i + 2]};
		float min_dist = distance(&pixel, &centroids[0].color);
		size_t min_index = 0;
		for (size_t j = 1; j < out_size; j++) {
			float dist = distance(&pixel, &centroids[j].color);
			if (dist < min_dist) {
				min_dist = dist;
				min_index = j;
			}
		}
		centroids[min_index].count++;
		centroids[min_index].pixels = realloc(centroids[min_index].pixels, sizeof(color_t) * centroids[min_index].count);
		centroids[min_index].pixels[centroids[min_index].count - 1] = pixel;
	}

	// Recalculate centroids
	for (size_t i = 0; i < out_size; i++) {
		float r = 0, g = 0, b = 0;
		for (size_t j = 0; j < centroids[i].count; j++) {
			r += centroids[i].pixels[j].r;
			g += centroids[i].pixels[j].g;
			b += centroids[i].pixels[j].b;
		}
		centroids[i].color.r = r / centroids[i].count;
		centroids[i].color.g = g / centroids[i].count;
		centroids[i].color.b = b / centroids[i].count;
	}

	for (size_t i = 0; i < out_size; i++) {
		out[i] = centroids[i].color;
	}

	free(centroids);
	return out;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Usage: %s <num_colors>\n", argv[0]);
		return 1;
	}
	const char* filename = argv[1];
	int width, height, channels;
	uint8_t *data = stbi_load(filename, &width, &height, &channels, 3);
	printf("Width: %d\nHeight: %d\nChannels: %d\n", width, height, channels);
	size_t num_dom_colors = 10;
	color_t* dom_colors = NULL;
	size_t data_size = width * height * 3;
	size_t iters = 10;
	for (size_t i = 0; i < iters; i++){
		dom_colors = extract_colors(data, data_size, dom_colors, num_dom_colors);
		write_output("out/output", i, dom_colors, num_dom_colors);
	}
	return 0;
}
