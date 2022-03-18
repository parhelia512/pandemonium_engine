#ifndef GROUND_CLUTTER_FOLIAGE_2D_H
#define GROUND_CLUTTER_FOLIAGE_2D_H
/*
Copyright (c) 2019-2022 Péter Magyar

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

#include "core/vector.h"

#include "ground_clutter_2d.h"

#include "scene/resources/texture.h"

class GroundClutter2DFoliage : public GroundClutter2D {
	GDCLASS(GroundClutter2DFoliage, GroundClutter2D);

public:
	int get_texture_count() const;
	Ref<Texture> get_texture(const int index);
	void remove_texture(const int index);
	void add_texture(Ref<Texture> texture);

	GroundClutter2DFoliage();
	~GroundClutter2DFoliage();

private:
	static void _bind_methods();

private:
	Vector<Ref<Texture>> _textures;
};

#endif
