
#include "precomp.h"
#include "debris.h"

Debris::Debris(CL_GraphicContext &gc)
{
	create_shader_programs(gc);

	int vertex_count = 5000;
	vertices = CL_VertexArrayBuffer(gc, sizeof(Vertex)*vertex_count);
	vertices.lock(cl_access_write_only);
	Vertex *vertices_data = reinterpret_cast<Vertex *>(vertices.get_data());

	for (int i = 0; i < vertex_count; i++)
	{
		CL_Vec3f p(rand()/(float)RAND_MAX, rand()/(float)RAND_MAX, rand()/(float)RAND_MAX);
		p -= CL_Vec3f(0.5f, 0.5f, 0.5f);
		p *= 100.0f;
		vertices_data[i].position = p;
	}

	vertices.unlock();
}

Debris::~Debris()
{
}

void Debris::render(CL_GraphicContext &gc, const LightModel &light_model, const CL_Vec3f &camera_pos)
{
	// Yes, this is brain dead.  But the GPU is fast, right? ;)

	int offset_x = ((int)camera_pos.x)/100;
	int offset_y = ((int)camera_pos.y)/100;
	int offset_z = ((int)camera_pos.z)/100;
	for (int x = -1; x < 2; x++)
	{
		for (int y = -1; y < 2; y++)
		{
			for (int z = -1; z < 2; z++)
			{
				render_debris(gc, light_model, offset_x+x, offset_y+y, offset_z+z);
			}
		}
	}
}

void Debris::render_debris(CL_GraphicContext &gc, const LightModel &light_model, int offset_x, int offset_y, int offset_z)
{
	gc.set_program_object(program_object);
	program_object.set_uniform4f(L"offset", (float)offset_x*100, (float)offset_y*100, (float)offset_z*100, 0.0f);
	CL_Pen pen;
	//pen.enable_point_antialiasing(true);
	pen.enable_vertex_program_point_size(true);
	pen.set_point_size(3.0f*gc.get_width()/2560.0f);
	gc.set_pen(pen);
	CL_PrimitivesArray prim_array(gc);
	prim_array.set_attributes(0, vertices, 3, cl_type_float, &static_cast<Vertex*>(0)->position, sizeof(Vertex));
	gc.draw_primitives(cl_points, 48000, prim_array);
	gc.reset_program_object();
	gc.reset_pen();
}

void Debris::create_shader_programs(CL_GraphicContext &gc)
{
	CL_String resource_dir = "Resources/Debris/"; // CL_Directory::get_resourcedata(app_name);

	CL_VirtualDirectory vd;
	program_object = CL_ProgramObject::load(gc, resource_dir + L"debris_vertex.glsl", resource_dir + L"debris_fragment.glsl", vd);
	program_object.bind_attribute_location(0, "in_position");
	if (!program_object.link())
		throw CL_Exception("Unable to link 3ds model program object (texture)");
}
