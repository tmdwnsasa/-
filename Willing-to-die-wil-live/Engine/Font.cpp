#include "pch.h"
#include "Engine.h"
#include "Font.h"

Font::Font() : Component(COMPONENT_TYPE::FONT)
{

}

Font::~Font()
{

}

void Font::BuildFont()
{
    mFontData["original"] = LoadFontData("..\\Resources\\Font\\example.txt");
    mFontData["deathspirit"] = LoadFontData("..\\Resources\\Font\\DeathSpirit.txt");

}

vector<FontType> Font::LoadFontData(const char* filename)
{
    ifstream fin;
    int i;
    char temp;


    vector<FontType> font(95);

    fin.open(filename);
    if (fin.fail())
    {
        return (vector<FontType>)0;
    }

    for (i = 0; i < 95; i++)
    {
        fin.get(temp);
        while (temp != ' ')
        {
            fin.get(temp);
        }
        fin.get(temp);
        while (temp != ' ')
        {
            fin.get(temp);
        }

        fin >> font[i].left;
        fin >> font[i].right;
        fin >> font[i].size;
    }

    fin.close();

    return font;
}

vector<Vertex> Font::GetTextVB(string sentence)
{
    const WindowInfo& window = GEngine->GetWindow();

    int numLetters = (int)sentence.size();

    if (numLetters >= gMaxNumTextCharacters)
    {
        throw std::runtime_error("sentence >= gMaxNumTextCharacters");
    }

    vector<Vertex> vertices(numLetters * 4);
    float drawX = -40.0f;
    float drawY = -0.5f;


    float scaleX = 100.0f;
    float scaleY = 150.0f;

    for (size_t i = 0, k = 0; i < numLetters; ++i, k += 4)
    {
        int letter = ((int)sentence[i] - 32);

        if (letter == 0)
        {
            drawX += 4.0f + scaleX;
        }
        else
        {
            // 0 3
            // 1 2

            // top left
            vertices[k + 0].pos = DirectX::XMFLOAT3(drawX, drawY, 0.0f);
            vertices[k + 0].uv = DirectX::XMFLOAT2(mFontData["deathspirit"][letter].left, 0.0f);

            // bottom left
            vertices[k + 1].pos = DirectX::XMFLOAT3(drawX, drawY - scaleY, 0.0f);
            vertices[k + 1].uv = DirectX::XMFLOAT2(mFontData["deathspirit"][letter].left, 1.0f);

            // bottom right
            vertices[k + 2].pos = DirectX::XMFLOAT3(drawX + mFontData["deathspirit"][letter].size + scaleX, drawY - scaleY, 0.0f);
            vertices[k + 2].uv = DirectX::XMFLOAT2(mFontData["deathspirit"][letter].right, 1.0f);

            // top right
            vertices[k + 3].pos = DirectX::XMFLOAT3(drawX + mFontData["deathspirit"][letter].size + scaleX, drawY, 0.0f);
            vertices[k + 3].uv = DirectX::XMFLOAT2(mFontData["deathspirit"][letter].right, 0.0f);

            drawX += mFontData["deathspirit"][letter].size + 10.0f + scaleX;
        }
    }
    return vertices;
}