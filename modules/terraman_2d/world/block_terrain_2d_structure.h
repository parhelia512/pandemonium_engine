#ifndef BLOCK_TERRAIN_2D_STRUCTURE_H
#define BLOCK_TERRAIN_2D_STRUCTURE_H
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

#include "core/containers/vector.h"

#include "terrain_2d_structure.h"

#include "../defines.h"

#include "core/containers/pool_vector.h"

#include "terrain_2d_chunk.h"

class BlockTerrain2DStructure : public Terrain2DStructure {
	GDCLASS(BlockTerrain2DStructure, Terrain2DStructure);

public:
	int get_channel_type() const;
	void set_channel_type(const int value);

	int get_voxel_type(int p_x, int p_y) const;

	void set_voxel(int p_x, int p_y, int p_type, int p_isolevel);

	void _write_to_chunk(Ref<Terrain2DChunk> chunk);

	void clear();

	BlockTerrain2DStructure();
	~BlockTerrain2DStructure();

protected:
	static void _bind_methods();

public:
	struct DataEntry {
		int x;
		int y;
		int data_type;
		int data_isolevel;
	};

private:
	int _channel_type;
	int _channel_isolevel;

	Vector<DataEntry> _data;
};

#endif
