
#version 330
in vec4 vPosition;
in vec3 vNormal;
in vec4 vVtxColor;
out vec4 vColor;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec4 vObjectColor;

void main()
{
	gl_Position = Projection * ModelView * vPosition;
	if( vVtxColor.x == -1.0f && vVtxColor.y == -1.0f && vVtxColor.z == -1.0f ) {
		vColor = vObjectColor;
	}
	else vColor = vVtxColor;
}