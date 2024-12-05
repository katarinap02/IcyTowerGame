#version 330 core

in vec2 chTex; //koordinate teksture
out vec4 outCol;

uniform sampler2D uTex; //teksturna jedinica, ne menjaju se nikad zato uniform sampler
uniform sampler2D uWallTex;
uniform sampler2D uLetterTex;
uniform sampler2D uAvatarTex;
uniform sampler2D uFinishedTex;
uniform sampler2D uNameTex;
in float ObjectType; 

uniform float uA;

void main()
{
	if(ObjectType == 1.0)
	{
		outCol = vec4(1.0, uA, uA, 1.0);
	}
	else if (ObjectType == 2.0) 
    {
        outCol = texture(uLetterTex, chTex); 
    }
	else if (ObjectType == 3.0) 
    {
        outCol = texture(uAvatarTex, chTex); 
    }
	else if (ObjectType == 4.0) {
        outCol = texture(uWallTex, chTex);
    } 
	else if (ObjectType == 5.0) {
        outCol = texture(uFinishedTex, chTex);
    }
	else if (ObjectType == 6.0) {
        outCol = texture(uNameTex, chTex);
    }
	else {
        outCol = texture(uTex, chTex);
    }
	
	
}