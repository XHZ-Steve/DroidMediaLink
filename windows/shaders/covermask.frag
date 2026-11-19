#version 440
layout(location=0) in vec2 qt_TexCoord0;
layout(location=0) out vec4 fragColor;
layout(std140,binding=0) uniform buf {
 mat4 qt_Matrix;
 float qt_Opacity;
 vec2 extent;
 vec2 artworkExtent;
 float radius;
};
layout(binding=1) uniform sampler2D source;
void main(){
 vec2 halfSize=artworkExtent*.5;
 vec2 corner=abs((qt_TexCoord0-.5)*extent)-halfSize+radius;
 float distance=length(max(corner,vec2(0)))+min(max(corner.x,corner.y),0.)-radius;
 // Integrate the contour over the current screen pixel, including fractional DPI.
 float edge=max(.001,fwidth(distance))*.65;
 fragColor=texture(source,qt_TexCoord0)*(1.-smoothstep(-edge,edge,distance))*qt_Opacity;
}
