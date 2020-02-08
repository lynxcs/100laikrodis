#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <ctime>
#include <chrono>

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

const char* FONT_PATH = "arial.ttf";
const int FONT_SIZE = 250;

SDL_Window* gWindow = NULL;
/* SDL_Surface* gScreenSurface = NULL; */
SDL_Surface* gHelloWorld = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;

// Days Remaining {{{
int gDaysRemaining = 0;
// date in format: YYYY-MM-DD
int getDaysRemaining(std::string date)
{
    std::string year = date.substr(0, 4);
    std::string month = date.substr(5, 2);
    std::string day = date.substr(8, std::string::npos);

    int y = stoi(year);
    int m = stoi(month);
    int d = stoi(day);

    tm then = {0};
    then.tm_year = y - 1900;
    then.tm_mon = m - 1;
    then.tm_mday = d;
    time_t then_secs = mktime(&then);

    time_t time_now = time(0);
    tm* now = localtime(&time_now);
    tm today = {0};
    today.tm_year = now->tm_year;
    today.tm_mon = now->tm_mon;
    today.tm_mday = now->tm_mday;
    time_t today_secs = mktime(&today);

    return (today_secs - then_secs) / (24*60*60) * -1;
}
// }}}
// Media loading{{{
bool loadMedia()
{
    gFont = TTF_OpenFont(FONT_PATH, FONT_SIZE);

    if (gFont == NULL)
        return false;

    return true;
}
// }}}
// Keypress handling {{{
bool test = false;
void handle_keyPresses(SDL_Event& e, bool& quit)
{
    while(SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
        {
            quit = true;
        }
        else if (e.type == SDL_KEYDOWN)
        {
            switch (e.key.keysym.sym)
            {
                case SDLK_SPACE:
                    test = true;
                    break;
                case SDLK_ESCAPE:
                    quit = true;
                    break;
            }
        }
    }
}
// }}}
// Color functions {{{
// Get new color
SDL_Color getColor()
{
    static int x = 0;
    SDL_Color color;
    x = x % 8;
    switch(x)
    {
        case 0:
            color = { 190, 0, 255 };
            break;
        case 1:
            color = { 255, 0, 139 };
            break;
        case 2:
            color = { 255, 131, 0 };
            break;
        case 3:
            color = { 0, 38, 255 };
            break;
        case 4:
            color = { 255, 250, 0 };
            break;
        case 5:
            color = { 255, 38, 0 };
            break;
        case 6:
            color = { 38, 255, 0 };
            break;
        case 7:
            color = { 0, 254, 255 };
            break;
    }
    x++;
    return color;
}

SDL_Color getColor(SDL_Color oldColor)
{
    return getColor();
}
// }}}
// Box class {{{
// Bounding box, that bounces around the play area.
// x,y is the top left corner
class Box
{
    private:
        int x, y, width, height = 0;
        int text_width = 0, text_height = 0;
        int screen_width = SCREEN_WIDTH;
        int screen_height = SCREEN_HEIGHT;
        float speed;
        SDL_Color color;
        SDL_Surface* surface;
        SDL_Texture* texture;

        bool UP = false;
        bool RIGHT = true;
    public:
        Box(float speed) : speed(speed) {
            color = getColor();
            surface = TTF_RenderText_Solid(gFont,
                    std::to_string(gDaysRemaining).c_str(), color);
            TTF_SizeText(gFont, std::to_string(gDaysRemaining).c_str(), &text_width, &text_height);

            texture = SDL_CreateTextureFromSurface(gRenderer, surface);
            int texW = 0;
            int texH = 0;
            SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
            width = texW;
            height = texH;
        }

        void update(double deltaTime)
        {
            SDL_Rect r = getTextRect();
            if(x+text_width >= screen_width)
            {
                RIGHT = false;
                color = getColor(color);
                recreateTexture();
            }
            else if (x <= 0)
            {
                RIGHT = true;
                color = getColor(color);
                recreateTexture();
            }

            if(r.y + r.h >= screen_height)
            {
                UP = true;
                color = getColor(color);
                recreateTexture();
            }
            else if (r.y <= 0)
            {
                UP = false;
                color = getColor(color);
                recreateTexture();
            }

            if(UP)
            {
                y -= speed * deltaTime;
            }
            else
            {
                y += speed * deltaTime;
            }

            if(RIGHT)
            {
                x += speed * deltaTime;
            }
            else
            {
                x -= speed * deltaTime;
            }
        }

        SDL_Rect getRect()
        {
            return {x, y, width, height};
        }

        SDL_Color currentColor()
        {
            return color;
        }

        SDL_Rect getTextRect()
        {
            return {x, y+50, width, height-100};
        }

        void setRect(SDL_Rect rect)
        {
            x = rect.x;
            y = rect.y;
            width = rect.w;
            height = rect.h;
        }

        void setDimensions(int w, int h)
        {
            width = w;
            height = h;
        }

        SDL_Texture* getTexture()
        {
            return texture;
        }

        SDL_Surface* getSurface()
        {
            return surface;
        }

        void recreateTexture()
        {
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(surface);

            surface = TTF_RenderText_Solid(gFont, std::to_string(gDaysRemaining).c_str(), color);
            TTF_SizeText(gFont, std::to_string(gDaysRemaining).c_str(), &text_width, &text_height);

            texture = SDL_CreateTextureFromSurface(gRenderer, surface);
            int texW = 0;
            int texH = 0;
            SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
            width = texW;
            height = texH;
        }
};
// }}}
// SDL Initialization {{{
bool init()
{
    bool success = true;
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        gWindow = SDL_CreateWindow("SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            /* gScreenSurface = SDL_GetWindowSurface(gWindow); */
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (gRenderer == NULL)
            {
                printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
                TTF_Init();
            }
        }
    }

    return success;
}
// }}}
// SDL Destruction {{{
void close()
{
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    TTF_CloseFont(gFont);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
// }}}

bool testFlagToggled = false;
int main(int argc, char* args[] )
{
    if (argc < 3)
    {
        printf("USAGE: ./a.out $YYYY-$MM-$DD $SPEED\n");
        printf("Example: ./a.out 2020-03-12 3");
    }
    srandom(time(NULL));
    gDaysRemaining = getDaysRemaining(args[1]);
    float speed = std::stof(args[2]);
    printf("Days remaining: %d\n", gDaysRemaining);
    if (!init())
    {
        printf("Failed to initilize!\n");
    }
    else
    {
        if (!loadMedia())
        {
            printf("Failed to load media!\n");
        }
        else
        {
            // Main loop flag
            bool quit = false;

            // Event handler
            SDL_Event e;
            uint64_t NOW = SDL_GetPerformanceCounter();
            uint64_t LAST = 0;
            double deltaTime = 0;
            /* SDL_Color color = { 255, 255, 255 }; */
            SDL_Color color = getColor();
            std::string days = std::to_string(gDaysRemaining);
            Box box(speed);
            while(!quit)
            {
                if (test || getDaysRemaining(args[1]) != gDaysRemaining)
                {
                    if (test)
                    {
                        testFlagToggled = true;
                        test = false;
                        gDaysRemaining--;
                        /* color = {255, 0, 0}; */
                        color = getColor(color);
                    }
                    else if (!testFlagToggled)
                    {
                        gDaysRemaining = getDaysRemaining(args[1]);
                    }
                    days = std::to_string(gDaysRemaining);

                    box.recreateTexture();
                }

                LAST = NOW;
                NOW = SDL_GetPerformanceCounter();
                deltaTime = (double)((NOW - LAST)*1000 / (double)SDL_GetPerformanceFrequency() );

                handle_keyPresses(e, quit);

                box.update(deltaTime);

                // Clear the screen
                SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
                SDL_RenderClear(gRenderer);

                SDL_Rect rect = box.getRect();
                SDL_Rect text_rect = box.getTextRect();

                SDL_Rect ellipse_rect;
                ellipse_rect.x = text_rect.x + text_rect.w / 2;
                ellipse_rect.y = text_rect.y + text_rect.h + 25;
                ellipse_rect.w = text_rect.w / 2;
                ellipse_rect.h = 50;
                aaellipseRGBA(gRenderer, ellipse_rect.x, ellipse_rect.y, ellipse_rect.w, ellipse_rect.h, 200, 150, 2, 255);
                filledEllipseRGBA(gRenderer, ellipse_rect.x, ellipse_rect.y, ellipse_rect.w, ellipse_rect.h, 200, 150, 2, 255);
                if (testFlagToggled)
                {
                    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                    SDL_RenderFillRect(gRenderer, &text_rect);
                }


                SDL_UpdateWindowSurface(gWindow);

                SDL_RenderCopy(gRenderer, box.getTexture(), NULL, &rect);
                SDL_RenderPresent(gRenderer);

                SDL_Delay(8);
            }
            SDL_DestroyTexture(box.getTexture());
            SDL_FreeSurface(box.getSurface());
        }
    }

    close();
    return 0;
}
