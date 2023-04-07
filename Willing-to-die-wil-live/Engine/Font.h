#pragma once
#include "Component.h"

class Font : public Component
{
public:
	Font();
	~Font();
	void BuildFont();
	vector<FontType> LoadFontData(const char* filename);
	vector<Vertex> GetTextVB(string sentence);
private:
	const static int gMaxNumTextCharacters = 256;
	std::unordered_map<std::string, std::vector<FontType>> mFontData;
};