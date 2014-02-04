
#include "precomp.h"
#include "sphere.h"

void Sphere::generate_octahedron()
{
	vertices.clear();
	triangles.clear();

	vertices.push_back(CL_Vec3f(1.0f, 0.0f, 0.0f));  // xplus
	vertices.push_back(CL_Vec3f(-1.0f, 0.0f, 0.0f)); // xminus
	vertices.push_back(CL_Vec3f(0.0f, 1.0f, 0.0f));  // yplus
	vertices.push_back(CL_Vec3f(0.0f, -1.0f, 0.0f)); // yminus
	vertices.push_back(CL_Vec3f(0.0f, 0.0f, 1.0f));  // zplus
	vertices.push_back(CL_Vec3f(0.0f, 0.0f, -1.0f)); // zminus

	triangles.push_back(Triangle(xplus, zplus, yplus));
	triangles.push_back(Triangle(yplus, zplus, xminus));
	triangles.push_back(Triangle(xminus, zplus, yminus));
    triangles.push_back(Triangle(yminus, zplus, xplus));
	triangles.push_back(Triangle(xplus, yplus, zminus));
    triangles.push_back(Triangle(yplus, xminus, zminus));
	triangles.push_back(Triangle(xminus, yminus, zminus));
	triangles.push_back(Triangle(yminus, xplus, zminus));
}

void Sphere::subdivide()
{
	/* Subdivide each polygon in the old approximation and normalize
	 *  the new points thus generated to lie on the surface of the unit
	 *  sphere.
	 * Each input triangle with vertices labelled [0,1,2] as shown
	 *  below will be turned into four new triangles:
	 *
	 *			Make new points
	 *			    a = (0+2)/2
	 *			    b = (0+1)/2
	 *			    c = (1+2)/2
	 *        1
	 *       /\		Normalize a, b, c
	 *      /  \
	 *    b/____\ c		Construct new triangles
	 *    /\    /\		    [0,b,a]
	 *   /	\  /  \		    [b,1,c]
	 *  /____\/____\	    [a,b,c]
	 * 0	  a     2	    [a,c,2]
	 */

	std::vector<MidpointVertex> new_vertices;
	std::vector<Triangle> new_triangles;

	size_t num_vertices = vertices.size();
	for (size_t i = 0; i < triangles.size(); i++)
	{
		size_t a = create_midpoint_vertex(new_vertices, triangles[i].vertex[0], triangles[i].vertex[2]);
		size_t b = create_midpoint_vertex(new_vertices, triangles[i].vertex[0], triangles[i].vertex[1]);
		size_t c = create_midpoint_vertex(new_vertices, triangles[i].vertex[1], triangles[i].vertex[2]);

		new_triangles.push_back(Triangle(triangles[i].vertex[0], b, a));
		new_triangles.push_back(Triangle(b, triangles[i].vertex[1], c));
		new_triangles.push_back(Triangle(a, b, c));
		new_triangles.push_back(Triangle(a, c, triangles[i].vertex[2]));
	}

	vertices.reserve(vertices.size()+new_vertices.size());
	for (size_t i = 0; i < new_vertices.size(); i++)
		vertices.push_back(new_vertices[i].pos);
	triangles = new_triangles;
}

size_t Sphere::create_midpoint_vertex(std::vector<MidpointVertex> &new_vertices, size_t v1, size_t v2)
{
	MidpointVertex v;
	if (v1 < v2)
	{
		v.v1 = v1;
		v.v2 = v2;
	}
	else
	{
		v.v1 = v2;
		v.v2 = v1;
	}
	for (size_t i = 0; i < new_vertices.size(); i++)
	{
		if (new_vertices[i].v1 == v.v1 && new_vertices[i].v2 == v.v2)
			return vertices.size()+i;
	}

	v.pos = (vertices[v.v1]+vertices[v.v2])/2.0f;
	v.pos.normalize();

	new_vertices.push_back(v);
	return vertices.size()+new_vertices.size()-1;
}
