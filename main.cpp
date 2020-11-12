#include "main.h"
#include "mesh.h"


int main(int argc, char **argv)
{
	if(false == sphere_mesh.read_triangles_from_binary_stereo_lithography_file("sphere.stl"))
	{
		cout << "Error: Could not properly read file sphere.stl" << endl;
		return 2;
	}
	
	sphere_mesh.scale_mesh(2.0f);

	if (false == game_piece_mesh.read_triangles_from_binary_stereo_lithography_file("fractal.stl"))
	{
		cout << "Error: Could not properly read file fractal.stl" << endl;
		return 2;
	}

	game_piece_mesh.scale_mesh(0.5f);	

	vec3 dir(0, 0, 1);
	dir = normalize(dir);

	float yaw = 0.0f;

	if (fabsf(dir.x) < 0.00001 && fabsf(dir.z) < 0.00001)
		yaw = 0.0f;
	else
		yaw = atan2f(dir.x, dir.z);

	float pitch = -atan2f(dir.y, sqrt(dir.x * dir.x + dir.z * dir.z));

	game_piece_mesh.rotate_and_translate_mesh(yaw, pitch, dir);


	
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
	glutPostRedisplay();
}

bool init_opengl(const int &width, const int &height)
{
	win_x = width;
	win_y = height;

	if(win_x < 1)
		win_x = 1;

	if(win_y < 1)
		win_y = 1;

	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
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








	return true;
}

void reshape_func(int width, int height)
{
	win_x = width;
	win_y = height;

	if(win_x < 1)
		win_x = 1;

	if(win_y < 1)
		win_y = 1;

	glutSetWindow(win_id);
	glutReshapeWindow(win_x, win_y);
	glViewport(0, 0, win_x, win_y);

	main_camera.calculate_camera_matrices(win_x, win_y);
}



void draw_meshes(GLint render_shader_program)
{
	sphere_mesh.draw(render_shader_program, win_x, win_y);

	game_piece_mesh.draw(render_shader_program, win_x, win_y);
}


void display_func(void)
{
	Frustum lightFrustum;

	GLuint shadowFBO, pass1Index, pass2Index;

	int shadowMapWidth = 8192;
	int shadowMapHeight = 8192;

	mat4 lightPV, shadowBias;

	GLfloat border[] = { 1.0f, 0.0f,0.0f,0.0f };
	// The depth buffer texture
	GLuint depthTex;
	glGenTextures(1, &depthTex);
	glBindTexture(GL_TEXTURE_2D, depthTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, shadowMapWidth, shadowMapHeight);
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

	// Create and set up the FBO
	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, depthTex, 0);

	GLenum drawBuffers[] = { GL_NONE };
	glDrawBuffers(1, drawBuffers);

	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (result == GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer is complete.\n");
	}
	else {
		printf("Framebuffer is not complete.\n");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);


	GLuint programHandle = shadow_map.get_program();
	pass1Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "recordDepth");
	pass2Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "shadeWithShadow");

	shadowBias = mat4(vec4(0.5f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.5f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.5f, 0.0f),
		vec4(0.5f, 0.5f, 0.5f, 1.0f)
	);

	float c = 1.65f;
	vec3 lightPos = vec3(10.0f, 10.0f, 10.0f);  // World coord

	lightFrustum.orient(lightPos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
	lightFrustum.setPerspective(45.0f, 1.0f, 1.0f, 25.0f);

	shadow_map.use_program();
	glUniform1i(glGetUniformLocation(shadow_map.get_program(), "shadow_map"), 0);


	mat4 model(1.0f);
	mat4 mv = model * lightFrustum.getViewMatrix();
	mat3 normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));
	mat4 mvp = lightFrustum.getProjectionMatrix() * mv;
	lightPV = shadowBias * lightFrustum.getProjectionMatrix() * lightFrustum.getViewMatrix();
	mat4 shadow = lightPV * model;
	mat4 view = lightFrustum.getViewMatrix();

	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ModelViewMatrix"), 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(shadow_map.get_program(), "NormalMatrix"), 1, GL_FALSE, &normal[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "MVP"), 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ViewMatrix"), 1, GL_FALSE, &view[0][0]);

	vec4 lp = view * vec4(lightPos, 1.0f);
	glUniform4f(glGetUniformLocation(shadow_map.get_program(), "LightPosition"), lp.x, lp.y, lp.z, lp.w);



	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, shadowMapWidth, shadowMapHeight);
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass1Index);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.5f, 10.0f);

	draw_meshes(shadow_map.get_program());
	glFlush();



	// reset camera matrices
	main_camera.calculate_camera_matrices(win_x, win_y);
	mv = model * main_camera.view_mat;
	normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));
	mvp = main_camera.projection_mat * mv;
	lightPV = shadowBias * lightFrustum.getProjectionMatrix() * lightFrustum.getViewMatrix();
	shadow = lightPV * model;
	view = main_camera.view_mat;

	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ModelViewMatrix"), 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix3fv(glGetUniformLocation(shadow_map.get_program(), "NormalMatrix"), 1, GL_FALSE, &normal[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "MVP"), 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ShadowMatrix"), 1, GL_FALSE, &shadow[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow_map.get_program(), "ViewMatrix"), 1, GL_FALSE, &view[0][0]);
	
	lp = view * vec4(lightPos, 1.0f);
	glUniform4f(glGetUniformLocation(shadow_map.get_program(), "LightPosition"), lp.x, lp.y, lp.z, lp.w);
	
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, win_x, win_y);
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass2Index);

	glCullFace(GL_BACK);
	draw_meshes(shadow_map.get_program());


	glFlush();
	glutSwapBuffers();

	glDeleteTextures(1, &depthTex);
	glDeleteFramebuffers(1, &shadowFBO);
}

void keyboard_func(unsigned char key, int x, int y)
{
	switch(tolower(key))
	{
	case 'a':
		break;

	default:
		break;
	}
}

void mouse_func(int button, int state, int x, int y)
{
	if(GLUT_LEFT_BUTTON == button)
	{
		if (GLUT_DOWN == state)
		{
			ray = screen_coords_to_world_coords(x, y, win_x, win_y);

			lmb_down = true;
		}
		else
			lmb_down = false;
	}
	else if(GLUT_MIDDLE_BUTTON == button)
	{
		if(GLUT_DOWN == state)
			mmb_down = true;
		else
			mmb_down = false;
	}
	else if(GLUT_RIGHT_BUTTON == button)
	{
		if(GLUT_DOWN == state)
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

	if(true == lmb_down && (0 != mouse_delta_x || 0 != mouse_delta_y))
	{
		//cout << main_camera.eye.x << ' ' << main_camera.eye.y << ' ' << main_camera.eye.z << endl;
		//cout << main_camera.look_at.x << ' ' << main_camera.look_at.y << ' ' << main_camera.look_at.z << endl;
		//cout << ray.x << ' ' << ray.y << ' ' << ray.z << endl;
		//cout << endl;

		main_camera.u -= static_cast<float>(mouse_delta_y)*u_spacer;
		main_camera.v += static_cast<float>(mouse_delta_x)*v_spacer;

		main_camera.calculate_camera_matrices(win_x, win_y);
	}
	else if(true == rmb_down && (0 != mouse_delta_y))
	{
		main_camera.w -= static_cast<float>(mouse_delta_y)*w_spacer;

		if(main_camera.w < 2.0f)
			main_camera.w = 2.0f;
		else if(main_camera.w > 20.0f)
			main_camera.w = 20.0f;

		main_camera.calculate_camera_matrices(win_x, win_y);
	}
}

void passive_motion_func(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
}




