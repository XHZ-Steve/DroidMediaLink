#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec4 primaryColor;
    vec4 secondaryColor;
    vec4 accentColor;
    vec4 deepColor;
    vec4 tuning;
    vec4 atmosphere;
    vec4 geometry;
    vec4 fluid;
    float phase;
    float monochromeStrength;
};
const vec3 luma=vec3(.2126,.7152,.0722);
vec3 linearColor(vec3 c) { return pow(max(c,vec3(0)),vec3(2.2)); }
vec3 vivid(vec3 c) {
    float y=dot(c,luma);
    float chroma=max(c.r,max(c.g,c.b))-min(c.r,min(c.g,c.b));
    float amount=max(0.,tuning.z+tuning.w*(1.-chroma));
    return clamp(mix(vec3(y),c,amount),0.,1.);
}
float region(vec2 p,vec2 center,vec2 stretch,float radius) {
    vec2 delta=(p-center)*stretch;
    // A broad shoulder removes the steep, pinched join of overlapping Gaussians.
    float d2=dot(delta,delta)/max(.015,radius*radius);
    return exp(-d2/(1.+1.6*fluid.x));
}
vec2 lightCenter(float phase, float offset) {
    float t=phase+offset;
    // Spend longer at the ends of each sweep and accelerate through the middle.
    float u=t-.24*sin(2.*t);
    vec2 meander=vec2(sin(7.*t+.6),cos(5.*t+1.1))*.014;
    return vec2(.5+.405*cos(u),.5+.29*sin(u)+.14*sin(3.*u))+meander;
}
vec2 separate(vec2 delta,vec2 aspect) {
    float distance=length(delta*aspect);
    return delta/max(distance,.001)*max(0.,.24-distance)*.5;
}
void main() {
    vec2 uv=qt_TexCoord0;
    float aspect=max(geometry.x,1.)/max(geometry.y,1.);
    vec2 metric=vec2(sqrt(aspect),inversesqrt(aspect));
    float radius=.42*atmosphere.y;
    float t=phase*.65;
    vec2 p0=lightCenter(t,0.);
    vec2 p1=lightCenter(t,2.0943951);
    vec2 p2=lightCenter(t,4.1887902);
    vec2 separationAspect=geometry.xy/max(1.,min(geometry.x,geometry.y));
    vec2 d01=separate(p0-p1,separationAspect),d02=separate(p0-p2,separationAspect),d12=separate(p1-p2,separationAspect);
    vec2 drift=(p0-vec2(.5))*.35;
    float a=region(uv,clamp(p0+d01+d02,.06,.94),metric,radius*(1.+.10*sin(t)));
    float b=region(uv,clamp(p1-d01+d12,.06,.94),metric,radius*.95);
    float c=region(uv,clamp(p2-d02-d12,.06,.94),metric,radius*.72)*(.52+.08*sin(t));
    // Local colour ownership prevents opposite hues becoming a flat grey field.
    vec3 weights=pow(vec3(a,b,c),vec3(1.35));
    float total=weights.x+weights.y+weights.z;
    vec3 colors=(linearColor(vivid(primaryColor.rgb))*weights.x+
                 linearColor(vivid(secondaryColor.rgb))*weights.y+
                 linearColor(vivid(accentColor.rgb))*weights.z)/max(total,.0001);
    vec3 base=vivid(deepColor.rgb);
    float darkLuma=dot(base,luma);
    base*=min(1.,.09/max(darkLuma,.001));
    vec3 darkBase=base+vec3(max(0.,.04*atmosphere.w-dot(base,luma)));
    // Continuous energy mapping has no clipped overlap boundary. Fill follows
    // the *uncovered* area and borrows spatially distinct broad palette lobes.
    float coverage=1.-exp(-total*tuning.x*2.2);
    vec3 fillWeights=vec3(
        region(uv,vec2(.08,.72)+drift*.35,metric,.85*fluid.w),
        region(uv,vec2(.92,.25)-drift*.35,metric,.85*fluid.w),
        region(uv,vec2(.52,.95),metric,.7*fluid.w));
    float fillTotal=dot(fillWeights,vec3(1.));
    vec3 fillColor=(linearColor(vivid(primaryColor.rgb))*fillWeights.x+
                    linearColor(vivid(secondaryColor.rgb))*fillWeights.y+
                    linearColor(vivid(accentColor.rgb))*fillWeights.z)/max(fillTotal,.0001);
    float fillEnergy=fluid.y*fluid.z*tuning.x*(1.-coverage)*(1.-exp(-fillTotal*2.));
    // Energy-weighted colour, rather than an opaque global brightening layer.
    colors=(colors*coverage+fillColor*fillEnergy)/max(coverage+fillEnergy,.0001);
    coverage=min(1.,coverage+fillEnergy);
    vec3 darkField=mix(linearColor(darkBase),colors,coverage*.32);
    vec3 lightField=mix(linearColor(vec3(.965,.972,.985)),colors,coverage*.21);
    vec3 result=pow(max(mix(darkField,lightField,geometry.w),vec3(0)),vec3(1./2.2));
    // Narrow high-chroma covers retain their hue, with a little neutral depth.
    // Source-gamut detection leaves every richer cover's output unchanged.
    float monochrome=clamp(monochromeStrength,0.,1.);
    result=mix(result,vec3(dot(result,luma)),monochrome*.22);
    result=mix(result,vec3(0.),monochrome*.18*a);
    result=mix(result,vec3(1.),monochrome*.065*b*(1.-.5*a));
    vec3 neutral=mix(vec3(.025,.032,.046),vec3(.975,.979,.988),geometry.w);
    result=mix(neutral,result,atmosphere.x)*tuning.y;
    // Fixed in device-pixel space, never keyed to time: sub-LSB spatial dither
    // breaks 8-bit contours without grain or temporal shimmer.
    vec2 pixel=floor(uv*geometry.xy*max(geometry.z,1.));
    float noise=fract(52.9829189*fract(dot(pixel,vec2(.06711056,.00583715))))-.5;
    result+=noise*(atmosphere.z/255.);
    fragColor=vec4(clamp(result,0.,1.),1.)*qt_Opacity;
}
