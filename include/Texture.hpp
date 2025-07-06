#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <SDL.h>
#include <SDL_ttf.h>

class Texture
{
private:
	SDL_Texture* texture_;

public:
	int width_;
	int height_;

	Texture();
	
	~Texture();

	void FreeTexture();

	bool LoadFromPath(SDL_Renderer* renderer, const char* path);

	bool LoadFromText(SDL_Renderer* renderer, TTF_Font* font, const char* text, const SDL_Color& text_color, int text_length = -1);

	void Render(SDL_Renderer* renderer, int x, int y, SDL_Rect* clip = nullptr, double scale = 1.0);
};

#endif
