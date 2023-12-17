#ifndef NAVIGATION_PATH_QUERY_RESULT_2D_H
#define NAVIGATION_PATH_QUERY_RESULT_2D_H

/*************************************************************************/
/*  navigation_path_query_result_2d.h                                    */
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

#include "core/object/reference.h"
#include "servers/navigation/navigation_utilities.h"

class NavigationPathQueryResult2D : public Reference {
	GDCLASS(NavigationPathQueryResult2D, Reference);

	Vector<Vector2> path;
	Vector<int> path_types;
	Array path_rids;
	Vector<ObjectID> path_owner_ids;

protected:
	static void _bind_methods();

public:
	enum PathSegmentType {
		PATH_SEGMENT_TYPE_REGION = 0,
		PATH_SEGMENT_TYPE_LINK = 1,
	};

	void set_path(const Vector<Vector2> &p_path);
	Vector<Vector2> get_path() const;

	void set_path_types(const Vector<int> &p_path_types);
	Vector<int> get_path_types() const;

	void set_path_rids(const Array &p_path_rids);
	Array get_path_rids() const;

	void set_path_owner_ids(const Vector<ObjectID> &p_path_owner_ids);
	Vector<ObjectID> get_path_owner_ids() const;

	void set_path_owner_ids_bind(const Array p_path_owner_ids);
	Array get_path_owner_ids_bind() const;

	void set_from_query_result(const NavigationUtilities::PathQueryResult2D &p_result);

	void reset();
};

VARIANT_ENUM_CAST(NavigationPathQueryResult2D::PathSegmentType);

#endif // NAVIGATION_PATH_QUERY_RESULT_2D_H
