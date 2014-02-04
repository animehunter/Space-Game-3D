
#pragma once

#include "RenderFramework/light_model.h"


class Asteroid
{
public:
	Asteroid(CL_GraphicContext &gc);
	~Asteroid();

	void render(CL_GraphicContext &gc, const LightModel &light_model);

private:
	void create(CL_GraphicContext &gc);
	void setup_light(CL_ProgramObject &program, const Material &material, const LightModel &light_model);
	CL_PixelBuffer load_image(const CL_String &filename);
	CL_Vec4f fractal(float x0, float y0, CL_Vec4f p);
	int fractal(float x0, float y0);

	struct Vertex
	{
		CL_Vec3f position;
		CL_Vec3f normal;
		CL_Vec2f texcoord0;
	};

	CL_ProgramObject program_object;
	CL_Texture texture, texture2;
	CL_VertexArrayBuffer vertices;
	CL_ElementArrayBuffer elements;
	int num_triangles;
	Material material;
};
