#ifndef __DRIVERS_SHADER_H
#define __DRIVERS_SHADER_H

typedef enum {
	SHADER_HQXX,
	SHADER_SCALE2X,
	SHADER_IPSHARPER,
	SHADER_IPXNOTY,
	SHADER_IPXNOTYSHARPER,
	SHADER_IPYNOTX,
	SHADER_IPYNOTXSHARPER
} ShaderType;


bool InitShader(ShaderType pixshader);
bool ShaderBegin(SDL_Surface *surface, const SDL_Rect *rect, int tw, int th);
bool ShaderEnd(void);
bool KillShader(void);

#endif
