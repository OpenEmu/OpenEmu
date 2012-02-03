void main()
{
    vec4 offsetx;
    vec4 offsety;
    
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_TexCoord[0] = gl_MultiTexCoord0;
}