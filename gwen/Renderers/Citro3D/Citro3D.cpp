#include "Gwen/Gwen.h"
#include "Gwen/BaseRender.h"
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"
#include "Gwen/Renderers/Citro3D.h"

#include "lodepng.h"
#include "vshader_shbin.h"

#define 	RGBA8(r, g, b, a)   ((((a)&0xFF)<<24) | (((b)&0xFF)<<16) | (((g)&0xFF)<<8) | (((r)&0xFF)<<0))
#define TEXTURE_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

namespace Gwen
{
	namespace Renderer
	{
		struct FontInfo
		{
			C3D_Tex *glyphSheets;
		};

		Citro3D::Citro3D( C3D_RenderTarget *target, gfxScreen_t screen) : m_render_target(target), m_screen(screen), m_size(Gwen::Rect(1,1))
		{
			last_texture = NULL;
			m_vertices = (Vertex*)linearAlloc(m_max_vertices * sizeof(Vertex));
			m_num_vertices = 0;
			m_last_num_vertices = 0;

			for ( int i = 0; i < m_max_vertices; i++ )
			{
				m_vertices[i].z = 0.5f;
			}
		}

		Citro3D::~Citro3D()
		{
			linearFree(m_vertices);
			shaderProgramFree(&program);
			DVLB_Free(vshader_dvlb);
		}

		void Citro3D::Init()
		{
			vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
			shaderProgramInit(&program);
			shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
			C3D_BindProgram(&program);

			uLoc_projection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");

			// Configure attributes for use with the vertex shader
			// Attribute format and element count are ignored in immediate mode
			C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
			AttrInfo_Init(attrInfo);
			AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
			AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 4); // v1=color
			AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 2); // v2=texcoord

			C3D_BufInfo* bufInfo = C3D_GetBufInfo();
			BufInfo_Init(bufInfo);
			BufInfo_Add(bufInfo, m_vertices, sizeof(Vertex), 3, 0x210);

			Mtx_OrthoTilt(&projection, 0.0, 320.0, 240.0, 0.0, 0.0, 1.0, true);

			m_tev = C3D_GetTexEnv(0);
			C3D_TexEnvSrc(m_tev, C3D_Both, GPU_PRIMARY_COLOR, GPU_TEXTURE0, 0);
			C3D_TexEnvOp(m_tev, C3D_Both, 0, 0, 0);
			C3D_TexEnvFunc(m_tev, C3D_Both, GPU_ADD);

			// Configure depth test to overwrite pixels with the same depth (important!!)
			C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);

			textures_enabled = false;
			//DisableTextures();
		}

		void Citro3D::Begin()
		{
			C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
			C3D_FrameDrawOn(m_render_target);
			C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);
		}

		void Citro3D::End()
		{
			Flush();
			C3D_FrameEnd(0);

			m_num_vertices = 0; // Reset the buffer.
			m_last_num_vertices = 0;
			last_texture = NULL; // invalidate
		}

		void Citro3D::SetDrawColor( Gwen::Color color )
		{
			m_color_r = color.r / 255.0f;
			m_color_g = color.g / 255.0f;
			m_color_b = color.b / 255.0f;
			m_color_a = color.a / 255.0f;
		}

		void Citro3D::Flush()
		{
			if(m_num_vertices != m_last_num_vertices)
			{
				C3D_DrawArrays(GPU_TRIANGLE_STRIP, m_last_num_vertices, m_num_vertices - m_last_num_vertices);
			}
			m_last_num_vertices = m_num_vertices;
		}

		void Citro3D::AddVertexColored(int x, int y)
		{
			if(m_num_vertices == m_max_vertices) { printf("vertex buffer overflow, clipping!\n"); return; }

			//DisableTextures();

			int i = m_num_vertices++;
			m_vertices[i].x = x;
			m_vertices[i].y = y;
			m_vertices[i].r = m_color_r;
			m_vertices[i].g = m_color_g;
			m_vertices[i].b = m_color_b;
			m_vertices[i].a = m_color_a;
		}

		void Citro3D::AddVertexTextured(int x, int y, float u, float v)
		{
			if(m_num_vertices == m_max_vertices) { Flush(); }
			/*if(!textures_enabled)
			{
				Flush();
				EnableTextures();
			}*/

			int i = m_num_vertices++;
			m_vertices[i].x = x;
			m_vertices[i].y = y;

			m_vertices[i].u = u;
			m_vertices[i].v = v;
		}

		void Citro3D::DrawFilledRect( Gwen::Rect rect )
		{
			Translate(rect);

			int x0 = rect.x;
			int y0 = rect.y;
			int x1 = rect.x+rect.w;
			int y1 = rect.y+rect.h;

			AddVertexColored(x0, y0);
			AddVertexColored(x0, y1);
			AddVertexColored(x1, y0);
			AddVertexColored(x1, y1);
		}

        void Citro3D::DrawLinedRect( Gwen::Rect rect )
        {
            Base::DrawLinedRect( rect );
        }

        void Citro3D::DrawPixel( int x, int y )
        {
            Base::DrawPixel( x, y );
        }

		void Citro3D::LoadFont( Gwen::Font* font )
		{
			FontInfo *fi = new FontInfo();
			TGLP_s* glyph_info = fontGetGlyphInfo();
			fi->glyphSheets = (C3D_Tex*)malloc(sizeof(C3D_Tex)*glyph_info->nSheets);
			for (int i = 0; i < glyph_info->nSheets; i++)
			{
				C3D_Tex* tex = &fi->glyphSheets[i];
				tex->data = fontGetGlyphSheetTex(i);
				tex->fmt = (GPU_TEXCOLOR)glyph_info->sheetFmt;
				tex->size = glyph_info->sheetSize;
				tex->width = glyph_info->sheetWidth;
				tex->height = glyph_info->sheetHeight;
				tex->param = GPU_TEXTURE_MAG_FILTER(GPU_LINEAR) | GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)
					| GPU_TEXTURE_WRAP_S(GPU_CLAMP_TO_EDGE) | GPU_TEXTURE_WRAP_T(GPU_CLAMP_TO_EDGE);
			}

			font->data = fi;
		}

		void Citro3D::FreeFont( Gwen::Font* pFont )
		{
			FontInfo *fi = (FontInfo*)pFont->data;
			if(fi != NULL)
			{
				free(fi->glyphSheets);
				delete fi;
			}
		}

		void Citro3D::RenderText(Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString & text)
		{
			if(!pFont) return;
			if(!pFont->data)
			{
				LoadFont(pFont);
			}

			//EnableTextures();
			
			Translate(pos.x, pos.y);

			int x,y;
			x = pos.x;
			y = pos.y;

			FontInfo *finfo = (FontInfo*)pFont->data;
			TGLP_s* glyph_info = fontGetGlyphInfo();

			float gwen_scale = Scale();
			float font_adjust_scale = pFont->size / glyph_info->cellHeight;
			float scale = gwen_scale * font_adjust_scale;

			//ssize_t  units;
			wchar_t code;

			const wchar_t* p = (const wchar_t*)text.data();

			u32 flags = GLYPH_POS_CALC_VTXCOORD;
			int lastSheet = -1;

			while((code = *p++))
			{
				if (code == '\n')
				{
					x = pos.x;
					y += scale*fontGetInfo()->lineFeed;
				}
				else if (code > 0)
				{
					int glyphIdx = fontGlyphIndexFromCodePoint(code);
					fontGlyphPos_s data;
					fontCalcGlyphPos(&data, glyphIdx, flags, scale, scale);

					if (data.sheetIndex != lastSheet)
					{
						Flush();
						last_texture = NULL; // IMPORTANT
						lastSheet = data.sheetIndex;
						C3D_TexBind(0, &finfo->glyphSheets[lastSheet]);
					}

					int x0 = x+data.vtxcoord.left;
					int y0 = y+data.vtxcoord.top;
					int x1 = x+data.vtxcoord.right;
					int y1 = y+data.vtxcoord.bottom;

					AddVertexTextured(x0, y0, data.texcoord.left,  data.texcoord.top);
					AddVertexTextured(x0, y1, data.texcoord.left,  data.texcoord.bottom);
					AddVertexTextured(x1, y0, data.texcoord.right, data.texcoord.top);
					AddVertexTextured(x1, y1, data.texcoord.right, data.texcoord.bottom);

					x += data.xAdvance;
				}
			}
		}

		Gwen::Point Citro3D::MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString & text )
		{
			TGLP_s* glyph_info = fontGetGlyphInfo();
			u32 flags = GLYPH_POS_CALC_VTXCOORD;

			float gwen_scale = Scale();
			float font_adjust_scale = pFont->size / glyph_info->cellHeight;
			float scale = gwen_scale * font_adjust_scale;

			wchar_t code;
			
			if(!pFont->realsize)
			{
				pFont->realsize = pFont->size * scale;
			}

			float w = 0;
			float h = glyph_info->cellHeight * scale;

			const wchar_t *p = text.data();
			while((code = *p++))
			{
				int gi = fontGlyphIndexFromCodePoint(code);
				fontGlyphPos_s data;
				fontCalcGlyphPos(&data, gi, flags, scale, scale);
				w += (data.vtxcoord.right - data.vtxcoord.left) * scale;
				w += data.xAdvance * scale;

				if(code == ' ')
				{
					w += data.xAdvance * scale;
				}
			}

			return Gwen::Point((int)w, (int)h);
		}

		void Citro3D::StartClip()
		{
			Flush();

			Gwen::Rect rect = ClipRegion();
			float scale = Scale();

			int x0 = rect.x * scale;
			int y0 = rect.y * scale;
			int x1 = (rect.x+rect.w) * scale;
			int y1 = (rect.y+rect.h) * scale;

			int scr_w = 320;
			int scr_h = 240;

			if(m_screen == GFX_TOP)
			{
				scr_w = 400;
			}

			// maths :(
			C3D_SetScissor(GPU_SCISSOR_NORMAL, scr_h - y1, scr_w - x1, scr_h - y0, scr_w - x0); 
		}

		void Citro3D::EndClip()
		{
			Flush();
			C3D_SetScissor(GPU_SCISSOR_DISABLE, 0, 0, 0, 0);
		}

		void Citro3D::LoadTexture( Gwen::Texture* pTexture )
		{
			const char* file_name = pTexture->name.Get().c_str();
			
			unsigned char* image;
  			unsigned width, height;
			unsigned int error = lodepng_decode32_file(&image, &width, &height, file_name);
			if(!error)
			{
				unsigned char *vram_tex = (unsigned char*) linearAlloc(width * height * 4);
				u8* src=image; u8 *dst=vram_tex;
				for(unsigned int i = 0; i<width*height; i++) // swizzle RGBA -> ABGR
				{
					int r = *src++;
					int g = *src++;
					int b = *src++;
					int a = *src++;

					*dst++ = a;
					*dst++ = b;
					*dst++ = g;
					*dst++ = r;
				}

				C3D_Tex *tex = new C3D_Tex();
				GSPGPU_FlushDataCache(vram_tex, width*height*4);
				C3D_TexInit(tex, width, height, GPU_RGBA8);
				C3D_SafeDisplayTransfer ((u32*)vram_tex, GX_BUFFER_DIM(width,height), (u32*)tex->data, GX_BUFFER_DIM(width,height), TEXTURE_TRANSFER_FLAGS);
				gspWaitForPPF();

				C3D_TexSetFilter(tex, GPU_LINEAR, GPU_NEAREST);

				free(image);
				linearFree(vram_tex);
				pTexture->data = tex;
				pTexture->width = width;
				pTexture->height = height;
			}
			else
			{
				printf("Failed to load %s!!!!\n", file_name);	
				pTexture->failed = true;
			}
		}

		void Citro3D::FreeTexture( Gwen::Texture* pTexture )
		{
			if(pTexture->data != NULL)
			{
				C3D_TexDelete((C3D_Tex*)(pTexture->data));
			}
		}

		/*void Citro3D::EnableTextures()
		{
			if(!textures_enabled)
			{
				printf("Textures on\n");
				Flush();
				textures_enabled = true;
				//C3D_SetTexEnv
			}
		}

		void Citro3D::DisableTextures()
		{
			if(textures_enabled)
			{
				printf("Textures off\n");
				Flush();
				textures_enabled = false;
				last_texture = NULL; // invalidate
			}
		}*/

		void Citro3D::BindTexture(Gwen::Texture *tex)
		{
			if(tex != last_texture)
			{
				Flush();
				C3D_TexBind(0, (C3D_Tex*)(tex->data));
				last_texture = tex;
			}
		}

		void Citro3D::DrawTexturedRect(Gwen::Texture* pTexture, Gwen::Rect rect, float u1, float v1, float u2, float v2)
		{
			if(m_num_vertices == m_max_vertices) { printf("vertex buffer overflow, clipping!\n"); return; }

			//EnableTextures();
			BindTexture(pTexture);
			Translate(rect);

			int x0 = rect.x;
			int y0 = rect.y;
			int x1 = rect.x+rect.w;
			int y1 = rect.y+rect.h;

			AddVertexTextured(x0, y0, u1, v1);
			AddVertexTextured(x0, y1, u1, v2);
			AddVertexTextured(x1, y0, u2, v1);
			AddVertexTextured(x1, y1, u2, v2);
		}

		Gwen::Color Citro3D::PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color & col_default )
		{
            //printf("PixelColour stub!!\n");
            return Gwen::Color(0,0,0,0);
		}
		
	}
}
