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

#include "darken_action.h"

#include "../paint_canvas.h"
#include "../paint_canvas_layer.h"
#include "../paint_utilities.h"

void DarkenAction::do_action(PaintCanvas *canvas, const Array &data) {
	PaintAction::do_action(canvas, data);

	/*
	.do_action(canvas, data)

	var pixels = GEUtils.get_pixels_in_line(data[0], data[1])
	for pixel in pixels:
		if canvas.get_pixel_v(pixel) == null:
				continue

		if canvas.is_alpha_locked() and canvas.get_pixel_v(pixel) == Color.transparent:
			continue

		if pixel in action_data.undo.cells:
			var darkened_color = canvas.get_pixel_v(pixel).darkened(dark_factor)
			canvas.set_pixel_v(pixel, darkened_color)

			action_data.redo.cells.append(pixel)
			action_data.redo.colors.append(darkened_color)
			continue

		action_data.undo.colors.append(canvas.get_pixel_v(pixel))
		action_data.undo.cells.append(pixel)
		var darkened_color = canvas.get_pixel_v(pixel).darkened(dark_factor)
		canvas.set_pixel_v(pixel, darkened_color)

		action_data.redo.cells.append(pixel)
		action_data.redo.colors.append(darkened_color)
	*/
}

DarkenAction::DarkenAction() {
	dark_factor = 0.1;
}

DarkenAction::~DarkenAction() {
}

void DarkenAction::_bind_methods() {
}
