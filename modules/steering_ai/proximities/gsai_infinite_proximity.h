#ifndef GSAI_INFINITE_PROXIMITY_H
#define GSAI_INFINITE_PROXIMITY_H

#include "core/object/func_ref.h"
#include "core/object/reference.h"

#include "gsai_proximity.h"

class GSAIInfiniteProximity : public GSAIProximity {
	GDCLASS(GSAIInfiniteProximity, GSAIProximity);

public:
	int _find_neighbors(Ref<FuncRef> callback);

	GSAIInfiniteProximity();
	~GSAIInfiniteProximity();

protected:
	static void _bind_methods();

	// Determines any agent that is in the specified list as being neighbors with the
	// owner agent, regardless of distance.
	// @category - Proximities
	// Returns a number of neighbors based on a `callback` function.
	//
	// `_find_neighbors` calls `callback` for each agent in the `agents` array and
	// adds one to the count if its `callback` returns true.
	// @tags - virtual
};

#endif
