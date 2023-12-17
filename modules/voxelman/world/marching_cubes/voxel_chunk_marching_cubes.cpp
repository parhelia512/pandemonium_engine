/*************************************************************************/
/*  voxel_chunk_marching_cubes.cpp                                       */
/*************************************************************************/
/*                         This file is part of:                         */
/*                          PANDEMONIUM ENGINE                           */
/*             https://github.com/Relintai/pandemonium_engine            */
/*************************************************************************/
/* Copyright (c) 2022-present Péter Magyar.                              */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/



#include "voxel_chunk_marching_cubes.h"

#include "../../defines.h"

VoxelChunkMarchingCubes::VoxelChunkMarchingCubes() {
}

VoxelChunkMarchingCubes::~VoxelChunkMarchingCubes() {
}

void VoxelChunkMarchingCubes::_channel_setup() {
	channel_set_count(MAX_DEFAULT_CHANNELS);
}

void VoxelChunkMarchingCubes::_bind_methods() {
	ADD_PROPERTYI(PropertyInfo(Variant::POOL_BYTE_ARRAY, "data_channel"), "channel_set_compressed", "channel_get_compressed", 0);
	ADD_PROPERTYI(PropertyInfo(Variant::POOL_BYTE_ARRAY, "isolevel_channel"), "channel_set_compressed", "channel_get_compressed", 1);

	//ClassDB::bind_method(D_METHOD("get_channel_compressed", "channel_index"), &VoxelChunk::get_channel_compressed);
	//ClassDB::bind_method(D_METHOD("set_channel_compressed", "channel_index", "array"), &VoxelChunk::set_channel_compressed);
}
