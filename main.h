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

#include <chrono>

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


mt19937 mt_rand(0);// static_cast<unsigned int>(time(0)));

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


float sphere_scale = 1.0f;
float game_piece_scale = 0.25f;



GLfloat border[] = { 1.0f, 0.0f,0.0f,0.0f };
// The depth buffer texture
GLuint depthTex;

size_t shadowMapWidth = 8192;
size_t shadowMapHeight = 8192;

GLuint shadowFBO, pass1Index, pass2Index;



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

bool screenshot_mode = false;



void take_screenshot(size_t num_cams_wide, const char* filename, const bool reverse_rows = false)
{
	screenshot_mode = true;

	// Set up Targa TGA image data.
	unsigned char  idlength = 0;
	unsigned char  colourmaptype = 0;
	unsigned char  datatypecode = 2;
	unsigned short int colourmaporigin = 0;
	unsigned short int colourmaplength = 0;
	unsigned char  colourmapdepth = 0;
	unsigned short int x_origin = 0;
	unsigned short int y_origin = 0;

	cout << "Image size: " << static_cast<size_t>(win_x) * num_cams_wide << "x" << static_cast<size_t>(win_y) * num_cams_wide << " pixels" << endl;

	if (static_cast<size_t>(win_x) * num_cams_wide > static_cast<unsigned short>(-1) ||
		static_cast<size_t>(win_y) * num_cams_wide > static_cast<unsigned short>(-1))
	{
		cout << "Image too large. Maximum width and height is " << static_cast<unsigned short>(-1) << endl;
		return;
	}

	unsigned short int px = win_x * static_cast<unsigned short>(num_cams_wide);
	unsigned short int py = win_y * static_cast<unsigned short>(num_cams_wide);
	unsigned char  bitsperpixel = 24;
	unsigned char  imagedescriptor = 0;
	vector<char> idstring;

	size_t num_bytes = 3 * px * py;
	vector<unsigned char> pixel_data(num_bytes);

	vector<unsigned char> fbpixels(3 * win_x * win_y);

	const size_t total_cams = num_cams_wide * num_cams_wide;
	size_t cam_count = 0;
	// Loop through subcameras.
	for (size_t cam_num_x = 0; cam_num_x < num_cams_wide; cam_num_x++)
	{
		for (size_t cam_num_y = 0; cam_num_y < num_cams_wide; cam_num_y++)
		{
			cout << "Camera: " << cam_count + 1 << " of " << total_cams << endl;

			// Set up camera, draw, then copy the frame buffer.
			main_camera.Set_Large_Screenshot(num_cams_wide, cam_num_x, cam_num_y, win_x, win_y);



			display_func();
			glReadPixels(0, 0, win_x, win_y, GL_RGB, GL_UNSIGNED_BYTE, &fbpixels[0]);

			// Copy pixels to large image.
			for (GLint i = 0; i < win_x; i++)
			{
				for (GLint j = 0; j < win_y; j++)
				{
					size_t fb_index = 3 * (j * win_x + i);

					size_t screenshot_x = cam_num_x * win_x + i;
					size_t screenshot_y = cam_num_y * win_y + j;
					size_t screenshot_index = 3 * (screenshot_y * (win_x * num_cams_wide) + screenshot_x);

					pixel_data[screenshot_index] = fbpixels[fb_index + 2];
					pixel_data[screenshot_index + 1] = fbpixels[fb_index + 1];
					pixel_data[screenshot_index + 2] = fbpixels[fb_index];
				}
			}

			cam_count++;
		}

	}

	screenshot_mode = false;

	main_camera.calculate_camera_matrices(win_x, win_y);

	// Write Targa TGA file to disk.
	ofstream out(filename, ios::binary);

	if (!out.is_open())
	{
		cout << "Failed to open TGA file for writing: " << filename << endl;
		return;
	}

	out.write(reinterpret_cast<char*>(&idlength), 1);
	out.write(reinterpret_cast<char*>(&colourmaptype), 1);
	out.write(reinterpret_cast<char*>(&datatypecode), 1);
	out.write(reinterpret_cast<char*>(&colourmaporigin), 2);
	out.write(reinterpret_cast<char*>(&colourmaplength), 2);
	out.write(reinterpret_cast<char*>(&colourmapdepth), 1);
	out.write(reinterpret_cast<char*>(&x_origin), 2);
	out.write(reinterpret_cast<char*>(&y_origin), 2);
	out.write(reinterpret_cast<char*>(&px), 2);
	out.write(reinterpret_cast<char*>(&py), 2);
	out.write(reinterpret_cast<char*>(&bitsperpixel), 1);
	out.write(reinterpret_cast<char*>(&imagedescriptor), 1);

	out.write(reinterpret_cast<char*>(&pixel_data[0]), num_bytes);
}

vec3 get_pseudorandom_unit_direction(void)
{
	float x = static_cast<float>(mt_rand()) / static_cast<float>(static_cast<long unsigned int>(-1));
	float y = static_cast<float>(mt_rand()) / static_cast<float>(static_cast<long unsigned int>(-1));
	float z = static_cast<float>(mt_rand()) / static_cast<float>(static_cast<long unsigned int>(-1));

	x *= 2.0f;
	x -= 1.0f;
	y *= 2.0f;
	y -= 1.0f;
	z *= 2.0f;
	z -= 1.0f;

	vec3 dir(x, y, z);
	dir = normalize(dir);

	return dir;
}

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



void draw_axis(GLuint program)
{
	GLuint components_per_vertex = 6;
	const GLuint components_per_normal = 3;
	GLuint components_per_position = 3;

	GLuint axis_buffer;

	glGenBuffers(1, &axis_buffer);

	vector<GLfloat> flat_data;

	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(1);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);

	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);

	GLuint num_vertices = static_cast<GLuint>(flat_data.size()) / components_per_vertex;

	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
	glBufferData(GL_ARRAY_BUFFER, flat_data.size() * sizeof(GLfloat), &flat_data[0], GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(glGetAttribLocation(program, "position"));
	glVertexAttribPointer(glGetAttribLocation(program, "position"),
		components_per_position,
		GL_FLOAT,
		GL_FALSE,
		components_per_vertex * sizeof(GLfloat),
		NULL);

	glEnableVertexAttribArray(glGetAttribLocation(program, "normal"));
	glVertexAttribPointer(glGetAttribLocation(program, "normal"),
		components_per_normal,
		GL_FLOAT,
		GL_TRUE,
		components_per_vertex * sizeof(GLfloat),
		(const GLvoid*)(components_per_position * sizeof(GLfloat)));

	glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), 0.0, 0.0, 1.0f);
	glDrawArrays(GL_LINES, 0, num_vertices);




	flat_data.clear();

	flat_data.push_back(0);
	flat_data.push_back(1);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);

	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);

	num_vertices = static_cast<GLuint>(flat_data.size()) / components_per_vertex;

	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
	glBufferData(GL_ARRAY_BUFFER, flat_data.size() * sizeof(GLfloat), &flat_data[0], GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(glGetAttribLocation(program, "position"));
	glVertexAttribPointer(glGetAttribLocation(program, "position"),
		components_per_position,
		GL_FLOAT,
		GL_FALSE,
		components_per_vertex * sizeof(GLfloat),
		NULL);

	glEnableVertexAttribArray(glGetAttribLocation(program, "normal"));
	glVertexAttribPointer(glGetAttribLocation(program, "normal"),
		components_per_normal,
		GL_FLOAT,
		GL_TRUE,
		components_per_vertex * sizeof(GLfloat),
		(const GLvoid*)(components_per_position * sizeof(GLfloat)));

	glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), 0.0, 1.0, 0.0f);
	glDrawArrays(GL_LINES, 0, num_vertices);



	flat_data.clear();

	flat_data.push_back(1);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);

	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);
	flat_data.push_back(0);

	num_vertices = static_cast<GLuint>(flat_data.size()) / components_per_vertex;

	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
	glBufferData(GL_ARRAY_BUFFER, flat_data.size() * sizeof(GLfloat), &flat_data[0], GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(glGetAttribLocation(program, "position"));
	glVertexAttribPointer(glGetAttribLocation(program, "position"),
		components_per_position,
		GL_FLOAT,
		GL_FALSE,
		components_per_vertex * sizeof(GLfloat),
		NULL);

	glEnableVertexAttribArray(glGetAttribLocation(program, "normal"));
	glVertexAttribPointer(glGetAttribLocation(program, "normal"),
		components_per_normal,
		GL_FLOAT,
		GL_TRUE,
		components_per_vertex * sizeof(GLfloat),
		(const GLvoid*)(components_per_position * sizeof(GLfloat)));

	glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), 1.0, 0.0, 0.0f);
	glDrawArrays(GL_LINES, 0, num_vertices);

	glDeleteBuffers(1, &axis_buffer);
}

#endif

