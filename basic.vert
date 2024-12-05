#version 330 core 

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTex; //Koordinate texture, propustamo ih u FS kao boje

out vec2 chTex;
uniform float isObject;
out float ObjectType;
uniform vec3 Loc;

void main()
{
	gl_Position = vec4(inPos.x*Loc.z + Loc.x, inPos.y + Loc.y, 0.0, 1.0);
	chTex = inTex;
	ObjectType = isObject;
}