#version 430




uniform sampler2DShadow shadow_map;

in vec3 Position;
in vec3 Normal;
in vec4 ShadowCoord;
in mat4 view_matrix;

    vec4 LightPosition = view_matrix * vec4(100.0, 100.0, 100.0, 1.0);
    vec3 LightIntensity = vec3(1.0, 1.0, 1.0);
    vec3 MaterialKa = vec3(0.1, 0.1, 0.1);
    vec3 MaterialKd = vec3(1.0, 1.0, 1.0);
    vec3 MaterialKs = vec3(1.0, 1.0, 1.0);
    float MaterialShininess = 100.0;

layout (location = 0) out vec4 FragColor;

vec3 phongModelDiffAndSpec()
{
    vec3 n = Normal;
    vec3 s = normalize(vec3(LightPosition) - Position);
    vec3 v = normalize(-Position.xyz);
    vec3 r = reflect( -s, n );
    float sDotN = max( dot(s,n), 0.0 );
   // vec3 diffuse = LightIntensity * MaterialKd * sDotN;
    vec3 diffuse = vec3(sDotN, sDotN, sDotN);
    vec3 spec = vec3(0.0);

    if( sDotN > 0.0 )
    {
        spec.x = pow( max( dot(r,v), 0.0 ), MaterialShininess );
         spec.y =  pow( max( dot(r,v), 0.0 ), MaterialShininess );
          spec.z = pow( max( dot(r,v), 0.0 ), MaterialShininess );
     }

    return diffuse + spec;
}

subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;

subroutine (RenderPassType)
void shadeWithShadow()
{
//   vec3 ambient = Light.Intensity * Material.Ka;
    vec3 diffAndSpec = phongModelDiffAndSpec();

    float shadow = 1.0;

    if( ShadowCoord.z >= 0 ) {
        shadow = textureProj(shadow_map, ShadowCoord);
    }

    vec3 ambient = vec3(0.1, 0.1, 0.1);
    // If the fragment is in shadow, use ambient light only.
    FragColor = vec4(diffAndSpec * shadow + ambient, 1.0);

    // Gamma correct
    FragColor = pow( FragColor, vec4(1.0 / 2.2) );
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
