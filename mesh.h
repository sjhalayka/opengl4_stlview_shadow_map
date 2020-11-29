#pragma once

#include "primitives.h"
#include "uv_camera.h"

#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4

#include <glm/gtx/euler_angles.hpp>

using namespace glm;

#include <cstdlib>
#include <GL/glew.h>
#include <GL/glut.h>

#include <iostream>
using std::cout;
using std::endl;

#include <algorithm>
using std::swap;

#include <fstream>
using std::ifstream;
using std::ofstream;

#include <ios>
using std::ios_base;
using std::ios;

#include <set>
using std::set;

#include <vector>
using std::vector;

#include <limits>
using std::numeric_limits;

#include <cstring> // for memcpy()

class mesh
{
public:
	vector<triangle> triangles;
	vector<vec3> face_normals;
	vector<vec3> vertices;
	vector<vec3> vertex_normals;
	vector<float> opengl_vertex_data;

	mat4 model_mat;

	vec3 min_location, max_location;

	void draw_tangent(void)
	{
		vec3 centre = model_mat * vec4((max_location + min_location) / 2.0f, 1.0f);
		vec3 tangent = geodesic_tangent;

		glBegin(GL_LINES);

		glVertex3f(centre.x, centre.y, centre.z);
		glVertex3f(centre.x + tangent.x, centre.y + tangent.y, centre.z + tangent.z);

		glEnd();
	}

	void calc_AABB_min_max_locations(void);

	bool read_triangles_from_binary_stereo_lithography_file(const char* const file_name);
	void scale_mesh(float max_extent);

	void draw(GLint render_shader_program, int win_x, int win_y);

	void draw_AABB(void);

	vec3 geodesic_dir;
	vec3 geodesic_left;
	vec3 geodesic_tangent;
	float displacement;

	void init_geodesic(vec3 dir, vec3 left, vec3 tangent, float displacement_factor)
	{
		geodesic_dir = dir;
		geodesic_left = left;
		geodesic_tangent = tangent;
		displacement = displacement_factor;

		set_transform();
	}

	void proceed_geodesic(float elapsed_time)
	{
		geodesic_dir = geodesic_dir * cos(elapsed_time) + geodesic_tangent * sin(elapsed_time);
		geodesic_tangent = normalize(cross(geodesic_dir, geodesic_left));

		set_transform();
	}


	void set_transform(void)
	{
		const vec3 n_forward = normalize(geodesic_tangent);
		const vec3 n_up = normalize(geodesic_dir);
		const vec3 n_left = normalize(geodesic_left);

		model_mat[0] = normalize(vec4(n_left, 0.0f));
		model_mat[1] = normalize(vec4(n_forward, 0.0f));
		model_mat[2] = normalize(vec4(n_up, 0.0f));
		model_mat[3] = vec4(n_up * displacement, 1.0f);
	}


	bool intersect_triangles(vec3 ray_origin, vec3 ray_dir, vec3& closest_intersection_point)
	{
		bool global_found_intersection = false;
		bool first_assignment = true;

		for (size_t i = 0; i < triangles.size(); i++)
		{
			vec3 v0;
			v0.x = triangles[i].vertex[0].x;
			v0.y = triangles[i].vertex[0].y;
			v0.z = triangles[i].vertex[0].z;

			vec3 v1;
			v1.x = triangles[i].vertex[1].x;
			v1.y = triangles[i].vertex[1].y;
			v1.z = triangles[i].vertex[1].z;

			vec3 v2;
			v2.x = triangles[i].vertex[2].x;
			v2.y = triangles[i].vertex[2].y;
			v2.z = triangles[i].vertex[2].z;

			vec3 closest;
			bool found_intersection = RayIntersectsTriangle(ray_origin,
				ray_dir,
				v0, v1, v2,
				closest);

			if (true == found_intersection)
			{
				global_found_intersection = true;

				if (first_assignment)
				{
					closest_intersection_point = closest;
					first_assignment = false;
				}
				else
				{
					vec3 c0 = ray_origin - closest;
					vec3 c1 = ray_origin - closest_intersection_point;

					if (length(c0) < length(c1))
						closest_intersection_point = closest;
				}
			}
		}

		return global_found_intersection;
	}

	bool RayIntersectsTriangle(const vec3 rayOrigin,
		const vec3 rayVector,
		const vec3 v0, const vec3 v1, const vec3 v2,
		vec3& outIntersectionPoint)
	{
		const float EPSILON = 0.0000001f;
		vec3 vertex0 = v0;// inTriangle->vertex0;
		vec3 vertex1 = v1;// inTriangle->vertex1;
		vec3 vertex2 = v2;// inTriangle->vertex2;
		vec3 edge1, edge2, h, s, q;
		float a, f, u, v;
		edge1 = vertex1 - vertex0;
		edge2 = vertex2 - vertex0;
		h = cross(rayVector, edge2);
		a = dot(edge1, h);
		if (a > -EPSILON && a < EPSILON)
			return false;    // This ray is parallel to this triangle.
		f = 1.0f / a;
		s = rayOrigin - vertex0;
		u = f * dot(s, h);
		if (u < 0.0f || u > 1.0f)
			return false;
		q = cross(s, edge1);
		v = f * dot(rayVector, q);
		if (v < 0.0f || u + v > 1.0f)
			return false;

		// At this stage we can compute t to find out where the intersection point is on the line.

		float t = f * dot(edge2, q);

		if (t > EPSILON) // ray intersection
		{
			outIntersectionPoint = rayOrigin + rayVector * t;
			return true;
		}
		else // This means that there is a line intersection but not a ray intersection.
			return false;
	}

	// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
	bool intersect_AABB(vec3 ray_origin, vec3 ray_dir)
	{
		float tmin = (min_location.x - ray_origin.x) / ray_dir.x;
		float tmax = (max_location.x - ray_origin.x) / ray_dir.x;

		if (tmin > tmax) swap(tmin, tmax);

		float tymin = (min_location.y - ray_origin.y) / ray_dir.y;
		float tymax = (max_location.y - ray_origin.y) / ray_dir.y;

		if (tymin > tymax) swap(tymin, tymax);

		if ((tmin > tymax) || (tymin > tmax))
			return false;

		if (tymin > tmin)
			tmin = tymin;

		if (tymax < tmax)
			tmax = tymax;

		float tzmin = (min_location.z - ray_origin.z) / ray_dir.z;
		float tzmax = (max_location.z - ray_origin.z) / ray_dir.z;

		if (tzmin > tzmax) swap(tzmin, tzmax);

		if ((tmin > tzmax) || (tzmin > tmax))
			return false;

		if (tzmin > tmin)
			tmin = tzmin;

		if (tzmax < tmax)
			tmax = tzmax;

		return true;
	}


protected:
	void get_vertices_and_normals_from_triangles();
	void init_opengl_data(void);
};




