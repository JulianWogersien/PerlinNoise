#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

struct TextBox
{
    sf::Vector2f pos;
    std::string text;
    sf::Vector2f size;
    bool selected;
    sf::Font* font;
    int state;
    int blinkingSpeed;
    int time = 0;
};

struct Button
{
    sf::Vector2f pos;
    std::string text;
    sf::Vector2f size;
    int state;
    bool held;
    sf::Font* font;
    void (*function)();
};

const float PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229f;
std::string typed;
int octaveCount = 8;
int mode = 0;
float bias = 2.0f;
bool regen = false;
bool seed = true;
bool back = false;
std::vector<TextBox> textboxes = std::vector<TextBox>();

void PerlinNoise1D(int count, float* seed, int octaves, float* output, float bias);
void PerlinNoise2D(int width, int height, float* seed, int octaves, float* output, float bias);
void DrawButtons(std::vector<Button>* buttons, sf::RenderWindow* window);
void drawTextBoxes(std::vector<TextBox>* textboxes, sf::RenderWindow* window);
void UpdateButtons(std::vector<Button>* buttons, sf::Vector2f mPos);
void updateTextBoxes(std::vector<TextBox>* textboxes, sf::Vector2f mPos);
void loadFont(std::string path, sf::Font* font);
void setPixel(int x, int y, sf::Color color, sf::Uint8* pixels, int col);
Button createButton(sf::Vector2f pos, std::string text, sf::Vector2f size, void (*function)(), sf::Font* font);
TextBox createTextBox(sf::Vector2f pos, std::string text, sf::Vector2f size, sf::Font* font, int blinkingSpeed);

int main()
{
    srand(time(0));
    sf::RenderWindow window(sf::VideoMode(1024, 1024), "Perlin Noise");
    window.setFramerateLimit(60);

    float fps;
    sf::Clock clock = sf::Clock::Clock();
    sf::Time previousTime = clock.getElapsedTime();
    sf::Time currentTime;

    bool firstGen = true;

    int outputWidth = 1024;
    int outputHeight = 1024;
    float* noiseSeed2D = new float[outputWidth * outputHeight];
    float* perlinNoise2D =new float[outputWidth * outputHeight];
    for(size_t i = 0; i < outputWidth * outputHeight; i++)
        noiseSeed2D[i] = (float)rand() / (float)RAND_MAX;

    int outputSize = window.getSize().x;
    float* noiseSeed1D = new float[outputSize];
    float* perlinNoise1D = new float[outputSize];
    for (size_t i = 0; i < outputSize; i++)
        noiseSeed1D[i] = (float)rand() / (float)RAND_MAX;

    sf::Uint8* pixels2D = new sf::Uint8[outputWidth * outputHeight * 4];
    sf::Uint8* pixels = new sf::Uint8[window.getSize().x * window.getSize().y * 4];
    sf::Texture texture;
    sf::Texture texture2D;
    texture2D.create(outputWidth, outputHeight);
    texture.create(window.getSize().x, window.getSize().y);
    sf::Sprite sprite(texture);
    sf::Sprite sprite2D(texture2D);

    sf::Font font;
    loadFont("Roboto-Regular.ttf", &font);

    textboxes.push_back(createTextBox(sf::Vector2f(900, 600), "2.0", sf::Vector2f(150, 30), &font, 10));
    
    std::vector<Button> buttons = std::vector<Button>();
    buttons.push_back(createButton(sf::Vector2f(10.0f, 600.0f), "Update Octave", sf::Vector2f(150.0f, 30.0f), []() {octaveCount = stoi(textboxes.at(0).text); }, &font));
    buttons.push_back(createButton(sf::Vector2f(10.0f, 650.0f), "Generate", sf::Vector2f(150.0f, 30.0f), []() {regen = true; }, &font));
    buttons.push_back(createButton(sf::Vector2f(10.0f, 700.0f), "Seed", sf::Vector2f(150.0f, 30.0f), []() {seed = true; }, &font));
    buttons.push_back(createButton(sf::Vector2f(10.0f, 750.0f), "Update Bias", sf::Vector2f(150.0f, 30.0f), []() {bias = stof(textboxes.at(0).text); }, &font));
    buttons.push_back(createButton(sf::Vector2f(10.0f, 800.0f), "Mode", sf::Vector2f(150.0f, 30.0f), []() {mode += 1; if (mode == 2) mode = 0; }, &font));

    for (int i = 0; i < window.getSize().x * window.getSize().y * 4; i += 4) {
        pixels[i] = 0;      //r
        pixels[i + 1] = 0;  //g
        pixels[i + 2] = 0;  //b
        pixels[i + 3] = 255;  //a
    }

    texture.update(pixels);
    texture2D.update(pixels);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                window.close();

            if (event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode < 128)
                {
                    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace))
                        typed += event.text.unicode;
                    else
                        back = true;
                }
            }
        }
        sf::Vector2i mPos = sf::Mouse::getPosition(window);
        UpdateButtons(&buttons, (sf::Vector2f)mPos);
        updateTextBoxes(&textboxes, (sf::Vector2f)mPos);

        sf::Text t;
        t.setFont(font);
        t.setFillColor(sf::Color::White);
        t.setCharacterSize(20);
        t.setPosition(900.0f, 650.0f);
        t.setString(std::to_string(octaveCount));

        sf::Text tb;
        tb.setFont(font);
        tb.setFillColor(sf::Color::White);
        tb.setCharacterSize(20);
        tb.setPosition(900.0f, 700.0f);
        tb.setString(std::to_string(bias));

        sf::Text m;
        m.setFont(font);
        m.setFillColor(sf::Color::White);
        m.setCharacterSize(20);
        m.setPosition(900.0f, 750.0f);
        m.setString(std::to_string(mode + 1) += "D");

        if (bias == 0.0f)
            bias += 0.1f;

        if (mode == 0)
        {
            if (seed)
            {
                for (size_t i = 0; i < outputSize; i++)
                    noiseSeed1D[i] = (float)rand() / (float)RAND_MAX;
                seed = false;
                regen = true;
            }
        }
        else if (mode == 1)
        {
            if (seed)
            {
                for (size_t i = 0; i < outputWidth * outputHeight; i++)
                    noiseSeed2D[i] = (float)rand() / (float)RAND_MAX;
                seed = false;
                regen = true;
            }
        }

        if (regen)
        {
            if (mode == 0)
            {
                PerlinNoise1D(outputSize, noiseSeed1D, octaveCount, perlinNoise1D, bias);
                firstGen = false;
                regen = false;
            }

            if (mode == 1)
            {
                PerlinNoise2D(outputWidth, outputHeight, noiseSeed2D, octaveCount, perlinNoise2D, bias);
                firstGen = false;
                regen = false;
            }
        }

        if(mode == 0)
            for (size_t i = 0; i < window.getSize().x * window.getSize().y * 4; i += 4)
            {
                pixels[i] = 0;      //r
                pixels[i + 1] = 0;  //g
                pixels[i + 2] = 0;  //b
                pixels[i + 3] = 255;  //a
            }

        if(mode == 1)
            for (size_t i = 0; i < outputWidth * outputHeight * 4; i += 4)
            {
                pixels2D[i] = 0;      //r
                pixels2D[i + 1] = 0;  //g
                pixels2D[i + 2] = 0;  //b
                pixels2D[i + 3] = 255;  //a
            }

        if (mode == 0)
        {
            if (!firstGen)
            {
                for (int i = 0; i < outputSize; i++)
                {
                    int y = -(perlinNoise1D[i] * (float)window.getSize().y / 2.0f) + (float)window.getSize().y / 2.0f;
                    for (int k = y; k < window.getSize().y / 2.0f; k++)
                    {
                        setPixel(i, k, sf::Color::Green, pixels, window.getSize().y);
                    }
                }
            }
        }

        if (mode == 1)
        {
            if (!firstGen)
            {
                for (size_t i = 0; i < outputWidth; i++)
                {
                    for (size_t j = 0; j < outputHeight; j++)
                    {
                        float pixelColour = (perlinNoise2D[j * outputWidth + i] * 255);
                        sf::Color c;
                        c.r = 255;
                        c.g = 255;
                        c.b = 255;
                        c.a = pixelColour;
                        setPixel(i, j, c, pixels2D, outputHeight);
                    }
                    
                }
            }
        }

        if (mode == 0)
        {
            texture.update(pixels);
            sprite.setTexture(texture);
        }
        else if (mode == 1)
        {
            texture2D.update(pixels2D);
            sprite2D.setTexture(texture2D);
        }

        window.clear();
        if (mode == 0)
            window.draw(sprite);
        else if (mode == 1)
            window.draw(sprite2D);
        DrawButtons(&buttons, &window);
        drawTextBoxes(&textboxes, &window);
        window.draw(t);
        window.draw(tb);
        window.draw(m);
        window.display();

        currentTime = clock.getElapsedTime();
        fps = 1.0f / (currentTime.asSeconds() - previousTime.asSeconds());
        std::string f = "Perlin Noise fps:";
        f.append(std::to_string(floor(fps)));
        window.setTitle(f);
        previousTime = currentTime;
    }

    return 0;
}

void PerlinNoise1D(int count, float* seed, int octaves, float* output, float bias)
{
    for (size_t i = 0; i < count; i++)
    {
        float noise = 0.0f;
        float scale = 1.0f;
        float scaleAcc = 0.0f;

        for (size_t j = 0; j < octaves; j++)
        {
            int pitch = count >> j;
            int sample1 = (i / pitch) * pitch;
            int sample2 = (sample1 + pitch) % count;

            float blend = (float)(i - sample1) / (float)pitch;
            float sample = (1.0f - blend) * seed[sample1] + blend * seed[sample2];
            noise += sample * scale;
            scaleAcc += scale;
            scale = scale / bias;
        }

        output[i] = noise / scaleAcc;
    }
}

void PerlinNoise2D(int width, int height, float* seed, int octaves, float* output, float bias)
{
    for (size_t i = 0; i < width; i++)
    {
        for (size_t j = 0; j < height; j++)
        {
            float noise = 0.0f;
            float scale = 1.0f;
            float scaleAcc = 0.0f;

            for (size_t k = 0; k < octaves; k++)
            {
                int pitch = width >> k;
                int sampleX1 = (i / pitch) * pitch;
                int sampleY1 = (j / pitch) * pitch;

                int sampleX2 = (sampleX1 + pitch) % width;
                int sampleY2 = (sampleY1 + pitch) % width;

                float blendX = (float)(i - sampleX1) / (float)pitch;
                float blendY = (float)(j - sampleY1) / (float)pitch;
                float sampleX = (1.0f - blendX) * seed[sampleY1 * width + sampleX1] + blendX * seed[sampleY1 * width + sampleX2];
                float sampleY = (1.0f - blendX) * seed[sampleY2 * width + sampleX1] + blendX * seed[sampleY2 * width + sampleX2];
                scaleAcc += scale;
                noise += (blendY * (sampleY - sampleX) + sampleX) * scale;
                scale = scale / bias;
            }

            output[j * width + i] = noise / scaleAcc;
        }
    }
}

void setPixel(int x, int y, sf::Color color, sf::Uint8* pixels, int col)
{
    pixels[((y * col + x) * 4) + 0] = color.r;
    pixels[((y * col + x) * 4) + 1] = color.g;
    pixels[((y * col + x) * 4) + 2] = color.b;
    pixels[((y * col + x) * 4) + 3] = color.a;
}

void loadFont(std::string path, sf::Font* font)
{
    if (!font->loadFromFile(path))
    {
        std::cout << "Error" << std::endl;
    }
}

void DrawButtons(std::vector<Button>* buttons, sf::RenderWindow* window)
{
    for (size_t i = 0; i < buttons->size(); i++)
    {
        sf::RectangleShape r;
        sf::Color c;
        if (buttons->at(i).state == 0)
        {
            c.r = 137;
            c.g = 137;
            c.b = 137;
        }
        else if (buttons->at(i).state == 1)
        {
            c.r = 100;
            c.g = 100;
            c.b = 100;
        }
        else if (buttons->at(i).state == 2)
        {
            c.r = 50;
            c.g = 50;
            c.b = 50;
        }
        r.setFillColor(c);
        r.setSize(buttons->at(i).size);
        r.setPosition(buttons->at(i).pos);
        sf::Text t;
        t.setFillColor(sf::Color::Black);
        t.setFont(*buttons->at(i).font);
        t.setCharacterSize(20);
        t.setString(buttons->at(i).text);
        t.setPosition(buttons->at(i).pos);
        window->draw(r);
        window->draw(t);
    }
}

void drawTextBoxes(std::vector<TextBox>* textboxes, sf::RenderWindow* window)
{
    for (size_t i = 0; i < textboxes->size(); i++)
    {
        sf::RectangleShape r;
        sf::Color c;
        c.r = 100;
        c.b = 100;
        c.g = 100;
        r.setFillColor(c);
        r.setSize(textboxes->at(i).size);
        r.setPosition(textboxes->at(i).pos);
        sf::Text t;
        t.setFillColor(sf::Color::Black);
        t.setFont(*textboxes->at(i).font);
        t.setCharacterSize(20);
        t.setString(textboxes->at(i).text);
        t.setPosition(textboxes->at(i).pos);
        window->draw(r);
        window->draw(t);
        if (textboxes->at(i).state == 1)
        {
            sf::Color cl;
            cl.r = 50;
            cl.g = 50;
            cl.b = 50;
            sf::RectangleShape e;
            e.setFillColor(cl);
            e.setSize(sf::Vector2f(2, textboxes->at(i).size.y));
            e.setPosition(textboxes->at(i).pos);
            window->draw(e);
            textboxes->at(i).state = 0;
        }
    }
}

void UpdateButtons(std::vector<Button>* buttons, sf::Vector2f mPos)
{
    for (size_t i = 0; i < buttons->size(); i++)
    {
        sf::Vector2f bPos = buttons->at(i).pos;
        sf::Vector2f bSize = buttons->at(i).size;
        if (bPos.x <= mPos.x && bPos.y <= mPos.y && bPos.x + bSize.x >= mPos.x && bPos.y + bSize.y >= mPos.y)
        {
            buttons->at(i).state = 1;
            
            if (buttons->at(i).held == true && !sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                buttons->at(i).function();
                buttons->at(i).state = 2;
                buttons->at(i).held = false;
            }

            if (buttons->at(i).held == false)
            {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    buttons->at(i).held = true;
                    buttons->at(i).state = 2;
                }
            }

            if (buttons->at(i).held == true)
                buttons->at(i).state = 2;
        }
        else 
        {
            buttons->at(i).state = 0;
        }
    }
}

void updateTextBoxes(std::vector<TextBox>* textboxes, sf::Vector2f mPos)
{
    for (size_t i = 0; i < textboxes->size(); i++)
    {
        sf::Vector2f tPos = textboxes->at(i).pos;
        sf::Vector2f tSize = textboxes->at(i).size;
        if (textboxes->at(i).selected)
        {
            if (textboxes->at(i).time != textboxes->at(i).blinkingSpeed)
            {
                textboxes->at(i).time++;
            }
            else
            {
                textboxes->at(i).time = 0;
                if (textboxes->at(i).state == 1)
                    textboxes->at(i).state = 0;
                else
                    textboxes->at(i).state = 1;
            }

            if (typed != "")
            {
                textboxes->at(i).text += typed;
                typed = "";
            }

            if (back)
            {
                if (textboxes->at(i).text.size() > 0)
                {
                    textboxes->at(i).text.erase(textboxes->at(i).text.size() - 1, 1);
                }
                back = false;
            }
        }

        if (tPos.x <= mPos.x && tPos.y <= mPos.y && tPos.x + tSize.x >= mPos.x && tPos.y + tSize.y >= mPos.y)
        {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                textboxes->at(i).selected = true;
                textboxes->at(i).time = textboxes->at(i).blinkingSpeed;
            }
        }
        else
        {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                textboxes->at(i).selected = false;
            }
        }
    }
}

Button createButton(sf::Vector2f pos, std::string text, sf::Vector2f size, void(*function)(), sf::Font* font)
{
    Button btn;
    btn.pos = pos;
    btn.text = text;
    btn.function = function;
    btn.size = size;
    btn.state = 0;
    btn.held = false;
    btn.font = font;
    return btn;
}

TextBox createTextBox(sf::Vector2f pos, std::string text, sf::Vector2f size, sf::Font* font, int blinkingSpeed)
{
    TextBox b;
    b.pos = pos;
    b.text = text;
    b.size = size;
    b.font = font;
    b.blinkingSpeed = blinkingSpeed;
    b.state = 0;
    b.selected = false;
    b.time = 0;
    return b;
}
