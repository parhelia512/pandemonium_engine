#ifndef GSAI_KINEMATIC_BODY_2D_AGENT_H
#define GSAI_KINEMATIC_BODY_2D_AGENT_H

#include "gsai_specialized_agent.h"

class KinematicBody2D;
class GSAITargetAcceleration;

class GSAIKinematicBody2DAgent : public GSAISpecializedAgent {
	GDCLASS(GSAIKinematicBody2DAgent, GSAISpecializedAgent);

public:
	KinematicBody2D *get_body();
	void set_body(KinematicBody2D *p_body);
	void set_body_bind(Node *p_body);

	int get_movement_type() const;
	void set_movement_type(const int val);

	enum MovementType {
		SLIDE,
		COLLIDE,
		POSITION
	};

	void _physics_process_connect();
	void _physics_process_disconnect();
	
	void _apply_steering(Ref<GSAITargetAcceleration> acceleration, float delta);

	void _apply_sliding_steering(KinematicBody2D *body, const Vector3 &accel, const float delta);
	void _apply_collide_steering(KinematicBody2D *body, const Vector3 &accel, const float delta);
	void _apply_position_steering(KinematicBody2D *body, const Vector3 &accel, const float delta);
	void _apply_orientation_steering(KinematicBody2D *body, const float angular_acceleration, const float delta);

	void _on_SceneTree_physics_frame();

	GSAIKinematicBody2DAgent();
	~GSAIKinematicBody2DAgent();

protected:
	static void _bind_methods();

	// A specialized steering agent that updates itself every frame so the user does
	// not have to using a KinematicBody2D
	// @category - Specialized agents
	// SLIDE uses `move_and_slide`
	// COLLIDE uses `move_and_collide`
	// POSITION changes the `global_position` directly

	// The KinematicBody2D to keep track of
	// setget _set_body
	// The type of movement the body executes
	int movement_type;
	Vector2 _last_position;
	ObjectID _body_ref;
	// Moves the agent's `body` by target `acceleration`.
	// @tags - virtual
};

#endif
