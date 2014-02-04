
#include "precomp.h"
#include "model.h"
#include "lib3ds_help.h"

Model::Model(CL_GraphicContext &gc, const CL_StringRef &filename, TextureTactic texture_tactic)
: scale(1.0f)
{
	CL_PixelBuffer image;
	if (texture_tactic == use_silver)
		image = load_image("Resources/Model/silver.jpg");
	else if (texture_tactic == use_wood)
		image = load_image("Resources/Model/wood.jpg");
	if (!image.is_null())
	{
		wood_texture = CL_Texture(gc, image.get_size());
		//wood_texture.set_generate_mipmap();
		wood_texture.set_image(image);
		wood_texture.set_min_filter(cl_filter_linear);
		wood_texture.set_mag_filter(cl_filter_linear);
		//wood_texture.set_min_filter(cl_filter_linear_mipmap_linear);
		//wood_texture.set_mag_filter(cl_filter_linear_mipmap_linear);
		wood_texture.set_max_anisotropy(8.0f);
	}

	create_shader_programs(gc);
	load_model(gc, filename);
}

Model::~Model()
{
}

void Model::render_instanced(CL_GraphicContext &gc, const LightModel &light_model, const std::vector<Position> &position, const std::vector<Orientation> &orientation)
{
	CL_PolygonRasterizer polygon_rasterizer;
	polygon_rasterizer.set_culled(false/*true*/);
	polygon_rasterizer.set_front_face(cl_face_side_counter_clockwise);
	gc.set_polygon_rasterizer(polygon_rasterizer);

	CL_PrimitivesArray prim_array(gc);
	prim_array.set_attributes(0, vertices, 3, cl_type_float, &static_cast<Vertex*>(0)->position, sizeof(Vertex));
	prim_array.set_attributes(1, vertices, 3, cl_type_float, &static_cast<Vertex*>(0)->normal, sizeof(Vertex));
	prim_array.set_attributes(2, vertices, 2, cl_type_float, &static_cast<Vertex*>(0)->texcoord0, sizeof(Vertex));
	gc.set_primitives_array(prim_array);

	program_object_texture.set_uniform1i("texture1", 0);

	matrix_instanced_cache.clear();
	for (size_t i = 0; i < position.size(); i++)
	{
		CL_Mat4f modelmatrix = CL_Mat4f::identity(); // gc.get_modelview();
		modelmatrix.multiply(position[i].to_matrix());
		modelmatrix.multiply(orientation[i].to_matrix().inverse());
		modelmatrix.multiply(CL_Mat4f::scale(scale, scale, scale));
		matrix_instanced_cache.push_back(modelmatrix);
	}

	//gc.push_modelview();
	for (int pass = 1; pass < 2; pass++)
	{
		CL_BufferControl buffer_control;
		buffer_control.enable_depth_test(true);
		buffer_control.enable_depth_write(pass == 1);
		gc.set_buffer_control(buffer_control);

		std::map<int, MaterialRange>::iterator it;
		for (it = material_ranges.begin(); it != material_ranges.end(); ++it)
		{
			CL_ProgramObject *program = 0;
			if (!wood_texture.is_null())
			{
				gc.set_texture(0, wood_texture);
				if (pass)
				{
					program = &program_object_texture;
					gc.set_program_object(program_object_texture);
				}
				else
				{
					program = &program_mirror_object_texture;
				}
				gc.set_program_object(*program);
				setup_light(*program, it->second.material, light_model);
			}
			else
			{
				if (!it->second.texture.is_null())
				{
					gc.set_texture(0, it->second.texture);
					if (pass)
					{
						program = &program_object_texture;
					}
					else
					{
						program = &program_mirror_object_texture;
					}
					gc.set_program_object(*program);
					setup_light(*program, it->second.material, light_model);
				}
				else
				{
					if (pass)
					{
						program = &program_object_no_texture;
					}
					else
					{
						program = &program_mirror_object_no_texture;
					}

					gc.set_program_object(*program);
					setup_light(*program, it->second.material, light_model);
				}
			}

			for (size_t i = 0; i < position.size(); i++)
			{
				//gc.set_modelview(matrix_instanced_cache[i]);
				program->set_uniform_matrix(L"modelmatrix", matrix_instanced_cache[i]);
				gc.draw_primitives_array(cl_triangles, it->second.start, it->second.length);
			}
		}
	}
	//gc.pop_modelview();

	gc.reset_texture(0);
	gc.reset_program_object();

	gc.reset_primitives_array();
	gc.reset_polygon_rasterizer();
}

void Model::render(CL_GraphicContext &gc, const LightModel &light_model, const Position &position, const Orientation &orientation)
{
	std::vector<Position> positions;
	positions.push_back(position);
	std::vector<Orientation> orientations;
	orientations.push_back(orientation);
	render_instanced(gc, light_model, positions, orientations);

#ifdef foobarism
	CL_Mat4f modelmatrix = CL_Mat4f::identity();
	modelmatrix.multiply(position.to_matrix());
	modelmatrix.multiply(orientation.to_matrix());
	modelmatrix.multiply(CL_Mat4f::scale(scale, scale, scale));
	gc.push_modelview();
	gc.mult_modelview(modelmatrix);

	CL_PolygonRasterizer polygon_rasterizer;
	polygon_rasterizer.set_culled(false/*true*/);
	polygon_rasterizer.set_front_face(cl_face_side_counter_clockwise);
	gc.set_polygon_rasterizer(polygon_rasterizer);

	CL_PrimitivesArray prim_array(gc);
	prim_array.set_attributes(0, vertices, 3, cl_type_float, &static_cast<Vertex*>(0)->position, sizeof(Vertex));
	prim_array.set_attributes(1, vertices, 3, cl_type_float, &static_cast<Vertex*>(0)->normal, sizeof(Vertex));
	prim_array.set_attributes(2, vertices, 2, cl_type_float, &static_cast<Vertex*>(0)->texcoord0, sizeof(Vertex));
	gc.set_primitives_array(prim_array);

	program_object_texture.set_uniform1i("texture1", 0);

	std::map<int, MaterialRange>::iterator it;
	for (it = material_ranges.begin(); it != material_ranges.end(); ++it)
	{
		if (!wood_texture.is_null())
		{
			gc.set_texture(0, wood_texture);
			gc.set_program_object(program_object_texture);
			setup_light(program_object_texture, it->second.material, light_model);
		}
		else
		{
			if (!it->second.texture.is_null())
			{
				gc.set_texture(0, it->second.texture);
				gc.set_program_object(program_object_texture);
				setup_light(program_object_texture, it->second.material, light_model);
			}
			else
			{
				gc.set_program_object(program_object_no_texture);
				setup_light(program_object_no_texture, it->second.material, light_model);
			}
		}

		gc.draw_primitives_array(cl_triangles, it->second.start, it->second.length);
	}

	gc.reset_texture(0);
	gc.reset_program_object();

	gc.reset_primitives_array();
	gc.reset_polygon_rasterizer();
	gc.pop_modelview();
#endif
}

void Model::setup_light(CL_ProgramObject &program, const Material &material, const LightModel &light_model)
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

void Model::create_shader_programs(CL_GraphicContext &gc)
{
	CL_String resource_dir = "Resources/Model/"; // CL_Directory::get_resourcedata(app_name);

	CL_VirtualDirectory vd;
	program_object_texture = CL_ProgramObject::load(gc, resource_dir + L"model_3ds_vertex.glsl", resource_dir + L"model_3ds_fragment.glsl", vd);
	program_object_texture.bind_attribute_location(0, "in_position");
	program_object_texture.bind_attribute_location(1, "in_normal");
	program_object_texture.bind_attribute_location(2, "in_texcoord");
	if (!program_object_texture.link())
		throw CL_Exception("Unable to link 3ds model program object (texture)");
	program_object_no_texture = CL_ProgramObject::load(gc, resource_dir + L"model_3ds_vertex.glsl", resource_dir + L"model_3ds_fragment_no_texture.glsl", vd);
	program_object_no_texture.bind_attribute_location(0, "in_position");
	program_object_no_texture.bind_attribute_location(1, "in_normal");
	if (!program_object_no_texture.link())
		throw CL_Exception("Unable to link 3ds model program object (no texture)");

	program_mirror_object_texture = CL_ProgramObject::load(gc, resource_dir + L"mirror_model_3ds_vertex.glsl", resource_dir + L"mirror_model_3ds_fragment.glsl", vd);
	program_mirror_object_texture.bind_attribute_location(0, "in_position");
	program_mirror_object_texture.bind_attribute_location(1, "in_normal");
	program_mirror_object_texture.bind_attribute_location(2, "in_texcoord");
	if (!program_mirror_object_texture.link())
		throw CL_Exception("Unable to link 3ds model program object (texture)");
	program_mirror_object_no_texture = CL_ProgramObject::load(gc, resource_dir + L"mirror_model_3ds_vertex.glsl", resource_dir + L"mirror_model_3ds_fragment_no_texture.glsl", vd);
	program_mirror_object_no_texture.bind_attribute_location(0, "in_position");
	program_mirror_object_no_texture.bind_attribute_location(1, "in_normal");
	if (!program_mirror_object_no_texture.link())
		throw CL_Exception("Unable to link 3ds model program object (no texture)");
}

void Model::load_model(CL_GraphicContext &gc, const CL_StringRef & filename)
{
	CL_String model_path = CL_PathHelp::get_fullpath(filename);

	CL_File file(filename, CL_File::open_existing, CL_File::access_read);
	CL_Lib3dsFile model_file;
	model_file.load(file);
	file.close();

	std::vector<CL_Lib3dsMesh> meshes = model_file.export_meshes();
	size_t total_vertices = 0;
	for (size_t i = 0; i < meshes.size(); i++)
	{
		for (size_t j = 0; j < meshes[i].face_materials.size(); j++)
			material_ranges[meshes[i].face_materials[j]].length += 3;
		total_vertices += meshes[i].positions.size();
	}

	vertices = CL_VertexArrayBuffer(gc, sizeof(Vertex)*total_vertices);
	vertices.lock(cl_access_write_only);

	Vertex *vertices_data = reinterpret_cast<Vertex *>(vertices.get_data());
	int offset = 0;

	std::map<int, MaterialRange>::iterator it;
	for (it = material_ranges.begin(); it != material_ranges.end(); ++it)
	{
		it->second.start = offset;
		for (size_t i = 0; i < meshes.size(); i++)
		{
			for (size_t j = 0; j < meshes[i].face_materials.size(); j++)
			{
				if (meshes[i].face_materials[j] == it->first)
				{
					for (int k = 0; k < 3; k++)
					{
						Vertex v;
						v.position = meshes[i].positions[j*3+k];
						v.normal = meshes[i].normals[j*3+k];
						v.texcoord0 = meshes[i].texcooords[j*3+k];
						vertices_data[offset+k] = v;
					}
					offset += 3;
				}
			}
		}

		if (it->first == -1)
		{
			// Default (null) material:
			Material material;
			material.ambient = CL_Vec4f(0.2f, 0.2f, 0.2f, 1.0f);
			material.diffuse = CL_Vec4f(0.8f, 0.8f, 0.8f, 1.0f);
			material.specular = CL_Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
			it->second.material = material;
		}
		else
		{
			Lib3dsMaterial *m = model_file->materials[it->first];

			if (wood_texture.is_null())
			{
				if (m->texture1_map.name[0])
				{
					if (textures.find(m->texture1_map.name) == textures.end())
					{
						CL_PixelBuffer image = load_image(model_path + CL_StringHelp::local8_to_text(m->texture1_map.name));
						textures[m->texture1_map.name] = CL_Texture(gc, image.get_size());
						//textures[m->texture1_map.name].set_generate_mipmap(true);
						textures[m->texture1_map.name].set_image(image);
						textures[m->texture1_map.name].set_wrap_mode(cl_wrap_repeat, cl_wrap_repeat);
						textures[m->texture1_map.name].set_min_filter(cl_filter_linear);
						textures[m->texture1_map.name].set_mag_filter(cl_filter_linear);
						//textures[m->texture1_map.name].set_min_filter(cl_filter_linear_mipmap_linear);
						//textures[m->texture1_map.name].set_mag_filter(cl_filter_linear_mipmap_linear);
						textures[m->texture1_map.name].set_max_anisotropy(8.0f);
					}
					it->second.texture = textures[m->texture1_map.name];
				}
			}

			// Should we also bind texture1_mask, texture2_map and texture2_mask?
			// In which situations are those textures used?
			// Textures can also be rotated and scaled on a material. How often is this done?

			Material material;
			material.ambient = CL_Vec4f(m->ambient[0], m->ambient[1], m->ambient[2], m->ambient[3]);
			material.diffuse = CL_Vec4f(m->diffuse[0], m->diffuse[1], m->diffuse[2], m->diffuse[3]);
			material.specular = CL_Vec4f(m->specular[0], m->specular[1], m->specular[2], m->specular[3]);
			float s = pow(2, 10.0f * m->shininess);
			if (s > 128.0f)
				s = 128.0f;
			material.shininess = s;
			it->second.material = material;
		}
	}

	vertices.unlock();
}

CL_PixelBuffer Model::load_image(const CL_String &filename)
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
