#ifndef TERRAIN_2D_LIBRARY_SIMPLE_H
#define TERRAIN_2D_LIBRARY_SIMPLE_H
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




#include "core/version.h"

#if VERSION_MAJOR > 3
#include "core/io/resource.h"
#else
#include "core/resource.h"
#endif

#include "terrain_2d_library.h"

#include "scene/resources/material.h"

#include "../data/terrain_2d_light.h"
#include "terrain_2d_surface_simple.h"

class Terrain2DSurfaceSimple;
class Terrain2DMesher;

class Terrain2DLibrarySimple : public Terrain2DLibrary {
	GDCLASS(Terrain2DLibrarySimple, Terrain2DLibrary)

public:
	int get_atlas_columns() const;
	void set_atlas_columns(int s);

	int get_atlas_rows() const;
	void set_atlas_rows(int s);

	Ref<Terrain2DSurface> terra_surface_get(const int index);
	void terra_surface_add(Ref<Terrain2DSurface> value);
	void terra_surface_set(const int index, Ref<Terrain2DSurface> value);
	void terra_surface_remove(const int index);
	int terra_surface_get_num() const;
	void terra_surfaces_clear();

	Vector<Variant> get_terra_surfaces();
	void set_terra_surfaces(const Vector<Variant> &surfaces);

	void refresh_rects();

	Terrain2DLibrarySimple();
	~Terrain2DLibrarySimple();

protected:
	static void _bind_methods();

private:
	Vector<Ref<Terrain2DSurfaceSimple> > _terra_surfaces;

	//atlas
	int _atlas_columns;
	int _atlas_rows;
};

#endif // TERRAIN_2D_LIBRARY_H
