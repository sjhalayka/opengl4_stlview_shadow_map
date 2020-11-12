#version 430

uniform sampler2DShadow shadow_map;

in vec3 Position;
in vec3 Normal;
in vec4 ShadowCoord;

in vec3 N;
in vec3 V;


    uniform vec4 LightPosition; // in view space
    
    vec3 LightIntensity = vec3(1.0, 1.0, 1.0);
    vec3 MaterialKa = vec3(0.1, 0.1, 0.1);
    vec3 MaterialKd = vec3(1.0, 1.0, 1.0);
    vec3 MaterialKs = vec3(1.0, 1.0, 1.0);
    float MaterialShininess = 1000.0;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 normal_depth;

vec3 phongModelDiffAndSpec()
{
    vec3 n = Normal;
    vec3 s = normalize(vec3(LightPosition) - Position);
    vec3 v = normalize(-Position.xyz);
    vec3 r = reflect( -s, n );
    float sDotN = max( dot(s,n), 0.0 );
    vec3 diffuse = LightIntensity * MaterialKd * sDotN;
    vec3 spec = vec3(0.0);

    if( sDotN > 0.0 )
    {
        spec.x = pow( max( dot(r,v), 0.0 ), MaterialShininess );
        spec.y = pow( max( dot(r,v), 0.0 ), MaterialShininess );
        spec.z = pow( max( dot(r,v), 0.0 ), MaterialShininess );
     }

    return diffuse + spec;
}

subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;

subroutine (RenderPassType)
void shadeWithShadow()
{
    vec3 diffAndSpec = phongModelDiffAndSpec();

    float shadow = 1.0;

    if( ShadowCoord.z >= 0 ) {
        shadow = textureProj(shadow_map, ShadowCoord);
    }

    // If the fragment is in shadow, use ambient light only.
    FragColor = vec4(diffAndSpec * shadow + MaterialKa, 1.0);

    // Gamma correct
    //FragColor = pow( FragColor, vec4(1.0 / 2.2) );
 
    normal_depth = vec4(normalize(N), V.z);
}

subroutine (RenderPassType)
void recordDepth()
{
    // Do nothing, depth will be written automatically
}

void main() {
    // This will call either shadeWithShadow or recordDepth
    RenderPass();
}
