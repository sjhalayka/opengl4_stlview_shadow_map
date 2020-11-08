#include "mesh.h"


void mesh::rotate_and_translate_mesh(float yaw, float pitch, vec3 translate_vec)
{
	for (size_t i = 0; i < triangles.size(); i++)
	{
		vec4 v0;
		v0.x = triangles[i].vertex[0].x;
		v0.y = triangles[i].vertex[0].y;
		v0.z = triangles[i].vertex[0].z;
		v0.w = 1;

		vec4 v1;
		v1.x = triangles[i].vertex[1].x;
		v1.y = triangles[i].vertex[1].y;
		v1.z = triangles[i].vertex[1].z;
		v1.w = 1;

		vec4 v2;
		v2.x = triangles[i].vertex[2].x;
		v2.y = triangles[i].vertex[2].y;
		v2.z = triangles[i].vertex[2].z;
		v2.w = 1;

		static const mat4 identity_mat = glm::mat4(1.0f);

		mat4 rot0_mat = rotate(identity_mat, yaw, glm::vec3(0.0, 1.0, 0.0));
		mat4 rot1_mat = rotate(identity_mat, pitch, glm::vec3(1.0, 0.0, 0.0));
		mat4 translate_mat = translate(identity_mat, translate_vec);

		mat4 transform = translate_mat * rot1_mat * rot0_mat;

		v0 = transform * v0;
		v1 = transform * v1;
		v2 = transform * v2;

		triangles[i].vertex[0].x = v0.x;
		triangles[i].vertex[0].y = v0.y;
		triangles[i].vertex[0].z = v0.z;

		triangles[i].vertex[1].x = v1.x;
		triangles[i].vertex[1].y = v1.y;
		triangles[i].vertex[1].z = v1.z;

		triangles[i].vertex[2].x = v2.x;
		triangles[i].vertex[2].y = v2.y;
		triangles[i].vertex[2].z = v2.z;

	}

	get_vertices_and_normals_from_triangles();

	init_opengl_data();
}


void mesh::get_vertices_and_normals_from_triangles(void)
{
	face_normals.clear();
	vertices.clear();
	vertex_normals.clear();
	
	if(0 == triangles.size())
		return;

	cout << "Triangles: " << triangles.size() << endl;
	
    cout << "Welding vertices" << endl;
 
    // Insert unique vertices into set.
    set<indexed_vertex_3> vertex_set;
 
    for(vector<triangle>::const_iterator i = triangles.begin(); i != triangles.end(); i++)
    {
        vertex_set.insert(i->vertex[0]);
        vertex_set.insert(i->vertex[1]);
        vertex_set.insert(i->vertex[2]);
    }
 
    cout << "Vertices: " << vertex_set.size() << endl;

    cout << "Generating vertex indices" << endl;
 
	vector<indexed_vertex_3> vv;

    // Add indices to the vertices.
    for(set<indexed_vertex_3>::const_iterator i = vertex_set.begin(); i != vertex_set.end(); i++)
    {
        size_t index = vv.size();
        vv.push_back(*i);
        vv[index].index = index;
    }

	for (size_t i = 0; i < vv.size(); i++)
	{
		vec3 vv_element(vv[i].x, vv[i].y, vv[i].z);
		vertices.push_back(vv_element);
	}
 
    vertex_set.clear();

	// Re-insert modifies vertices into set.
    for(vector<indexed_vertex_3>::const_iterator i = vv.begin(); i != vv.end(); i++)
        vertex_set.insert(*i);
 
    cout << "Assigning vertex indices to triangles" << endl;
   
    // Find the three vertices for each triangle, by index.
    set<indexed_vertex_3>::iterator find_iter;
 
    for(vector<triangle>::iterator i = triangles.begin(); i != triangles.end(); i++)
    {
        find_iter = vertex_set.find(i->vertex[0]);
        i->vertex[0].index = find_iter->index;
 
        find_iter = vertex_set.find(i->vertex[1]);
        i->vertex[1].index = find_iter->index;
 
        find_iter = vertex_set.find(i->vertex[2]);
        i->vertex[2].index = find_iter->index;
    }

	vertex_set.clear();

	cout << "Calculating normals" << endl;
	face_normals.resize(triangles.size());
	vertex_normals.resize(vertices.size());

	for(size_t i = 0; i < triangles.size(); i++)
	{
		vec3 v0;
		v0.x = triangles[i].vertex[1].x - triangles[i].vertex[0].x;
		v0.y = triangles[i].vertex[1].y - triangles[i].vertex[0].y;
		v0.z = triangles[i].vertex[1].z - triangles[i].vertex[0].z;

		vec3 v1;
		v1.x = triangles[i].vertex[2].x - triangles[i].vertex[0].x;
		v1.y = triangles[i].vertex[2].y - triangles[i].vertex[0].y;
		v1.z = triangles[i].vertex[2].z - triangles[i].vertex[0].z;

		face_normals[i] = cross(v0, v1);
		face_normals[i] = normalize(face_normals[i]);

		vertex_normals[triangles[i].vertex[0].index] = vertex_normals[triangles[i].vertex[0].index] + face_normals[i];
		vertex_normals[triangles[i].vertex[1].index] = vertex_normals[triangles[i].vertex[1].index] + face_normals[i];
		vertex_normals[triangles[i].vertex[2].index] = vertex_normals[triangles[i].vertex[2].index] + face_normals[i];
	}

	for (size_t i = 0; i < vertex_normals.size(); i++)
		vertex_normals[i] = normalize(vertex_normals[i]);
}


void mesh::init_opengl_data(void)
{
	opengl_vertex_data.clear();

	for (size_t i = 0; i < triangles.size(); i++)
	{
		size_t v0_index = triangles[i].vertex[0].index;
		size_t v1_index = triangles[i].vertex[1].index;
		size_t v2_index = triangles[i].vertex[2].index;

		vec3 v0_fn(vertex_normals[v0_index].x, vertex_normals[v0_index].y, vertex_normals[v0_index].z);
		vec3 v1_fn(vertex_normals[v1_index].x, vertex_normals[v1_index].y, vertex_normals[v1_index].z);
		vec3 v2_fn(vertex_normals[v2_index].x, vertex_normals[v2_index].y, vertex_normals[v2_index].z);

		vec3 v0(triangles[i].vertex[0].x, triangles[i].vertex[0].y, triangles[i].vertex[0].z);
		vec3 v1(triangles[i].vertex[1].x, triangles[i].vertex[1].y, triangles[i].vertex[1].z);
		vec3 v2(triangles[i].vertex[2].x, triangles[i].vertex[2].y, triangles[i].vertex[2].z);

		opengl_vertex_data.push_back(v0.x);
		opengl_vertex_data.push_back(v0.y);
		opengl_vertex_data.push_back(v0.z);
		opengl_vertex_data.push_back(v0_fn.x);
		opengl_vertex_data.push_back(v0_fn.y);
		opengl_vertex_data.push_back(v0_fn.z);

		opengl_vertex_data.push_back(v1.x);
		opengl_vertex_data.push_back(v1.y);
		opengl_vertex_data.push_back(v1.z);
		opengl_vertex_data.push_back(v1_fn.x);
		opengl_vertex_data.push_back(v1_fn.y);
		opengl_vertex_data.push_back(v1_fn.z);

		opengl_vertex_data.push_back(v2.x);
		opengl_vertex_data.push_back(v2.y);
		opengl_vertex_data.push_back(v2.z);
		opengl_vertex_data.push_back(v2_fn.x);
		opengl_vertex_data.push_back(v2_fn.y);
		opengl_vertex_data.push_back(v2_fn.z);
	}
}

bool mesh::read_triangles_from_binary_stereo_lithography_file(const char *const file_name)
{
	triangles.clear();
	
    // Write to file.
    ifstream in(file_name, ios_base::binary);
 
    if(in.fail())
        return false;

	const size_t header_size = 80;
	vector<char> buffer(header_size, 0);
	unsigned int num_triangles = 0; // Must be 4-byte unsigned int.
	indexed_vertex_3 normal;

	// Read header.
	in.read(reinterpret_cast<char *>(&(buffer[0])), header_size);
	
	if(header_size != in.gcount())
		return false;

	// Read number of triangles.
	in.read(reinterpret_cast<char *>(&num_triangles), sizeof(unsigned int));
	
	if(sizeof(unsigned int) != in.gcount())
		return false;

	triangles.resize(num_triangles);

	// Enough bytes for twelve 4-byte floats plus one 2-byte integer, per triangle.
	const size_t data_size = (12*sizeof(float) + sizeof(short unsigned int)) * num_triangles;
	buffer.resize(data_size, 0);

	in.read(reinterpret_cast<char *>(&buffer[0]), data_size);

	if(data_size != in.gcount())
		return false;

	// Use a pointer to assist with the copying.
	// Should probably use std::copy() instead, but memcpy() does the trick, so whatever...
	char *cp = &buffer[0];

    for(vector<triangle>::iterator i = triangles.begin(); i != triangles.end(); i++)
    {
		// Skip face normal.
		cp += 3*sizeof(float);
		
		// Get vertices.
		memcpy(&i->vertex[0].x, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[0].y, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[0].z, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[1].x, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[1].y, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[1].z, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[2].x, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[2].y, cp, sizeof(float)); cp += sizeof(float);
		memcpy(&i->vertex[2].z, cp, sizeof(float)); cp += sizeof(float);

		// Skip attribute.
		cp += sizeof(short unsigned int);
    }

	in.close();

	get_vertices_and_normals_from_triangles();

	init_opengl_data();




    return true;
} 

void mesh::scale_mesh(float max_extent)
{
	float curr_x_min = numeric_limits<float>::max();
	float curr_y_min = numeric_limits<float>::max();
	float curr_z_min = numeric_limits<float>::max();
	float curr_x_max = numeric_limits<float>::min();
	float curr_y_max = numeric_limits<float>::min();
	float curr_z_max = numeric_limits<float>::min();
	
	for(size_t i = 0; i < triangles.size(); i++)
	{
		for(size_t j = 0; j < 3; j++)
		{
			if(triangles[i].vertex[j].x < curr_x_min)
				curr_x_min = triangles[i].vertex[j].x;
				
			if(triangles[i].vertex[j].x > curr_x_max)
				curr_x_max = triangles[i].vertex[j].x;

			if(triangles[i].vertex[j].y < curr_y_min)
				curr_y_min = triangles[i].vertex[j].y;
				
			if(triangles[i].vertex[j].y > curr_y_max)
				curr_y_max = triangles[i].vertex[j].y;

			if(triangles[i].vertex[j].z < curr_z_min)
				curr_z_min = triangles[i].vertex[j].z;
				
			if(triangles[i].vertex[j].z > curr_z_max)
				curr_z_max = triangles[i].vertex[j].z;
		}			
	}
	
	float curr_x_extent = fabsf(curr_x_min - curr_x_max);
	float curr_y_extent = fabsf(curr_y_min - curr_y_max);
	float curr_z_extent = fabsf(curr_z_min - curr_z_max);

	float curr_max_extent = curr_x_extent;
	
	if(curr_y_extent > curr_max_extent)
		curr_max_extent = curr_y_extent;
		
	if(curr_z_extent > curr_max_extent)
		curr_max_extent = curr_z_extent;
	
	float scale_value = max_extent / curr_max_extent;
	
	cout << "Original max extent: " << curr_max_extent << endl;
	cout << "Scaling all vertices by a factor of: " << scale_value << endl;
	cout << "New max extent: " << max_extent << endl;

	for(size_t i = 0; i < triangles.size(); i++)
	{
		for(size_t j = 0; j < 3; j++)
		{
			triangles[i].vertex[j].x *= scale_value;
			triangles[i].vertex[j].y *= scale_value;
			triangles[i].vertex[j].z *= scale_value;
		}			
	}

	get_vertices_and_normals_from_triangles();

	init_opengl_data();
}

void mesh::draw(GLint render_shader_program,
	int win_x,
	int win_y
//	uv_camera& main_camera,
//	GLint proj_matrix_uniform_location,
//	GLint view_matrix_uniform_location,
//	GLint specular_uniform_location,
//	bool use_specular
)
{
	glUseProgram(render_shader_program);

	//main_camera.calculate_camera_matrices(win_x, win_y);
	//glUniformMatrix4fv(proj_matrix_uniform_location, 1, GL_FALSE, &main_camera.projection_mat[0][0]);
	//glUniformMatrix4fv(view_matrix_uniform_location, 1, GL_FALSE, &main_camera.view_mat[0][0]);

	//glUniform1i(specular_uniform_location, static_cast<GLint>(use_specular));


	GLuint components_per_vertex = 6;
	const GLuint components_per_normal = 3;
	GLuint components_per_position = 3;

	GLuint triangle_buffer;

	glGenBuffers(1, &triangle_buffer);

	GLuint num_vertices = static_cast<GLuint>(opengl_vertex_data.size()) / components_per_vertex;

	glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer);
	glBufferData(GL_ARRAY_BUFFER, opengl_vertex_data.size() * sizeof(GLfloat), &opengl_vertex_data[0], GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(glGetAttribLocation(render_shader_program, "position"));
	glVertexAttribPointer(glGetAttribLocation(render_shader_program, "position"),
		components_per_position,
		GL_FLOAT,
		GL_FALSE,
		components_per_vertex * sizeof(GLfloat),
		NULL);

	glEnableVertexAttribArray(glGetAttribLocation(render_shader_program, "normal"));
	glVertexAttribPointer(glGetAttribLocation(render_shader_program, "normal"),
		components_per_normal,
		GL_FLOAT,
		GL_TRUE,
		components_per_vertex * sizeof(GLfloat),
		(const GLvoid*)(components_per_position * sizeof(GLfloat)));

	glDrawArrays(GL_TRIANGLES, 0, num_vertices);

	glDeleteBuffers(1, &triangle_buffer);
}