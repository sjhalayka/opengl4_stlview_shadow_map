#version 430

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;

out vec3 Normal;
out vec3 Position;
out vec4 ShadowCoord;

out vec3 N;
out vec3 V;



uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;
uniform mat4 ShadowMatrix;
uniform mat4 ViewMatrix;

void main()
{
    Position = (ModelViewMatrix * vec4(position,1.0)).xyz;
    Normal = normalize( NormalMatrix * normal );
    ShadowCoord = ShadowMatrix * vec4(position,1.0);
    gl_Position = MVP * vec4(position,1.0);


        // Calculate view-space coordinate
    vec4 P = ModelViewMatrix * vec4(position, 1.0);

    // Calculate normal in view-space
    N = mat3(ModelViewMatrix) * normal;

    // Calculate view vector
    V = -P.xyz;


}
