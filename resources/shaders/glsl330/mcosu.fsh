#version 110

uniform sampler2D texture0;
uniform int style;
uniform float bodyColorSaturation;
uniform float bodyAlphaMultiplier;
uniform float borderSizeMultiplier;
uniform float borderFeather;
uniform vec3 colBorder;
uniform vec3 colBody;

varying vec2 fragTexCoord;

const float defaultTransitionSize = 0.011;
const float defaultBorderSize = 0.11;
const float outerShadowSize = 0.08;

vec4 getInnerBodyColor(in vec4 bodyColor)
{
	float brightnessMultiplier = 0.25;
	bodyColor.r = min(1.0, bodyColor.r * (1.0 + 0.5 * brightnessMultiplier) + brightnessMultiplier);
	bodyColor.g = min(1.0, bodyColor.g * (1.0 + 0.5 * brightnessMultiplier) + brightnessMultiplier);
	bodyColor.b = min(1.0, bodyColor.b * (1.0 + 0.5 * brightnessMultiplier) + brightnessMultiplier);
	return vec4(bodyColor);
}

vec4 getOuterBodyColor(in vec4 bodyColor)
{
	float darknessMultiplier = 0.1;
	bodyColor.r = min(1.0, bodyColor.r / (1.0 + darknessMultiplier));
	bodyColor.g = min(1.0, bodyColor.g / (1.0 + darknessMultiplier));
	bodyColor.b = min(1.0, bodyColor.b / (1.0 + darknessMultiplier));
	return vec4(bodyColor);
}

void main()
{
	float borderSize = (defaultBorderSize + borderFeather) * borderSizeMultiplier;
	float transitionSize = defaultTransitionSize + borderFeather;

	// output var
	vec4 out_color = vec4(0.0);
	
	// dynamic color calculations
	vec4 borderColor = vec4(colBorder.x, colBorder.y, colBorder.z, 1.0);
	vec4 bodyColor = vec4(colBody.x, colBody.y, colBody.z, 0.7*bodyAlphaMultiplier);
	vec4 outerShadowColor = vec4(0, 0, 0, 0.25);
	vec4 innerBodyColor = getInnerBodyColor(bodyColor);
	vec4 outerBodyColor = getOuterBodyColor(bodyColor);
	
	innerBodyColor.rgb *= bodyColorSaturation;
	outerBodyColor.rgb *= bodyColorSaturation;
	
	// osu!next style color modifications
	if (style == 1)
	{
		outerBodyColor.rgb = bodyColor.rgb * bodyColorSaturation;
		outerBodyColor.a = 1.0*bodyAlphaMultiplier;
		innerBodyColor.rgb = bodyColor.rgb * 0.5 * bodyColorSaturation;
		innerBodyColor.a = 0.0;
	}
	
	// a bit of a hack, but better than rough edges
	if (borderSizeMultiplier < 0.01)
		borderColor = outerShadowColor;
	
	// conditional variant
	
	if (fragTexCoord.x < outerShadowSize - transitionSize) // just shadow
	{
		float delta = fragTexCoord.x / (outerShadowSize - transitionSize);
		out_color = mix(vec4(0), outerShadowColor, delta);
	}
	if (fragTexCoord.x > outerShadowSize - transitionSize && fragTexCoord.x < outerShadowSize + transitionSize) // shadow + border
	{
		float delta = (fragTexCoord.x - outerShadowSize + transitionSize) / (2.0*transitionSize);
		out_color = mix(outerShadowColor, borderColor, delta);
	}
	if (fragTexCoord.x > outerShadowSize + transitionSize && fragTexCoord.x < outerShadowSize + borderSize - transitionSize) // just border
	{
		out_color = borderColor;
	}
	if (fragTexCoord.x > outerShadowSize + borderSize - transitionSize && fragTexCoord.x < outerShadowSize + borderSize + transitionSize) // border + outer body
	{
		float delta = (fragTexCoord.x - outerShadowSize - borderSize + transitionSize) / (2.0*transitionSize);
		out_color = mix(borderColor, outerBodyColor, delta);
	}
	if (fragTexCoord.x > outerShadowSize + borderSize + transitionSize) // outer body + inner body
	{	
		float size = outerShadowSize + borderSize + transitionSize;
		float delta = ((fragTexCoord.x - size) / (1.0-size));
		out_color = mix(outerBodyColor, innerBodyColor, delta);
	}
	
	gl_FragColor = out_color;
}