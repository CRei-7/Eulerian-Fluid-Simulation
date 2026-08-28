#version 430
in vec2 vUV;

out vec4 FragColor;

uniform int uXCount;
uniform int uYCount;
uniform float uLineWidth;
uniform float uMaxSpeed;

uniform sampler2D uSolidMask;
uniform sampler2D uVelMask;

void main(){
	vec2 cell = fract(vUV * vec2(uXCount, uYCount));//takes fractional part, creating a repeated grid of cells
	float lineX = step(cell.x, uLineWidth) + step(1.0 - uLineWidth, cell.x);//step returns 1 if uLineWidth >= cell.x for the first part, basically labeling a fragment whether it is a line or not
	float lineY = step(cell.y, uLineWidth) + step(1.0 - uLineWidth, cell.y);
	float line = clamp(lineX + lineY, 0.0, 1.0);

	float speed = texture(uVelMask, vUV).r;
	float normSpeed = clamp(speed / uMaxSpeed, 0.0, 1.0);

	vec3 flowColor = mix(vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0), normSpeed);
	vec3 lineColor = vec3(1.0, 1.0, 1.0);

	float solid = texture(uSolidMask, vUV).r;//for solid cells
	vec3 solidColor = vec3(0.2, 0.2, 0.2);

	vec3 base = mix(flowColor, solidColor, solid);
	FragColor = vec4(mix(base, lineColor, line), 1.0);//interpolates between base and lineColor using line; (1-line)*base + line*lineColor
}