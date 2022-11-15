#ifndef PAINT_NODE_H
#define PAINT_NODE_H

#include "core/math/vector2i.h"

#include "scene/2d/node_2d.h"

class PaintProject;

class PaintNode : public Node2D {
	GDCLASS(PaintNode, Node2D);

public:
	Vector2i get_size();
	void set_size(const Vector2i &size);

	PoolVector2iArray util_get_pixels_in_line(const Vector2i &from, const Vector2i &to);

	int util_to_1d_v(const Vector2i &p, int w);
	int util_to_1d(int x, int y, int w);
	Vector2i util_to_2d(int idx, int w);

	Color util_color_from_array(const PoolRealArray &color_array);
	Color util_random_color();
	Color util_random_color_alt();

	PaintProject *get_paint_project();

	String get_configuration_warning() const;

	PaintNode();
	~PaintNode();

protected:
	static void _bind_methods();

	Vector2i _size;
};

#endif
