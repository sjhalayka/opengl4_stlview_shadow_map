#version 430

uniform sampler2DShadow shadow_map;

in vec3 Position;
in vec3 Normal;
in vec4 ShadowCoord;
in vec3 Untransformed_Position;

uniform vec4 LightPosition; // in view space
uniform vec4 LightPosition_Untransformed; // in world space

uniform vec3 piece_dir;
uniform float dp_limit;
uniform int highlight_move_region;

uniform int flat_colour = 0;

vec3 LightIntensity = vec3(1.0, 1.0, 1.0);

uniform vec3 MaterialKd = vec3(1.0, 1.0, 1.0);

vec3 MaterialKs = vec3(1.0, 0.5, 0.0);
vec3 MaterialKa = vec3(0.0, 0.025, 0.075);
float MaterialShininess = 10.0;

layout (location = 0) out vec4 frag_colour;

vec3 phongModelDiffAndSpec(bool do_specular)
{
    vec3 n = Normal;
    vec3 s = normalize(vec3(LightPosition.xyz) - Position);
    vec3 v = normalize(-Position.xyz);
    vec3 r = reflect( -s, n );
    float sDotN = max( dot(s,n), 0.0 );
    vec3 diffuse = LightIntensity * MaterialKd * sDotN;
    vec3 spec = vec3(0.0);

    if( sDotN > 0.0 )
    {
        spec.x = MaterialKs.x * pow( max( dot(r,v), 0.0 ), MaterialShininess );
        spec.y = MaterialKs.y * pow( max( dot(r,v), 0.0 ), MaterialShininess );
        spec.z = MaterialKs.z * pow( max( dot(r,v), 0.0 ), MaterialShininess );
    }

    vec3 n2 = Normal;
    vec3 s2 = normalize(vec3(-LightPosition) - Position);
    vec3 v2 = normalize(-Position.xyz);
    vec3 r2 = reflect( -s2, n2 );
    float sDotN2 = max( dot(s2,n2)*0.5f, 0.0 );
    vec3 diffuse2 = LightIntensity*0.25 * MaterialKa * sDotN2;

    float k = (1.0 - sDotN)/2.0;
    vec3 ret = diffuse + diffuse2 + MaterialKa*k;

    if(do_specular)
        ret = ret + spec;
    
    return ret;
}

subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;

subroutine (RenderPassType)
void shadeWithShadow()
{
    if(flat_colour == 1)
    {
        frag_colour = vec4(MaterialKd, 1.0);
        return;
    }

    float shadow = 1.0;
    
    if( ShadowCoord.z >= 0.0 )
    {
        shadow = textureProj(shadow_map, ShadowCoord);

        /*
        vec3 n = normalize(Normal);
        vec3 n2 = normalize(LightPosition.xyz);
        float dp = dot(n, n2);

        if(dp <= 0.0)
        {
            shadow = 1.0;
        }
        else
        {
            if(shadow == 0.0)
                shadow = 1.0 - dp;
        }
        */    
    }
    
    
    vec3 diffAndSpec;
    
    if(shadow == 1.0)
    {
        diffAndSpec = phongModelDiffAndSpec(true);
        frag_colour = vec4(diffAndSpec, 1.0);// + vec4(diffAndSpec * shadow + MaterialKa*(1.0 - shadow), 1.0);
    }
    else
    {
        diffAndSpec = phongModelDiffAndSpec(false);
        frag_colour = vec4(diffAndSpec * shadow + MaterialKa*(1.0 - shadow), 1.0) + vec4(diffAndSpec, 1.0) + vec4(diffAndSpec * shadow + MaterialKa*(1.0 - shadow), 1.0);
        frag_colour /= 3;
    }

    frag_colour = pow( frag_colour, vec4(1.0 / 2.2) );



    /*
    // posterize https://www.geeks3d.com/20091027/shader-library-posterization-post-processing-effect-glsl/
        float gamma = 1.0; // 0.6
    float numColors = 4.0; // 8.0

    vec3 c = frag_colour.rgb;
    c = pow(c, vec3(gamma, gamma, gamma));
    c = c * numColors;
    c = floor(c);
    c = c / numColors;
    c = pow(c, vec3(1.0/gamma));
    frag_colour = vec4(c, 1.0);
    */
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