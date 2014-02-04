
#include "precomp.h"
#include "asteroid.h"
#include "Sphere/sphere.h"

Asteroid::Asteroid(CL_GraphicContext &gc)
: num_triangles(0)
{
	create(gc);
}

Asteroid::~Asteroid()
{
}

void Asteroid::render(CL_GraphicContext &gc, const LightModel &light_model)
{
	CL_PolygonRasterizer polygon_rasterizer;
	polygon_rasterizer.set_culled(true);
	polygon_rasterizer.set_front_face(cl_face_side_clockwise);
	gc.set_polygon_rasterizer(polygon_rasterizer);

	CL_PrimitivesArray prim_array(gc);
	prim_array.set_attributes(0, vertices, 3, cl_type_float, &static_cast<Vertex*>(0)->position, sizeof(Vertex));
	prim_array.set_attributes(1, vertices, 3, cl_type_float, &static_cast<Vertex*>(0)->normal, sizeof(Vertex));
	prim_array.set_attributes(2, vertices, 2, cl_type_float, &static_cast<Vertex*>(0)->texcoord0, sizeof(Vertex));
	gc.set_primitives_array(prim_array);

	CL_BufferControl buffer_control;
	buffer_control.enable_depth_test(true);
	buffer_control.enable_depth_write(true);
	gc.set_buffer_control(buffer_control);

	gc.set_texture(0, texture);
	gc.set_texture(1, texture2);
	gc.set_program_object(program_object);
	program_object.set_uniform1i(L"texture1", 0);
	program_object.set_uniform1i(L"texture2", 1);
//	setup_light(program_object, material, light_model);
//	program_object.set_uniform_matrix(L"modelmatrix", CL_Mat4f::identity());

	gc.draw_primitives_elements(cl_triangles, num_triangles*3, elements, cl_type_unsigned_short);

	gc.reset_program_object();
	gc.reset_buffer_control();
	gc.reset_primitives_array();
	gc.reset_polygon_rasterizer();
}

CL_Vec4f Asteroid::fractal(float x0, float y0, CL_Vec4f p)
{
	return p + fractal((x0+p.x)*1000.0f, (y0+p.y)*1000.0f)/30.0f;
}

int Asteroid::fractal(float x0, float y0)
{
	float x = 0;
	float y = 0;

	int iteration = 0;
	int max_iteration = 10;
	while (x*x+y*y <= 2*2 && iteration < max_iteration)
	{
		float xtemp = x*x + y*y + x0;
		y = 2*x*y + y0;
		x = xtemp;
		iteration++;
	}
	if (iteration == max_iteration)
	{
		return 10;
	}
	else
	{
		return iteration;
	}
}

void Asteroid::create(CL_GraphicContext &gc)
{
	Sphere sphere;
	sphere.generate_octahedron();
	for (int i = 0; i < 4; i++)
		sphere.subdivide();

	CL_String resource_dir = "Resources/Asteroid/"; // CL_Directory::get_resourcedata(app_name);
	CL_VirtualDirectory vd;
	program_object = CL_ProgramObject::load(gc, resource_dir + L"asteroid_vertex.glsl", resource_dir + L"asteroid_fragment.glsl", vd);
	program_object.bind_attribute_location(0, "in_position");
	program_object.bind_attribute_location(1, "in_normal");
	if (!program_object.link())
		throw CL_Exception("Unable to link asteroid program object");

	texture = CL_Texture(gc, cl_texture_cube_map);
	texture.set_cube_map(
		load_image("Resources/Asteroid/ground.png"),
		load_image("Resources/Asteroid/ground.png"),
		load_image("Resources/Asteroid/ground.png"),
		load_image("Resources/Asteroid/ground.png"),
		load_image("Resources/Asteroid/ground.png"),
		load_image("Resources/Asteroid/ground.png"));
	texture.set_min_filter(cl_filter_linear);
	texture.set_mag_filter(cl_filter_linear);

	texture2 = CL_Texture(gc, cl_texture_cube_map);
	texture2.set_cube_map(
		load_image("Resources/Asteroid/sketch.png"),
		load_image("Resources/Asteroid/sketch.png"),
		load_image("Resources/Asteroid/sketch.png"),
		load_image("Resources/Asteroid/sketch.png"),
		load_image("Resources/Asteroid/sketch.png"),
		load_image("Resources/Asteroid/sketch.png"));
	texture2.set_min_filter(cl_filter_linear);
	texture2.set_mag_filter(cl_filter_linear);


	vertices = CL_VertexArrayBuffer(gc, 0, sphere.vertices.size()*sizeof(Vertex));
	vertices.lock(cl_access_write_only);
	Vertex *v = reinterpret_cast<Vertex*>(vertices.get_data());
	for (size_t i = 0; i < sphere.vertices.size(); i++)
	{
		v[i].position = sphere.vertices[i]*5.0f;
		v[i].normal = sphere.vertices[i];
		v[i].texcoord0 = CL_Vec2f(0.0f, 0.0f);
	}
	vertices.unlock();

	elements = CL_ElementArrayBuffer(gc, 0, sphere.triangles.size()*3*sizeof(unsigned short));
	elements.lock(cl_access_write_only);
	unsigned short *s = reinterpret_cast<unsigned short*>(elements.get_data());
	for (size_t i = 0; i < sphere.triangles.size(); i++)
	{
		for (size_t j = 0; j < 3; j++)
			s[i*3+j] = (unsigned short)sphere.triangles[i].vertex[j];
	}
	elements.unlock();
	num_triangles = sphere.triangles.size();
}

void Asteroid::setup_light(CL_ProgramObject &program, const Material &material, const LightModel &light_model)
{
/*
uniform vec4 lightSourcePosition;
uniform vec4 lightSourceHalfVector;
uniform vec4 lightSourceDiffuse;
uniform vec4 lightSourceAmbient;
uniform vec4 lightSourceSpecular;
uniform float lightSourceConstantAttenuation;
uniform float lightSourceLinearAttenuation;
uniform float lightSourceQuadraticAttenuation;
uniform vec4 frontMaterialDiffuse;
uniform vec4 frontMaterialAmbient;
uniform vec4 frontMaterialSpecular;
uniform float frontMaterialShininess;
uniform vec4 lightModelAmbient;
*/
	program.set_uniform4f("lightSourcePosition", light_model.light_sources[0].position);
	program.set_uniform4f("lightSourceHalfVector", light_model.light_sources[0].get_halfvector());
	program.set_uniform4f("lightSourceDiffuse", light_model.light_sources[0].diffuse);
	program.set_uniform4f("lightSourceAmbient", light_model.light_sources[0].ambient);
	program.set_uniform4f("lightSourceSpecular", light_model.light_sources[0].specular);
	program.set_uniform1f("lightSourceConstantAttenuation", light_model.light_sources[0].constant_attenuation);
	program.set_uniform1f("lightSourceLinearAttenuation", light_model.light_sources[0].constant_attenuation);
	program.set_uniform1f("lightSourceQuadraticAttenuation", light_model.light_sources[0].quadratic_attenuation);
	program.set_uniform4f("frontMaterialDiffuse", material.diffuse);
	program.set_uniform4f("frontMaterialAmbient", material.ambient);
	program.set_uniform4f("frontMaterialSpecular", material.specular);
	program.set_uniform1f("frontMaterialShininess", material.shininess);
	program.set_uniform4f("lightModelAmbient", light_model.scene_ambient);

/*
	program.set_uniform4f(L"lightSourcePosition", light_model.light_sources[0].position);
	program.set_uniform4f(L"frontLightProductAmbient", light_model.get_light_ambient(material, light_model.light_sources[0]));
	program.set_uniform4f(L"frontLightProductDiffuse", light_model.get_light_diffuse(material, light_model.light_sources[0]));
	program.set_uniform4f(L"frontLightProductSpecular", light_model.get_light_specular(material, light_model.light_sources[0]));
	program.set_uniform1f(L"frontMaterialShininess", material.shininess);
	program.set_uniform4f(L"frontLightModelProductSceneColor", light_model.get_scene_color(material));
*/
}

CL_PixelBuffer Asteroid::load_image(const CL_String &filename)
{
	// We flip the image because CL_Texture loads the image where 0,0 is upper left corner,
	// but most 3D software expects 0,0 to be the lower left corner.

	CL_PixelBuffer image = CL_ImageProviderFactory::load(filename).to_format(cl_rgba8);
	CL_Size image_size = image.get_size();
	unsigned int *p = static_cast<unsigned int *>(image.get_data());
	for (int y = 0; y < image_size.height/2; y++)
	{
		unsigned int *line1 = p+y*image_size.width;
		unsigned int *line2 = p+(image_size.height-y-1)*image_size.width;
		for (int x = 0; x < image_size.width; x++)
		{
			unsigned int t = line1[x];
			line1[x] = line2[x];
			line2[x] = t;
		}
	}
	return image;
}
