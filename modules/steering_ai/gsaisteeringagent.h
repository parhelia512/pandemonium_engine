#ifndef GSAISTEERINGAGENT_H
#define GSAISTEERINGAGENT_H


class GSAISteeringAgent : public GSAIAgentLocation {
 GDCLASS(GSAISteeringAgent, GSAIAgentLocation);

 public:

 float get_zero_linear_speed_threshold() const;
 void set_zero_linear_speed_threshold(const float val);

 float get_linear_speed_max() const;
 void set_linear_speed_max(const float val);

 float get_linear_acceleration_max() const;
 void set_linear_acceleration_max(const float val);

 float get_angular_speed_max() const;
 void set_angular_speed_max(const float val);

 float get_angular_acceleration_max() const;
 void set_angular_acceleration_max(const float val);

 Vector3 get_linear_velocity();
 void set_linear_velocity(const Vector3 &val);

 float get_angular_velocity() const;
 void set_angular_velocity(const float val);

 float get_bounding_radius() const;
 void set_bounding_radius(const float val);

 bool get_is_tagged() const;
 void set_is_tagged(const bool val);


 GSAISteeringAgent();
 ~GSAISteeringAgent();

 protected:
 static void _bind_methods();

 // Adds velocity, speed, and size data to `GSAIAgentLocation`.
 //
 // It is the character's responsibility to keep this information up to date for
 // the steering toolkit to work correctly.
 // @category - Base types
 // The amount of velocity to be considered effectively not moving.
 float zero_linear_speed_threshold = 0.01;
 // The maximum speed at which the agent can move.
 float linear_speed_max = 0.0;
 // The maximum amount of acceleration that any behavior can apply to the agent.
 float linear_acceleration_max = 0.0;
 // The maximum amount of angular speed at which the agent can rotate.
 float angular_speed_max = 0.0;
 // The maximum amount of angular acceleration that any behavior can apply to an
 // agent.
 float angular_acceleration_max = 0.0;
 // Current velocity of the agent.
 Vector3 linear_velocity = Vector3.ZERO;
 // Current angular velocity of the agent.
 float angular_velocity = 0.0;
 // The radius of the sphere that approximates the agent's size in space.
 float bounding_radius = 0.0;
 // Used internally by group behaviors and proximities to mark the agent as already
 // considered.
 bool is_tagged = false;
};


#endif
