#version 430

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;

out vec3 Normal;
out vec3 Position;
out vec3 Untransformed_Position;
out vec4 ShadowCoord;


uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

uniform mat3 NormalMatrix;
uniform mat4 ShadowMatrix;


void main()
{
    mat4 mv = ViewMatrix * ModelMatrix;
    mat4 mvp = ProjectionMatrix*mv;

    Position = (mv * vec4(position,1.0)).xyz;
    Normal = normalize( NormalMatrix * normal );
    ShadowCoord = ShadowMatrix * vec4(position, 1.0);
    Untransformed_Position = position;

    gl_Position = mvp * vec4(position, 1.0);

}
