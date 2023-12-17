

#include "register_types.h"

#include "library/terrain_surface.h"
#include "library/terrain_surface_simple.h"

#include "library/terrain_library.h"
#include "library/terrain_library_simple.h"
#include "library/terrain_material_cache.h"

#include "modules/modules_enabled.gen.h"

#ifdef MODULE_TEXTURE_PACKER_ENABLED
#include "library/terrain_library_merger.h"
#include "library/terrain_library_merger_pcm.h"
#include "library/terrain_material_cache_pcm.h"
#include "library/terrain_surface_merger.h"
#endif

#include "data/terrain_light.h"
#include "meshers/terrain_mesher.h"

#include "world/block_terrain_structure.h"
#include "world/terrain_chunk.h"
#include "world/terrain_environment_data.h"
#include "world/terrain_structure.h"
#include "world/terrain_world.h"

#include "world/default/terrain_chunk_default.h"
#include "world/default/terrain_world_default.h"

#include "level_generator/terrain_level_generator.h"
#include "level_generator/terrain_level_generator_flat.h"

#include "areas/terrain_world_area.h"

#include "world/terrain_world_editor.h"

#include "meshers/blocky/terrain_mesher_blocky.h"

#include "world/blocky/terrain_chunk_blocky.h"
#include "world/blocky/terrain_world_blocky.h"

#include "nodes/terrain_light_node.h"

#include "world/jobs/terrain_job.h"
#include "world/jobs/terrain_light_job.h"
#include "world/jobs/terrain_mesher_job_step.h"
#include "world/jobs/terrain_prop_job.h"
#include "world/jobs/terrain_terrain_job.h"

void register_terraman_types(ModuleRegistrationLevel p_level) {
	if (p_level == MODULE_REGISTRATION_LEVEL_SCENE) {
		ClassDB::register_class<TerrainMesher>();
		ClassDB::register_class<TerrainMesherDefault>();

		ClassDB::register_class<TerrainSurface>();
		ClassDB::register_class<TerrainSurfaceSimple>();

		ClassDB::register_class<TerrainLibrary>();
		ClassDB::register_class<TerrainLibrarySimple>();

		ClassDB::register_class<TerrainMaterialCache>();

#ifdef MODULE_TEXTURE_PACKER_ENABLED
		ClassDB::register_class<TerrainSurfaceMerger>();
		ClassDB::register_class<TerrainLibraryMerger>();
		ClassDB::register_class<TerrainLibraryMergerPCM>();
		ClassDB::register_class<TerrainMaterialCachePCM>();
#endif

		ClassDB::register_class<TerrainLight>();
		ClassDB::register_class<TerrainLightNode>();

		ClassDB::register_class<TerrainWorld>();
		ClassDB::register_class<TerrainChunk>();
		ClassDB::register_class<TerrainStructure>();
		ClassDB::register_class<BlockTerrainStructure>();
		ClassDB::register_class<TerrainEnvironmentData>();

		ClassDB::register_class<TerrainChunkDefault>();
		ClassDB::register_class<TerrainWorldDefault>();

		ClassDB::register_class<TerrainMesherBlocky>();
		ClassDB::register_class<TerrainWorldBlocky>();
		ClassDB::register_class<TerrainChunkBlocky>();

		ClassDB::register_class<TerrainLevelGenerator>();
		ClassDB::register_class<TerrainLevelGeneratorFlat>();

		ClassDB::register_class<TerrainWorldArea>();

		ClassDB::register_class<TerrainJob>();
		ClassDB::register_class<TerrainTerrainJob>();
		ClassDB::register_class<TerrainMesherJobStep>();
		ClassDB::register_class<TerrainLightJob>();
		ClassDB::register_class<TerrainPropJob>();
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_REGISTRATION_LEVEL_EDITOR) {
		EditorPlugins::add_by_type<TerrainWorldEditorPlugin>();
	}
#endif
}

void unregister_terraman_types(ModuleRegistrationLevel p_level) {
}
