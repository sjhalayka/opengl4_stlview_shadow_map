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

	void calc_AABB_min_max_locations(void);

	bool read_triangles_from_binary_stereo_lithography_file(const char* const file_name);
	void scale_mesh(float max_extent);
	void rotate_and_translate_mesh(float yaw, float pitch, vec3 translate_vec);

	void draw(GLint render_shader_program, int win_x, int win_y);

	void draw_AABB(void);

	void set_transform(vec3 dir, float displacement_factor)
	{
		dir = normalize(dir);

		float yaw = 0.0f;

		if (fabsf(dir.x) < 0.00001 && fabsf(dir.z) < 0.00001)
			yaw = 0.0f;
		else
			yaw = atan2f(dir.x, dir.z);

		float pitch = -atan2f(dir.y, sqrt(dir.x * dir.x + dir.z * dir.z));

		static const mat4 identity_mat = mat4(1.0f);

		mat4 rot0_mat = rotate(identity_mat, yaw, vec3(0.0, 1.0, 0.0));
		mat4 rot1_mat = rotate(identity_mat, pitch, vec3(1.0, 0.0, 0.0));
		mat4 translate_mat = translate(identity_mat, dir * displacement_factor);

		model_mat = translate_mat * rot0_mat * rot1_mat;
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




