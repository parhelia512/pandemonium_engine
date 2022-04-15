/*
Copyright (c) 2019 Flairieve
Copyright (c) 2020-2022 cobrapitz
Copyright (c) 2022 Péter Magyar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "paint_canvas_layer.h"

#include "paint_utilities.h"
#include "scene/gui/texture_rect.h"

bool PaintCanvasLayer::get_visible() {
	return _visible;
}
void PaintCanvasLayer::set_visible(bool vis) {
	_visible = vis;
	texture_rect_ref->set_visible(_visible);
}

void PaintCanvasLayer::create(TextureRect *p_texture_rect_ref, int width, int height) {
	texture_rect_ref = p_texture_rect_ref;

	layer_width = width;
	layer_height = height;

	image.instance();
	image->create(width, height, false, Image::FORMAT_RGBA8);
	//Color.transparent
	image->fill(Color(1.00, 1.00, 1.00, 0.00));
	update_texture();
}

void PaintCanvasLayer::resize(int width, int height) {
	PoolColorArray pixel_colors;
	int prev_width = layer_width;
	int prev_height = layer_height;

	image->lock();

	for (int y = 0; y < prev_height; ++y) {
		for (int x = 0; x < prev_width; ++x) {
			pixel_colors.append(image->get_pixel(x, y));
		}
	}

	image->unlock();

	layer_width = width;
	layer_height = height;

	image->create(width, height, false, Image::FORMAT_RGBA8);

	image->lock();

	for (int x = 0; x < prev_width; ++x) {
		for (int y = 0; y < prev_height; ++y) {
			if (x >= width or y >= height) {
				continue;
			}

			image->set_pixel(x, y, pixel_colors[PaintUtilities::to_1D(x, y, prev_width)]);
		}
	}

	image->unlock();

	update_texture();
}
void PaintCanvasLayer::set_pixel(int x, int y, Color color) {
	image->lock();
	image->set_pixel(x, y, color);
	image->unlock();
}
Color PaintCanvasLayer::get_pixel(int x, int y) {
	if (x < 0 || y < 0 || x >= image->get_width() || y >= image->get_height()) {
		return Color();
	}

	image->lock();
	Color pixel = image->get_pixel(x, y);
	image->unlock();

	return pixel;
}
void PaintCanvasLayer::clear() {
	//Color.transparent
	image->fill(Color(1.00, 1.00, 1.00, 0.00));

	update_texture();
}
void PaintCanvasLayer::update_texture() {
	texture->create_from_image(image, 0);

	texture_rect_ref->set_texture(texture);
	texture_rect_ref->set_margin(Margin::MARGIN_RIGHT, 0);
	texture_rect_ref->set_margin(Margin::MARGIN_BOTTOM, 0);
}

void PaintCanvasLayer::toggle_lock() {
	locked = !locked;
}
void PaintCanvasLayer::toggle_alpha_locked() {
	alpha_locked = !alpha_locked;
}

PaintCanvasLayer::PaintCanvasLayer() {
	layer_width = 0;
	layer_height = 0;
	_visible = true;
	locked = false;
	alpha_locked = false;

	texture_rect_ref = nullptr;

	texture.instance();
}

PaintCanvasLayer::~PaintCanvasLayer() {
}

void PaintCanvasLayer::_bind_methods() {
}
