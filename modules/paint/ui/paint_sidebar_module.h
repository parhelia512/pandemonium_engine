#ifndef PAINT_SIDEBAR_MODULE_H
#define PAINT_SIDEBAR_MODULE_H

/*
Copyright (c) 2022 Péter Magyar

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

#include "scene/gui/panel_container.h"

class PaintNode;

class PaintSidebarModule : public PanelContainer {
	GDCLASS(PaintSidebarModule, PanelContainer);

public:
	void on_paint_node_selected(PaintNode *paint_node);
	void on_paint_node_selected_bind(Node *paint_node);
	virtual void _on_paint_node_selected(Node *paint_node);

	PaintSidebarModule();
	~PaintSidebarModule();

protected:
	static void _bind_methods();
};

#endif
