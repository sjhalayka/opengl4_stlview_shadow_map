#ifndef MAIN_H
#define MAIN_H

#include "frustum.h"
#include "uv_camera.h"
#include "primitives.h"
#include "mesh.h"
#include "vertex_fragment_shader.h"
#include "vertex_geometry_fragment_shader.h"

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
using namespace glm;

#include <cstdlib>
#include <GL/glew.h>
#include <GL/glut.h>
#pragma comment(lib, "glew32")

#include <ctime>
using std::time;

#include <random>
using std::mt19937;

#include <vector>
using std::vector;

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;
using std::istringstream;


void idle_func(void);
bool init_opengl(const int &width, const int &height);
void reshape_func(int width, int height);
void display_func(void);
void keyboard_func(unsigned char key, int x, int y);
void mouse_func(int button, int state, int x, int y);
void motion_func(int x, int y);
void passive_motion_func(int x, int y);


mt19937 mt_rand(static_cast<unsigned int>(time(0)));

vector<mesh> player_game_piece_meshes; 
vector<mesh> enemy_game_piece_meshes;

mesh sphere_mesh;

enum possible_collision_locations { player_game_piece, enemy_game_piece, sphere, background };
possible_collision_locations col_loc = background;
size_t collision_location_index = 0;

vector<triangle> triangles;
vector<vec3> face_normals;
vector<vec3> vertices;
vector<vec3> vertex_normals;



vertex_fragment_shader shadow_map;



uv_camera main_camera;

GLint win_id = 0;
GLint win_x = 800, win_y = 600;

float u_spacer = 0.01f;
float v_spacer = 0.5f*u_spacer;
float w_spacer = 0.1f;

bool lmb_down = false;
bool mmb_down = false;
bool rmb_down = false;
int mouse_x = 0;
int mouse_y = 0;

vec3 ray;
vec3 collision_location;

GLuint      render_fbo = 0;
GLuint      fbo_textures[3] = { 0, 0, 0 };
GLuint      quad_vao = 0;
GLuint      points_buffer = 0;







vec3 screen_coords_to_world_coords(const int x, const int y, const int screen_width, const int screen_height)
{
	const float half_screen_width = screen_width / 2.0f;
	const float half_screen_height = screen_height / 2.0f;

	mat4 inv_mat = inverse(main_camera.projection_mat * main_camera.view_mat);

	vec4 n((x - half_screen_width) / half_screen_width, -1 * (y - half_screen_height) / half_screen_height, -1, 1.0);
	vec4 f((x - half_screen_width) / half_screen_width, -1 * (y - half_screen_height) / half_screen_height, 1, 1.0);

	vec4 near_result = inv_mat * n;
	vec4 far_result = inv_mat * f;

	near_result /= near_result.w;
	far_result /= far_result.w;

	vec3 dir = vec3(far_result - near_result);

	vec3 ret_dir(dir.x, dir.y, dir.z);
	ret_dir = normalize(ret_dir);

	return ret_dir;
}


// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
bool line_sphere_intersect(const vec3 orig, const vec3 dir, const vec3 center, const float radius, float& t)
{
	float t0, t1; // solutions for t if the ray intersects 

	vec3 L = center - orig;
	float tca = dot(L, dir);

	if (tca < 0)
		return false;
	
	float d2 = dot(L, L) - tca * tca;

	float radius2 = radius * radius;

	if (d2 > radius2) 
		return false;

	float thc = sqrt(radius2 - d2);
	t0 = tca - thc;
	t1 = tca + thc;

	if (t0 > t1) std::swap(t0, t1);

	if (t0 < 0) 
	{
		t0 = t1; // if t0 is negative, let's use t1 instead 
		
		if (t0 < 0)
			return false; // both t0 and t1 are negative 
	}

	t = t0;

	return true;
}






#endif