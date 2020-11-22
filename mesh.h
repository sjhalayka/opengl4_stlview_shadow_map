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

	// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
	bool intersect_AABB(vec3 ray_origin, vec3 ray_dir)
	{
		float tmin = (min_location .x - ray_origin.x) / ray_dir.x;
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




