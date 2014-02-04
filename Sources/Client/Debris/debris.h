
#pragma once

class LightModel;
class Position;
class Orientation;

class Debris
{
public:
	Debris(CL_GraphicContext &gc);
	~Debris();

	void render(CL_GraphicContext &gc, const LightModel &light_model, const CL_Vec3f &camera_pos);

private:
	struct Vertex
	{
		CL_Vec3f position;
	};

	void create_shader_programs(CL_GraphicContext &gc);
	void render_debris(CL_GraphicContext &gc, const LightModel &light_model, int offset_x, int offset_y, int offset_z);

	CL_ProgramObject program_object;
	CL_VertexArrayBuffer vertices;
};
