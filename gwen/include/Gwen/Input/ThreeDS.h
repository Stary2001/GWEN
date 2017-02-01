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
					m_Canvas = NULL;
					m_MouseX = 0;
					m_MouseY = 0;
                    m_LeftMouseDown = false;
                    m_RightMouseDown = false;
                    m_MiddleMouseDown = false;
                    m_XButton1MouseDown = false;
                    m_XButton2MouseDown = false;
				}

				void Initialize( Gwen::Controls::Canvas* c )
				{
					m_Canvas = c;
				}

				unsigned char TranslateKeyCode( int iKeyCode )
				{
					switch ( iKeyCode )
					{

					}

					return Gwen::Key::Invalid;
				}

				bool ProcessMessage()
				{
					if ( !m_Canvas ) { return false; }

					return false;
				}

			protected:

				Gwen::Controls::Canvas*	m_Canvas;
				int m_MouseX;
				int m_MouseY;
                bool m_LeftMouseDown;
                bool m_RightMouseDown;
                bool m_MiddleMouseDown;
                bool m_XButton1MouseDown;
                bool m_XButton2MouseDown;

		};
	}
}
#endif
