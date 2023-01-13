/*
Copyright (c) 2023-present Péter Magyar
Copyright (c) 2020-2023 GDQuest

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

#include "register_types.h"

#include "core/config/engine.h"

#include "gsai_utils.h"

#include "gsai_agent_location.h"
#include "gsai_group_behavior.h"
#include "gsai_path.h"
#include "gsai_steering_agent.h"
#include "gsai_steering_behavior.h"
#include "gsai_target_acceleration.h"
#include "proximities/gsai_proximity.h"

#include "proximities/gsai_radius_proximity.h"
#include "proximities/gsai_infinite_proximity.h"

static GSAIUtils *gs_ai_utils = NULL;

void register_steering_ai_types() {
	gs_ai_utils = memnew(GSAIUtils);
	ClassDB::register_class<GSAIUtils>();
	Engine::get_singleton()->add_singleton(Engine::Singleton("GSAIUtils", GSAIUtils::get_singleton()));

	ClassDB::register_class<GSAITargetAcceleration>();
	ClassDB::register_class<GSAISteeringBehavior>();
	ClassDB::register_class<GSAISteeringAgent>();
	ClassDB::register_class<GSAIPath>();
	ClassDB::register_class<GSAIGroupBehavior>();
	ClassDB::register_class<GSAIAgentLocation>();
	ClassDB::register_class<GSAIProximity>();

	ClassDB::register_class<GSAIRadiusProximity>();
	ClassDB::register_class<GSAIInfiniteProximity>();
}

void unregister_steering_ai_types() {
	if (gs_ai_utils) {
		memdelete(gs_ai_utils);
	}
}
