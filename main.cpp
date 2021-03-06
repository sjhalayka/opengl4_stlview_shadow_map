#include "main.h"
#include "mesh.h"


int main(int argc, char** argv)
{

	if (false == sphere_mesh.read_triangles_from_binary_stereo_lithography_file("sphere.stl"))
	{
		cout << "Error: Could not properly read file sphere.stl" << endl;
		return 2;
	}

	sphere_mesh.scale_mesh(sphere_scale); // radius == 0.5f;





	mesh game_piece_mesh;

	if (false == game_piece_mesh.read_triangles_from_binary_stereo_lithography_file("fractal.stl"))
	{
		cout << "Error: Could not properly read file fractal.stl" << endl;
		return 2;
	}

	game_piece_mesh.scale_mesh(game_piece_scale);

	for (size_t i = 0; i < 1; i++)
		player_game_piece_meshes.push_back(game_piece_mesh);

	for (size_t i = 0; i < player_game_piece_meshes.size(); i++)
	{
		vec3 dir = get_pseudorandom_unit_direction();
		vec3 dir2 = get_pseudorandom_unit_direction();

		vec3 left = normalize(cross(dir, dir2));
		vec3 tangent = cross(dir, left);

		player_game_piece_meshes[i].init_geodesic(dir, left, tangent, sphere_scale * 0.5f + game_piece_scale * 0.5f); // 1/2 + .25/2
	}




	if (false == game_piece_mesh.read_triangles_from_binary_stereo_lithography_file("classic.stl"))
	{
		cout << "Error: Could not properly read file fractal.stl" << endl;
		return 2;
	}

	game_piece_mesh.scale_mesh(game_piece_scale);


	for (size_t i = 0; i < 0; i++)
		enemy_game_piece_meshes.push_back(game_piece_mesh);

	for (size_t i = 0; i < enemy_game_piece_meshes.size(); i++)
	{
		vec3 dir = get_pseudorandom_unit_direction();
		vec3 dir2 = get_pseudorandom_unit_direction();

		vec3 left = normalize(cross(dir, dir2));
		vec3 tangent = cross(dir, left);

		enemy_game_piece_meshes[i].init_geodesic(dir, left, tangent, sphere_scale * 0.5f + game_piece_scale * 0.5f); // 1/2 + .25/2
	}



	glutInit(&argc, argv);

	if (false == init_opengl(win_x, win_y))
		return 1;



	glutReshapeFunc(reshape_func);
	glutIdleFunc(idle_func);
	glutDisplayFunc(display_func);
	glutKeyboardFunc(keyboard_func);
	glutMouseFunc(mouse_func);
	glutMotionFunc(motion_func);
	glutPassiveMotionFunc(passive_motion_func);
	//glutIgnoreKeyRepeat(1);
	glutMainLoop();
	glutDestroyWindow(win_id);

	return 0;
}



void idle_func(void)
{
	static std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float, std::milli> elapsed = end_time - start_time;

	float x = elapsed.count() / 1000.0f;
	x *= 0.25f;

	start_time = std::chrono::high_resolution_clock::now();

	//for (size_t i = 0; i < player_game_piece_meshes.size(); i++)
	//	player_game_piece_meshes[i].proceed_geodesic(x);

	//for (size_t i = 0; i < enemy_game_piece_meshes.size(); i++)
	//	enemy_game_piece_meshes[i].proceed_geodesic(x);

	glutPostRedisplay();
}

bool init_opengl(const int& width, const int& height)
{
	win_x = width;
	win_y = height;

	if (win_x < 1)
		win_x = 1;

	if (win_y < 1)
		win_y = 1;

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(win_x, win_y);
	win_id = glutCreateWindow("Binary Stereo Lithography file viewer");

	if (GLEW_OK != glewInit())
	{
		cout << "GLEW initialization error" << endl;
		return false;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);


	if (false == shadow_map.init("shadowmap.vs.glsl", "shadowmap.fs.glsl"))
	{
		cout << "Could not load shadowmap shader" << endl;
		return false;
	}

	glGenTextures(1, &depthTex);
	glBindTexture(GL_TEXTURE_2D, depthTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, static_cast<GLsizei>(shadowMapWidth), static_cast<GLsizei>(shadowMapHeight));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

	// Assign the depth buffer texture to texture channel 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthTex);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, depthTex, 0);

	GLenum drawBuffers[] = { GL_NONE };
	glDrawBuffers(1, drawBuffers);


	return true;
}

void reshape_func(int width, int height)
{
	win_x = width;
	win_y = height;

	if (win_x < 1)
		win_x = 1;

	if (win_y < 1)
		win_y = 1;

	glutSetWindow(win_id);
	glutReshapeWindow(win_x, win_y);
	glViewport(0, 0, win_x, win_y);

	main_camera.calculate_camera_matrices(win_x, win_y);
}






void display_func(void)
{
	static std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float, std::milli> elapsed = end_time - start_time;


	mat4 lightPV, shadowBias;
	Frustum lightFrustum;

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);


	vec3 player_colour(1, 0, 0);
	vec3 enemy_colour(0.75f, 0.75f, 0.75f);


	shadow_map.use_program();
	glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 0);

	GLuint programHandle = shadow_map.get_program();
	pass1Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "recordDepth");
	pass2Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "shadeWithShadow");

	shadowBias = mat4(
		vec4(0.5f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.5f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.5f, 0.0f),
		vec4(0.5f, 0.5f, 0.5f, 1.0f)
	);

	vec3 left = cross(normalize(main_camera.eye), normalize(main_camera.up));
	vec3 lightPos = normalize(main_camera.eye) + normalize(main_camera.up) * 2.0f + left * 2.0f;
	lightPos = normalize(lightPos) * 10.0f;

	lightFrustum.orient(lightPos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
	lightFrustum.setPerspective(45.0f, 1.0f, 1.0f, 25.0f);


	glUniform1i(glGetUniformLocation(shadow_map.get_program(), "shadow_map"), 0);

	vec3 piece_dir = normalize(player_game_piece_meshes[0].geodesic_dir); // normalize(centre);// = normalize(vec3(1, 1, 1));
	float dp_limit = -0.5f;
	int highlight_move_region = 0;

	glUniform3f(glGetUniformLocation(shadow_map.get_program(), "piece_dir"), piece_dir.x, piece_dir.y, piece_dir.z);
	glUniform1f(glGetUniformLocation(shadow_map.get_program(), "dp_limit"), dp_limit);
	glUniform1i(glGetUniformLocation(shadow_map.get_program(), "highlight_move_region"), 0);


	mat4 model = mat4(1.0f);
	mat4 view = lightFrustum.getViewMatrix();
	mat4 proj = lightFrustum.getProjectionMatrix();

	mat3 normal = mat3(vec3((lightFrustum.getViewMatrix() * model)[0]), vec3((lightFrustum.getViewMatrix() * model)[1]), vec3((lightFrustum.getViewMatrix() * model)[2]));
	lightPV = shadowBias * lightFrustum.getProjectionMatrix() * lightFrustum.getViewMatrix();
	mat4 shadow = lightPV * model;

	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ModelMatrix"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ViewMatrix"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ProjectionMatrix"), 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(shadow_map.get_program(), "NormalMatrix"), 1, GL_FALSE, &normal[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);

	vec4 lp = view * vec4(lightPos, 0.0f);
	glUniform4f(glGetUniformLocation(shadow_map.get_program(), "LightPosition"), lp.x, lp.y, lp.z, lp.w);

	vec4 lp_untransformed = vec4(lightPos, 0.0f);
	glUniform4f(glGetUniformLocation(shadow_map.get_program(), "LightPosition_Untransformed"), lp_untransformed.x, lp_untransformed.y, lp_untransformed.z, lp_untransformed.w);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, static_cast<GLsizei>(shadowMapWidth), static_cast<GLsizei>(shadowMapHeight));
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass1Index);
	glDisable(GL_CULL_FACE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.5f, 10.0f);





	glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), 1.0f, 1.0f, 1.0f);

	model = mat4(1.0f);
	normal = mat3(vec3((view * model)[0]), vec3((view * model)[1]), vec3((view * model)[2]));
	shadow = lightPV * model;
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ModelMatrix"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(shadow_map.get_program(), "NormalMatrix"), 1, GL_FALSE, &normal[0][0]);

	sphere_mesh.draw(shadow_map.get_program(), win_x, win_y);




	glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), player_colour.x, player_colour.y, player_colour.z);

	for (size_t i = 0; i < player_game_piece_meshes.size(); i++)
	{
		model = player_game_piece_meshes[i].model_mat;
		normal = mat3(vec3((lightFrustum.getViewMatrix() * model)[0]), vec3((lightFrustum.getViewMatrix() * model)[1]), vec3((lightFrustum.getViewMatrix() * model)[2]));
		shadow = lightPV * model;
		glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ModelMatrix"), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix3fv(glGetUniformLocation(shadow_map.get_program(), "NormalMatrix"), 1, GL_FALSE, &normal[0][0]);

		player_game_piece_meshes[i].draw(shadow_map.get_program(), win_x, win_y);

	}

	glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), enemy_colour.x, enemy_colour.y, enemy_colour.z);

	for (size_t i = 0; i < enemy_game_piece_meshes.size(); i++)
	{
		model = enemy_game_piece_meshes[i].model_mat;
		normal = mat3(vec3((lightFrustum.getViewMatrix() * model)[0]), vec3((lightFrustum.getViewMatrix() * model)[1]), vec3((lightFrustum.getViewMatrix() * model)[2]));
		shadow = lightPV * model;
		glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ModelMatrix"), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix3fv(glGetUniformLocation(shadow_map.get_program(), "NormalMatrix"), 1, GL_FALSE, &normal[0][0]);

		enemy_game_piece_meshes[i].draw(shadow_map.get_program(), win_x, win_y);
	}



	glFlush();


	// reset camera matrices

	if(false == screenshot_mode)
		main_camera.calculate_camera_matrices(win_x, win_y);

	model = mat4(1.0f);
	view = main_camera.view_mat;
	proj = main_camera.projection_mat;
	normal = mat3(vec3((view * model)[0]), vec3((view * model)[1]), vec3((view * model)[2]));
	lightPV = shadowBias * lightFrustum.getProjectionMatrix() * lightFrustum.getViewMatrix();
	shadow = lightPV * model;

	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ModelMatrix"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ViewMatrix"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ProjectionMatrix"), 1, GL_FALSE, &proj[0][0]);

	glUniformMatrix3fv(glGetUniformLocation(shadow_map.get_program(), "NormalMatrix"), 1, GL_FALSE, &normal[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);


	lp = view * vec4(lightPos, 0.0f);
	glUniform4f(glGetUniformLocation(shadow_map.get_program(), "LightPosition"), lp.x, lp.y, lp.z, lp.w);
	lp_untransformed = vec4(lightPos, 0.0f);
	glUniform4f(glGetUniformLocation(shadow_map.get_program(), "LightPosition_Untransformed"), lp_untransformed.x, lp_untransformed.y, lp_untransformed.z, lp_untransformed.w);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, win_x, win_y);
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass2Index);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), 1.0f, 1.0f, 1.0f);
	model = mat4(1.0f);
	normal = mat3(vec3((view * model)[0]), vec3((view * model)[1]), vec3((view * model)[2]));
	shadow = lightPV * model;

	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ModelMatrix"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(shadow_map.get_program(), "NormalMatrix"), 1, GL_FALSE, &normal[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);


	glUniform1i(glGetUniformLocation(shadow_map.get_program(), "highlight_move_region"), 1);

	sphere_mesh.draw(shadow_map.get_program(), win_x, win_y);

	glUniform1i(glGetUniformLocation(shadow_map.get_program(), "highlight_move_region"), 0);



	for (size_t i = 0; i < player_game_piece_meshes.size(); i++)
	{
		glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), player_colour.x, player_colour.y, player_colour.z);

		model = player_game_piece_meshes[i].model_mat;
		normal = mat3(vec3((view * model)[0]), vec3((view * model)[1]), vec3((view * model)[2]));
		shadow = lightPV * model;

		glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ModelMatrix"), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix3fv(glGetUniformLocation(shadow_map.get_program(), "NormalMatrix"), 1, GL_FALSE, &normal[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);

		player_game_piece_meshes[i].draw(shadow_map.get_program(), win_x, win_y);

		glCullFace(GL_FRONT);
		glPolygonMode(GL_BACK, GL_LINE);

		glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), 0, 0, 0);

		glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 1);
		player_game_piece_meshes[i].draw(shadow_map.get_program(), win_x, win_y);
		glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 0);

		glPolygonMode(GL_BACK, GL_FILL);
		glCullFace(GL_BACK);
	}


	for (size_t i = 0; i < enemy_game_piece_meshes.size(); i++)
	{
		glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), enemy_colour.x, enemy_colour.y, enemy_colour.z);

		model = enemy_game_piece_meshes[i].model_mat;
		normal = mat3(vec3((view * model)[0]), vec3((view * model)[1]), vec3((view * model)[2]));
		shadow = lightPV * model;

		glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ModelMatrix"), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix3fv(glGetUniformLocation(shadow_map.get_program(), "NormalMatrix"), 1, GL_FALSE, &normal[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);

		enemy_game_piece_meshes[i].draw(shadow_map.get_program(), win_x, win_y);

		glCullFace(GL_FRONT);
		glPolygonMode(GL_BACK, GL_LINE);

		glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), 0, 0, 0);

		glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 1);
		enemy_game_piece_meshes[i].draw(shadow_map.get_program(), win_x, win_y);// draw_AABB();
		glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 0);

		glPolygonMode(GL_BACK, GL_FILL);
		glCullFace(GL_BACK);
	}


	glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), 0.0f, 0.0f, 0.0f);
	model = mat4(1.0f);
	normal = mat3(vec3((view * model)[0]), vec3((view * model)[1]), vec3((view * model)[2]));
	shadow = lightPV * model;

	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ModelMatrix"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(shadow_map.get_program(), "NormalMatrix"), 1, GL_FALSE, &normal[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);

	glCullFace(GL_FRONT);
	glPolygonMode(GL_BACK, GL_LINE);

	glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), 0, 0, 0);

	glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 1);
	sphere_mesh.draw(shadow_map.get_program(), win_x, win_y);
	glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 0);

	glPolygonMode(GL_BACK, GL_FILL);
	glCullFace(GL_BACK);



	model = mat4(1.0f);
	normal = mat3(vec3((view * model)[0]), vec3((view * model)[1]), vec3((view * model)[2]));
	shadow = lightPV * model;

	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ModelMatrix"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(shadow_map.get_program(), "NormalMatrix"), 1, GL_FALSE, &normal[0][0]);		glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);

	glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 1);
	draw_axis(shadow_map.get_program());
	glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 0);

	float f = elapsed.count();
	f /= 1000;
	f *= glm::pi<float>();

	float s = sin(f);
	s += 1.0f;
	s *= 2.0f;

	if (col_loc == sphere)
	{
		sphere_mesh.draw_AABB();
	}
	else if (col_loc == player_game_piece)
	{
		vec3 outline_colour(0, 1 * s, 0);

		glDisable(GL_DEPTH_TEST);

		glPolygonMode(GL_FRONT, GL_LINES);

		GLubyte checkered_stipple_pattern[] =
		{
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc,
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc,
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc,
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc,
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc,
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc,
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc,
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc
		};

		glEnable(GL_POLYGON_STIPPLE);

		glPolygonStipple(checkered_stipple_pattern);


		model = player_game_piece_meshes[collision_location_index].model_mat;
		normal = mat3(vec3((view * model)[0]), vec3((view * model)[1]), vec3((view * model)[2]));
		shadow = lightPV * model;

		glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ModelMatrix"), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix3fv(glGetUniformLocation(shadow_map.get_program(), "NormalMatrix"), 1, GL_FALSE, &normal[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);

		player_game_piece_meshes[collision_location_index].draw(shadow_map.get_program(), win_x, win_y);

		glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), outline_colour.x, outline_colour.y, outline_colour.z);
		glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 1);
		player_game_piece_meshes[collision_location_index].draw(shadow_map.get_program(), win_x, win_y);// draw_AABB();
		glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 0);

		glDisable(GL_POLYGON_STIPPLE);


		glPolygonMode(GL_FRONT, GL_FILL);
		glEnable(GL_DEPTH_TEST);

		glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), player_colour.x, player_colour.y, player_colour.z);
		player_game_piece_meshes[collision_location_index].draw(shadow_map.get_program(), win_x, win_y);// draw_AABB();	

		// Draw outline code from NeHe lesson 37:
		// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=37
		glCullFace(GL_FRONT);
		glPolygonMode(GL_BACK, GL_LINE);

		glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), outline_colour.x, outline_colour.y, outline_colour.z);

		glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 1);
		player_game_piece_meshes[collision_location_index].draw(shadow_map.get_program(), win_x, win_y);// draw_AABB();
		glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 0);

		glPolygonMode(GL_BACK, GL_FILL);
		glCullFace(GL_BACK);


	}
	else if (col_loc == enemy_game_piece)
	{
		vec3 outline_colour(0, 0, 1 * s);

		glDisable(GL_DEPTH_TEST);

		glPolygonMode(GL_FRONT, GL_LINES);

		GLubyte checkered_stipple_pattern[] =
		{
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc,
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc,
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc,
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc,
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc,
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc,
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc,
			0x33,0x33,0x33,0x33,
			0x33,0x33,0x33,0x33,
			0xcc,0xcc,0xcc,0xcc,
			0xcc,0xcc,0xcc,0xcc
		};

		glEnable(GL_POLYGON_STIPPLE);

		glPolygonStipple(checkered_stipple_pattern);


		model = enemy_game_piece_meshes[collision_location_index].model_mat;
		normal = mat3(vec3((view * model)[0]), vec3((view * model)[1]), vec3((view * model)[2]));
		shadow = lightPV * model;

		glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ModelMatrix"), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix3fv(glGetUniformLocation(shadow_map.get_program(), "NormalMatrix"), 1, GL_FALSE, &normal[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);

		enemy_game_piece_meshes[collision_location_index].draw(shadow_map.get_program(), win_x, win_y);

		glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), outline_colour.x, outline_colour.y, outline_colour.z);
		glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 1);
		enemy_game_piece_meshes[collision_location_index].draw(shadow_map.get_program(), win_x, win_y);// draw_AABB();
		glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 0);

		glDisable(GL_POLYGON_STIPPLE);


		glPolygonMode(GL_FRONT, GL_FILL);
		glEnable(GL_DEPTH_TEST);

		glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), enemy_colour.x, enemy_colour.y, enemy_colour.z);
		enemy_game_piece_meshes[collision_location_index].draw(shadow_map.get_program(), win_x, win_y);// draw_AABB();	

		// Draw outline code from NeHe lesson 37:
		// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=37
		glCullFace(GL_FRONT);
		glPolygonMode(GL_BACK, GL_LINE);

		glUniform3f(glGetUniformLocation(shadow_map.get_program(), "MaterialKd"), outline_colour.x, outline_colour.y, outline_colour.z);

		glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 1);
		enemy_game_piece_meshes[collision_location_index].draw(shadow_map.get_program(), win_x, win_y);// draw_AABB();
		glUniform1i(glGetUniformLocation(shadow_map.get_program(), "flat_colour"), 0);

		glPolygonMode(GL_BACK, GL_FILL);
		glCullFace(GL_BACK);

	}



	/*glDisable(GL_DEPTH);
	glPointSize(4.0f);

	vec3 cl = collision_location;

	glBegin(GL_POINTS);

	glVertex3f(cl.x, cl.y, cl.z);

	glEnd();*/



	//for (size_t i = 0; i < player_game_piece_meshes.size(); i++)
	//	player_game_piece_meshes[i].draw_basis();

	//for (size_t i = 0; i < enemy_game_piece_meshes.size(); i++)
	//	enemy_game_piece_meshes[i].draw_basis();

	glFlush();

	//if(false == screenshot_mode)
		glutSwapBuffers();
}

void keyboard_func(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'a':
		take_screenshot(4, "out.tga");// , const bool reverse_rows = false)
		break;

	default:
		break;
	}
}

void mouse_func(int button, int state, int x, int y)
{
	if (GLUT_LEFT_BUTTON == button)
	{
		if (GLUT_DOWN == state)
		{
			ray = screen_coords_to_world_coords(x, y, win_x, win_y);

			float t = 0;

			bool first_assignment = true;
			mat4 collision_transform(1.0f);

			if (true == line_sphere_intersect(main_camera.eye, ray, vec3(0, 0, 0), 0.5f, t))
			{
				collision_location = main_camera.eye + ray * t;
				col_loc = sphere;
				first_assignment = false;
				collision_transform = mat4(1.0f);
			}

			for (size_t i = 0; i < player_game_piece_meshes.size(); i++)
			{
				mat4 inv = inverse(player_game_piece_meshes[i].model_mat);
				vec4 start = inv * vec4(main_camera.eye, 1.0);
				vec4 direction = inv * vec4(ray, 0.0);
				direction = normalize(direction);

				if (true == player_game_piece_meshes[i].intersect_AABB(start, direction))
				{
					vec3 closest_intersection_point;

					if (true == player_game_piece_meshes[i].intersect_triangles(start, direction, closest_intersection_point))
					{
						closest_intersection_point = player_game_piece_meshes[i].model_mat * vec4(closest_intersection_point, 1);

						if (first_assignment)
						{
							collision_location = closest_intersection_point;

							col_loc = player_game_piece;
							collision_location_index = i;

							first_assignment = false;
							collision_transform = inv;
						}
						else
						{
							vec3 c0 = vec3(main_camera.eye) - closest_intersection_point;
							vec3 c1 = vec3(main_camera.eye) - collision_location;

							if (length(c0) < length(c1))
							{
								collision_location = closest_intersection_point;

								col_loc = player_game_piece;
								collision_location_index = i;
								collision_transform = inv;
							}
						}
					}
				}
			}


			for (size_t i = 0; i < enemy_game_piece_meshes.size(); i++)
			{
				mat4 inv = inverse(enemy_game_piece_meshes[i].model_mat);
				vec4 start = inv * vec4(main_camera.eye, 1.0);
				vec4 direction = inv * vec4(ray, 0.0);
				direction = normalize(direction);

				if (true == enemy_game_piece_meshes[i].intersect_AABB(start, direction))
				{
					vec3 closest_intersection_point;

					if (true == enemy_game_piece_meshes[i].intersect_triangles(start, direction, closest_intersection_point))
					{
						closest_intersection_point = enemy_game_piece_meshes[i].model_mat * vec4(closest_intersection_point, 1);

						if (first_assignment)
						{
							collision_location = closest_intersection_point;

							col_loc = enemy_game_piece;
							collision_location_index = i;

							first_assignment = false;
							collision_transform = inv;
						}
						else
						{
							vec3 c0 = vec3(main_camera.eye) - closest_intersection_point;
							vec3 c1 = vec3(main_camera.eye) - collision_location;

							if (length(c0) < length(c1))
							{
								collision_location = closest_intersection_point;

								col_loc = enemy_game_piece;
								collision_location_index = i;
								collision_transform = inv;
							}
						}
					}
				}
			}

			collision_location = collision_transform * vec4(collision_location, 1);

			if (first_assignment)
			{
				collision_location = vec3(0, 0, 0);
				col_loc = background;
				collision_transform = mat4(1.0f);
			}

			lmb_down = true;
		}
		else
			lmb_down = false;
	}
	else if (GLUT_MIDDLE_BUTTON == button)
	{
		if (GLUT_DOWN == state)
			mmb_down = true;
		else
			mmb_down = false;
	}
	else if (GLUT_RIGHT_BUTTON == button)
	{
		if (GLUT_DOWN == state)
			rmb_down = true;
		else
			rmb_down = false;
	}
}

void motion_func(int x, int y)
{
	int prev_mouse_x = mouse_x;
	int prev_mouse_y = mouse_y;

	mouse_x = x;
	mouse_y = y;

	int mouse_delta_x = mouse_x - prev_mouse_x;
	int mouse_delta_y = prev_mouse_y - mouse_y;

	if (true == lmb_down && (0 != mouse_delta_x || 0 != mouse_delta_y))
	{
		main_camera.u -= static_cast<float>(mouse_delta_y) * u_spacer;
		main_camera.v += static_cast<float>(mouse_delta_x) * v_spacer;

		main_camera.calculate_camera_matrices(win_x, win_y);
	}
	else if (true == rmb_down && (0 != mouse_delta_y))
	{
		main_camera.w -= static_cast<float>(mouse_delta_y) * w_spacer;

		if (main_camera.w < 2.0f)
			main_camera.w = 2.0f;
		else if (main_camera.w > 20.0f)
			main_camera.w = 20.0f;

		main_camera.calculate_camera_matrices(win_x, win_y);
	}
}

void passive_motion_func(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
}




