/*
	GWEN
	Copyright (c) 2011 Facepunch Studios
	See license in Gwen.h
*/

#pragma once
#ifndef GWEN_RENDERERS_CITRO3D_H
#define GWEN_RENDERERS_CITRO3D_H
#include "Gwen/Gwen.h"
#include "Gwen/BaseRender.h"

#include <3ds.h>
#include <citro3d.h>

namespace Gwen
{
	namespace Renderer
	{
		struct Vertex
		{
			float x, y, z;
			float r, g, b, a;
			float u, v;
		};

		class Citro3D  : public Gwen::Renderer::Base
		{
			public:

				Citro3D(C3D_RenderTarget *target, gfxScreen_t screen);
				~Citro3D();

				virtual void Init();

				virtual void Begin();
				virtual void End();

				virtual void SetDrawColor( Gwen::Color color );

				virtual void DrawFilledRect( Gwen::Rect rect );

				virtual void LoadFont( Gwen::Font* pFont );
				virtual void FreeFont( Gwen::Font* pFont );
				virtual void RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString & text );
				virtual Gwen::Point MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString & text );

				void StartClip();
				void EndClip();

				void DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1 = 0.0f, float v1 = 0.0f, float u2 = 1.0f, float v2 = 1.0f );
				void LoadTexture( Gwen::Texture* pTexture );
				void FreeTexture( Gwen::Texture* pTexture );
				Gwen::Color PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color & col_default );

				virtual void DrawLinedRect( Gwen::Rect rect );
				virtual void DrawPixel( int x, int y );

			protected:
				bool textures_enabled;
				Vertex *m_vertices;
				int m_num_vertices;
				int m_last_num_vertices;
				const int m_max_vertices = 1023; 

				void Flush();
				void AddVertexTextured(int x, int y, float u = 0.0f , float v = 0.0f);
				void AddVertexColored(int x, int y);
				void EnableTextures();
				void DisableTextures();
				Gwen::Texture *last_texture;
				void BindTexture(Gwen::Texture *tex);
				
				C3D_TexEnv			*m_tev;

				float m_color_r, m_color_g, m_color_b, m_color_a;

				C3D_RenderTarget 	*m_render_target;
				gfxScreen_t m_screen;
				Gwen::Rect          m_size;

				int uLoc_projection;
				C3D_Mtx projection;

				DVLB_s* vshader_dvlb;
				shaderProgram_s program;	
		};
	}
}
#endif
