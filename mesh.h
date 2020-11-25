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

	void draw(GLint render_shader_program,
		int win_x,
		int win_y
		//uv_camera& main_camera,
		//GLint proj_matrix_uniform_location,
		//GLint view_matrix_uniform_location,
		//GLint use_specular_uniform_location,
		//bool use_specular
	);

	void draw_AABB(void);


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



	bool intersect_OBB(
		glm::vec3 ray_origin,        // Ray origin, in world space
		glm::vec3 ray_direction     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
	) {

		// Intersection method from Real-Time Rendering and Essential Mathematics for Games

		float tMin = 0.0f;
		float tMax = 100000.0f;

		glm::vec3 OBBposition_worldspace(model_mat[3].x, model_mat[3].y, model_mat[3].z);

		glm::vec3 delta = OBBposition_worldspace - ray_origin;

		// Test intersection with the 2 planes perpendicular to the OBB's X axis
		{
			glm::vec3 xaxis(model_mat[0].x, model_mat[0].y, model_mat[0].z);
			float e = glm::dot(xaxis, delta);
			float f = glm::dot(ray_direction, xaxis);

			if (fabs(f) > 0.001f) { // Standard case

				float t1 = (e + min_location.x) / f; // Intersection with the "left" plane
				float t2 = (e + max_location.x) / f; // Intersection with the "right" plane
				// t1 and t2 now contain distances betwen ray origin and ray-plane intersections

				// We want t1 to represent the nearest intersection, 
				// so if it's not the case, invert t1 and t2
				if (t1 > t2) {
					float w = t1; t1 = t2; t2 = w; // swap t1 and t2
				}

				// tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
				if (t2 < tMax)
					tMax = t2;
				// tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
				if (t1 > tMin)
					tMin = t1;

				// And here's the trick :
				// If "far" is closer than "near", then there is NO intersection.
				// See the images in the tutorials for the visual explanation.
				if (tMax < tMin)
					return false;

			}
			else { // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
				if (-e + min_location.x > 0.0f || -e + max_location.x < 0.0f)
					return false;
			}
		}


		// Test intersection with the 2 planes perpendicular to the OBB's Y axis
		// Exactly the same thing than above.
		{
			glm::vec3 yaxis(model_mat[1].x, model_mat[1].y, model_mat[1].z);
			float e = glm::dot(yaxis, delta);
			float f = glm::dot(ray_direction, yaxis);

			if (fabs(f) > 0.001f) {

				float t1 = (e + min_location.y) / f;
				float t2 = (e + max_location.y) / f;

				if (t1 > t2) { float w = t1; t1 = t2; t2 = w; }

				if (t2 < tMax)
					tMax = t2;
				if (t1 > tMin)
					tMin = t1;
				if (tMin > tMax)
					return false;

			}
			else {
				if (-e + min_location.y > 0.0f || -e + max_location.y < 0.0f)
					return false;
			}
		}


		// Test intersection with the 2 planes perpendicular to the OBB's Z axis
		// Exactly the same thing than above.
		{
			glm::vec3 zaxis(model_mat[2].x, model_mat[2].y, model_mat[2].z);
			float e = glm::dot(zaxis, delta);
			float f = glm::dot(ray_direction, zaxis);

			if (fabs(f) > 0.001f) {

				float t1 = (e + min_location.z) / f;
				float t2 = (e + max_location.z) / f;

				if (t1 > t2) { float w = t1; t1 = t2; t2 = w; }

				if (t2 < tMax)
					tMax = t2;
				if (t1 > tMin)
					tMin = t1;
				if (tMin > tMax)
					return false;

			}
			else {
				if (-e +  min_location.z > 0.0f || -e + max_location.z < 0.0f)
					return false;
			}
		}

		//intersection_distance = tMin;
		return true;

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




