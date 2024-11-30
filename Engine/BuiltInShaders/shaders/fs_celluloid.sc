$input v_worldPos, v_normal, v_texcoord0, v_TBN, v_color0

#include "../common/common.sh"
#include "../common/BRDF.sh"
#include "../common/Material.sh"
#include "../common/Camera.sh"

#include "../common/LightSource.sh"
#include "../common/Envirnoment.sh"

uniform vec4 u_emissiveColorAndFactor;
uniform vec4 u_cameraNearFarPlane;

uniform vec4 u_dividLine;
uniform vec4 u_specular;
uniform vec4 u_firstShadowColor;
uniform vec4 u_secondShadowColor;
uniform vec4 u_rimLightColor;
uniform vec4 u_rimLight;

vec3 GetCelDirectional(Material material, vec3 worldPos, vec3 viewDir, float csmDepth, CelParameter celParameter) {
	return CalculateCelLights(material, worldPos, viewDir, csmDepth, celParameter);
}

// TODO : add envirnoment light

void main()
{
	Material material = GetMaterial(v_texcoord0, v_normal, v_TBN);
	if (material.opacity < u_alphaCutOff.x) {
		discard;
	}
	
	vec3 cameraPos = GetCamera().position.xyz;
	vec3 viewDir = normalize(cameraPos - v_worldPos);
	
	// Directional Light
	float csmDepth = (v_color0.z - u_cameraNearFarPlane.x) / (u_cameraNearFarPlane.y - u_cameraNearFarPlane.x);
	CelParameter celParameter;
	celParameter.dividLine = u_dividLine;
	celParameter.specular = u_specular;
	celParameter.firstShadowColor = vec3(u_firstShadowColor.x, u_firstShadowColor.y, u_firstShadowColor.z);
	celParameter.secondShadowColor = vec3(u_secondShadowColor.x, u_secondShadowColor.y, u_secondShadowColor.z);
	celParameter.rimLightColor = vec3(u_rimLightColor.x, u_rimLightColor.y, u_rimLightColor.z);
	celParameter.rimLight = vec3(u_rimLight.x, u_rimLight.y, u_rimLight.z);
	vec3 dirColor = GetCelDirectional(material, v_worldPos, viewDir, csmDepth, celParameter);
	
	// Emissive
	vec3 emiColor = material.emissive * u_emissiveColorAndFactor.xyz * vec3_splat(u_emissiveColorAndFactor.w);
	
	// Fragment Color
	//gl_FragData[0] = vec4(dirColor + emiColor, 1.0);
	//gl_FragData[1] = vec4(emiColor, 1.0);
	
	// OpenGL : error C7616: global variable gl_FragData is removed after version 420
	gl_FragColor = vec4(dirColor + emiColor, 1.0);
	
	// Post-processing will be used in the last pass.
}
