precision mediump float; 
varying vec4 v_color;

uniform sampler2D uTexture;
uniform sampler2D uShadowMap;
uniform bool uShadowMapEnabled;

varying vec2 v_texCoord;
varying vec4 v_projectorCoord;
varying float v_distance;

void main() 
{
  vec4 projTexColor = vec4(1.0);
  vec4 rcoord = v_projectorCoord;

  if (greaterThan(v_projectorCoord, vec4(1.0)).x)
     rcoord = vec4(1.0);

  if (lessThan(v_projectorCoord, vec4(0.0)).x)
     rcoord = vec4(0.0);

  projTexColor = texture2DProj(uShadowMap,v_projectorCoord);

  if (projTexColor.r != 1.0 && rcoord.z > 0.0 && uShadowMapEnabled)
  {
    projTexColor = v_color * 0.8;
	projTexColor.a = 0.9;
  }
  else
  {
    projTexColor = vec4(1.0);
  }

  gl_FragColor = v_color * projTexColor * texture2D(uTexture, v_texCoord);
}
