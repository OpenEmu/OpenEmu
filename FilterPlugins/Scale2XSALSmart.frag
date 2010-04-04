/*

  2xSaL Smart shader 

           - Copyright (C) 2007 guest(r) - guest.r@gmail.com

           - License: GNU-GPL  
*/



uniform sampler2D OGL2Texture;

const vec3 dt = vec3(1.4,1.4,1.4);

void main()
{

    // Calculating texel coordinates
	
    vec2 size     = vec2(2048.0,1024.0);
    vec2 inv_size = vec2(1.0/2048.0, 1.0/1024.0);

    vec2 OGL2Pos = gl_TexCoord[0].xy*size;
    vec2 fp  = fract(OGL2Pos);
    vec2 dx  = vec2(inv_size.x,0.0);
    vec2 dy  = vec2(0.0,inv_size.y);
    vec2 g1  = vec2( inv_size.x,inv_size.y);
    vec2 g2  = vec2(-inv_size.x,inv_size.y);

    vec2 pC4 = floor(OGL2Pos)*inv_size;
   

    // Reading the texels

    vec3 C0 = texture2D(OGL2Texture,pC4-g1).xyz; 
    vec3 C1 = texture2D(OGL2Texture,pC4-dy).xyz;
    vec3 C2 = texture2D(OGL2Texture,pC4-g2).xyz;
    vec3 C3 = texture2D(OGL2Texture,pC4-dx).xyz;
    vec3 C4 = texture2D(OGL2Texture,pC4   ).xyz;
    vec3 C5 = texture2D(OGL2Texture,pC4+dx).xyz;
    vec3 C6 = texture2D(OGL2Texture,pC4+g2).xyz;
    vec3 C7 = texture2D(OGL2Texture,pC4+dy).xyz;
    vec3 C8 = texture2D(OGL2Texture,pC4+g1).xyz;

    vec3 ul,ur,dl,dr; float m1,m2;

    m1=dot(abs(C0-C4),dt)+0.001;
    m2=dot(abs(C1-C3),dt)+0.001;
    ul=(m2*(C0+C4)+m1*(C1+C3))/(m1+m2);

    m1=dot(abs(C1-C5),dt)+0.001;
    m2=dot(abs(C2-C4),dt)+0.001;
    ur=(m2*(C1+C5)+m1*(C2+C4))/(m1+m2);

    m1=dot(abs(C3-C7),dt)+0.001;
    m2=dot(abs(C6-C4),dt)+0.001;
    dl=(m2*(C3+C7)+m1*(C6+C4))/(m1+m2);

    m1=dot(abs(C4-C8),dt)+0.001;
    m2=dot(abs(C5-C7),dt)+0.001;
    dr=(m2*(C4+C8)+m1*(C5+C7))/(m1+m2);
    
    gl_FragColor.xyz = 0.5*((dr*fp.x+dl*(1-fp.x))*fp.y+(ur*fp.x+ul*(1-fp.x))*(1-fp.y));
	gl_FragColor.a = 1.0;
}

