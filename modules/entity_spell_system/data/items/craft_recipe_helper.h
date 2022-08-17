#ifndef CRAFT_RECIPE_HELPER_H
#define CRAFT_RECIPE_HELPER_H
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

#include "core/object/resource.h"

#include "item_template.h"

class CraftRecipeHelper : public Resource {
	GDCLASS(CraftRecipeHelper, Resource);

public:
	Ref<ItemTemplate> get_item();
	void set_item(Ref<ItemTemplate> value);
	int get_count();
	void set_count(int value);

	CraftRecipeHelper(Ref<ItemTemplate> item, int count);
	CraftRecipeHelper();
	~CraftRecipeHelper();

protected:
	static void _bind_methods();

private:
	Ref<ItemTemplate> _item;
	int _count;
};

#endif
