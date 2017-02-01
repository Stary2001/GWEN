/*
	GWEN
	Copyright (c) 2011 Facepunch Studios
	See license in Gwen.h
*/

#pragma once
#ifndef GWEN_INPUT_THREEDS_H
#define GWEN_INPUT_THREEDS_H

#include "Gwen/InputHandler.h"
#include "Gwen/Gwen.h"
#include "Gwen/Controls/Canvas.h"

#include <3ds.h>

namespace Gwen
{
	namespace Input
	{
		class ThreeDS
		{
			public:

				ThreeDS()
				{
					m_canvas = NULL;
					m_lastx = 0;
					m_lasty = 0;
                    m_touching = false;
				}

				void Initialize(Gwen::Controls::Canvas* c)
				{
					m_canvas = c;
				}

				unsigned char TranslateKeyCode(int iKeyCode)
				{
					return Gwen::Key::Invalid;
				}

				bool Poll()
				{
					if (!m_canvas) { return false; }

					u32 kDown = hidKeysDown();
					u32 kUp = hidKeysUp();
					u32 kHeld = hidKeysHeld();

					touchPosition touch;

					if(kDown & KEY_TOUCH)
					{
						hidTouchRead(&touch);
						m_touching = true;
						m_lastx = touch.px;
						m_lasty = touch.py;
						m_canvas->InputMouseMoved(touch.px, touch.py, 0, 0);

						return m_canvas->InputMouseButton(0, true);
					}
					else if(kUp & KEY_TOUCH)
					{
						m_touching = false;
						m_lastx = 0;
						m_lasty = 0;
						return m_canvas->InputMouseButton(0, false);
					}
					else if(kHeld & KEY_TOUCH)
					{
						hidTouchRead(&touch);
						int dx = touch.px - m_lastx;
						int dy = touch.px - m_lasty;

						m_lastx = touch.px;
						m_lasty = touch.py;
						m_canvas->InputMouseMoved(touch.px, touch.py, dx, dy);
					}

					return false;
				}

			protected:

				Gwen::Controls::Canvas*	m_canvas;
				int m_lastx;
				int m_lasty;
                bool m_touching;

		};
	}
}
#endif
