#ifndef VOXEL_LEVEL_GENERATOR_H
#define VOXEL_LEVEL_GENERATOR_H
/*
Copyright (c) 2019-2020 Péter Magyar

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
#include "core/object/gdvirtual.gen.inc"
#include "core/object/script_language.h"
#else
#include "core/resource.h"
#endif

class VoxelChunk;

class VoxelLevelGenerator : public Resource {
	GDCLASS(VoxelLevelGenerator, Resource);

public:
	void generate_chunk(Ref<VoxelChunk> chunk);

#if VERSION_MAJOR >= 4
	GDVIRTUAL1(_generate_chunk, Ref<VoxelChunk>);
#endif

	VoxelLevelGenerator();
	~VoxelLevelGenerator();

protected:
	static void _bind_methods();
};

#endif
