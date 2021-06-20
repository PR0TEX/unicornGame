#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}
#define LEVEL_WIDTH 2000
#define LEVEL_HEIGHT 480
#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480
#define AM_OBST 11
#define JUMP_SIZE 15
#define SKIP_SIZE 5
#define DASH_TIME 3
#define DASH_POWER 1
#define JUMP_TIME 2.2
#define JUMP_POWER 3
#define AM_NUM 3


// narysowanie napisu txt na powierzchni screen, zaczynając od punktu (x, y)
// charset to bitmapa 128x128 zawierająca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
        SDL_Surface* charset) {
        int px, py, c;
        SDL_Rect s, d;
        s.w = 8;
        s.h = 8;
        d.w = 8;
        d.h = 8;
        while (*text) {
                c = *text & 255;
                px = (c % 16) * 8;
                py = (c / 16) * 8;
                s.x = px;
                s.y = py;
                d.x = x;
                d.y = y;
                SDL_BlitSurface(charset, &s, screen, &d);
                x += 8;
                text++;
        };
};

// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt środka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
        SDL_Rect dest;
        dest.x = x - sprite->w / 2;
        dest.y = y - sprite->h / 2;
        dest.w = sprite->w;
        dest.h = sprite->h;
        SDL_BlitSurface(sprite, NULL, screen, &dest);
};

// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
        if (x < SCREEN_WIDTH && x >= 0)
        {

                int bpp = surface->format->BytesPerPixel;
                Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
                *(Uint32*)p = color;

        }
};

// rysowanie linii o długości l w pionie (gdy dx = 0, dy = 1)
// bądź poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
        for (int i = 0; i < l; i++) {
                DrawPixel(screen, x, y, color);
                x += dx;
                y += dy;
        };
};

// rysowanie prostokąta o długości boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
        Uint32 outlineColor, Uint32 fillColor) {
        int i;
        DrawLine(screen, x, y, k, 0, 1, outlineColor);
        DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
        DrawLine(screen, x, y, l, 1, 0, outlineColor);
        DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
        for (i = y + 1; i < y + k - 1; i++)
                DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};
//tworzenie przeszkod
SDL_Rect setObstacle(int x, int y, int w, int h)
{
        SDL_Rect obstacle;
        obstacle.x = x;
        obstacle.y = y;
        obstacle.w = w;
        obstacle.h = h;

        return obstacle;
}

// zapisywanie punktow do pliku
void saveFile(int allpoints)
{
        FILE* file;
        file = fopen("wyniki.txt", "r");
        if (file == NULL)
        {
                file = fopen("wyniki.txt", "w");
                fprintf(file, "%d \n", allpoints);
        }
        else
        {
                file = fopen("wyniki.txt", "a");
                fprintf(file, "%d \n", allpoints);
        }
        fclose(file);
}
// resetowanie wartosci w grze
void reset(int& x, int& y, SDL_Rect& player, int& scrollingOffset, int& allPoints, int& pkt)
{
        x = player.w / 2;
        y = player.h / 2;
        scrollingOffset = 0;
        player.x = 0;
        player.y = 0;
        allPoints += pkt;
        pkt = 0;
}
// ekran w wyborze resetowania
void con(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Texture* scrtex, SDL_Surface* charset,int& quit)
{
        int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
        char text[128];
        SDL_Event event;
        DrawRectangle(screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, czarny, czarny);
        sprintf(text, "PRESS n TO RESPAWN");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
        sprintf(text, "PRESS esc TO QUIT");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 36, text, charset);
        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        //              SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);
        while (true)
        {
                while (SDL_PollEvent(&event)) {
                        switch (event.type) {
                        case  SDL_KEYDOWN:
                                if (event.key.keysym.sym == SDLK_n) {
                                        return;
                                }
                                else if (event.key.keysym.sym == SDLK_ESCAPE) {
                                        quit = 1;
                                        return;
                                }
                                break;
                        default:
                                break;
                        }
                }
        }
}
//wyswietlanie menu koncowego z opcja n - nowa gra, esc - wyjscie z gry, p - zapisanie wyniku do pliku
void endMenu(SDL_Surface* screen, SDL_Renderer* renderer, SDL_Texture* scrtex, SDL_Surface* charset, int& allPoints, int& quit, int& num_life)
{
        char text[128];
        int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
        SDL_Event event;
        DrawRectangle(screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, czarny, czarny);
        sprintf(text, "MENU");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
        sprintf(text, "Press n to start a new game");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 36, text, charset);
        sprintf(text, "Press esc to quit");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 46, text, charset);
        sprintf(text, "Press p to save result");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 56, text, charset);
        sprintf(text, "All points: %d", allPoints);
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 70, text, charset);
        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        //              SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);
        while (true)
        {

                while (SDL_PollEvent(&event)) {
                        switch (event.type) {
                        case  SDL_KEYDOWN:
                                if (event.key.keysym.sym == SDLK_ESCAPE) {
                                        quit = 1;
                                        return;
                                }
                                else if (event.key.keysym.sym == SDLK_n)
                                {
                                        num_life = AM_NUM;
                                        allPoints = 0;
                                        return;
                                }
                                else if (event.key.keysym.sym == SDLK_p) {
                                        saveFile(allPoints);
                                        break;
                                }
                                break;
                        case SDL_TEXTINPUT:
                                break;
                        default:
                                break;
                        }
                }
        }
}


// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
        int t1, t2, quit, restart, frames, rc, up, right, down, jumpAm, dashAm, pkt, allPoints;
        double delta, worldTime, fpsTimer, fps, unicornSpeed, dashFinish, jumpFinish;
        bool autoPlay, jump, dash;
        SDL_Event event;
        SDL_Surface* screen, * charset;
        SDL_Surface* unicorn, * bg, *heart;
        SDL_Texture* scrtex;
        SDL_Window* window;
        SDL_Renderer* renderer;

        int num_life = AM_NUM;
        bool ifTouch = false;

        // okno konsoli nie jest widoczne, jeżeli chcemy zobaczyć
        // komunikaty wypisywane printf-em trzeba w opcjach:
        // project -> szablon2 properties -> Linker -> System -> Subsystem
        // zmienić na "Console"
        // console window is not visible, to see the printf output
        // the option:
        // project -> szablon2 properties -> Linker -> System -> Subsystem
        // must be changed to "Console"
        printf("wyjscie printfa trafia do tego okienka\n");
        printf("printf output goes here\n");

        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
                printf("SDL_Init error: %s\n", SDL_GetError());
                return 1;
        }

        //tryb pełnoekranowy / fullscreen mode
        /*rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
                                                                 &window, &renderer);*/

        rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
                &window, &renderer);


        if (rc != 0) {
                SDL_Quit();
                printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
                return 1;
        };

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        SDL_SetWindowTitle(window, "UNICORN 2021");


        screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
                0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

        scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING,
                SCREEN_WIDTH, SCREEN_HEIGHT);



        // wyłączenie widoczności kursora myszy
        SDL_ShowCursor(SDL_DISABLE);

        // wczytanie obrazka cs8x8.bmp
        charset = SDL_LoadBMP("./cs8x8.bmp");
        if (charset == NULL) {
                printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
                SDL_FreeSurface(screen);
                SDL_DestroyTexture(scrtex);
                SDL_DestroyWindow(window);
                SDL_DestroyRenderer(renderer);
                SDL_Quit();
                return 1;
        };
        SDL_SetColorKey(charset, true, 0x000000);

        unicorn = SDL_LoadBMP("./skoczek1.bmp");
        if (unicorn == NULL) {
                printf("SDL_LoadBMP(skoczek1.bmp) error: %s\n", SDL_GetError());
                SDL_FreeSurface(charset);
                SDL_FreeSurface(screen);
                SDL_DestroyTexture(scrtex);
                SDL_DestroyWindow(window);
                SDL_DestroyRenderer(renderer);
                SDL_Quit();
                return 1;
        };

        bg = SDL_LoadBMP("./bg2.bmp");
        if (bg == NULL) {
                printf("SDL_LoadBMP(bg2.bmp) error: %s\n", SDL_GetError());
                SDL_FreeSurface(charset);
                SDL_FreeSurface(screen);
                SDL_DestroyTexture(scrtex);
                SDL_DestroyWindow(window);
                SDL_DestroyRenderer(renderer);
                SDL_Quit();
                return 1;
        };
        heart = SDL_LoadBMP("./serduszko.bmp");
        if (heart == NULL) {
                printf("SDL_LoadBMP(serduszko.bmp) error: %s\n", SDL_GetError());
                SDL_FreeSurface(charset);
                SDL_FreeSurface(screen);
                SDL_DestroyTexture(scrtex);
                SDL_DestroyWindow(window);
                SDL_DestroyRenderer(renderer);
                SDL_Quit();
                return 1;
        };
        char text[128];
        int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
        int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
        int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
        int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
        int bialy = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);

        t1 = SDL_GetTicks();

        frames = 0;
        fpsTimer = 0;
        fps = 0;
        quit = 0;
        restart = 0;
        up = 0;
        right = 0;
        allPoints = 0;
        down = 0;
        worldTime = 0;
        dashFinish = 0;
        jumpFinish = 0;
        unicornSpeed = 0.5;
        jumpAm = 2;
        dashAm = 1;
        pkt = 0;
        dash = false;
        jump = false;
        autoPlay = true;

        int x = unicorn->w / 2;
        int obX = unicorn->w * 3;
        int y = unicorn->h / 2;
        int scrollingOffset = 0;


        SDL_Rect obstacles[AM_OBST];

        //tworzenie przeszkod
        obstacles[0] = setObstacle(0, SCREEN_HEIGHT - 100, 400, 100);
        obstacles[1] = setObstacle(obX + 400, SCREEN_HEIGHT - 70, 140, 70);
        obstacles[2] = setObstacle(obX + 250, 0, 200, 120);
        obstacles[3] = setObstacle(obX + 500, SCREEN_HEIGHT - 150, 200, 150);
        obstacles[4] = setObstacle(obX + 400, SCREEN_HEIGHT - 100, 400, 100);
        obstacles[5] = setObstacle(obX + 500, 0, 60,240);
        obstacles[6] = setObstacle(obX + 700, 0, 150, 140);
        obstacles[7] = setObstacle(obX + 1000, SCREEN_HEIGHT - 200, 130, 200);
        obstacles[8] = setObstacle(obX + 1300, SCREEN_HEIGHT - 100, 300, 100);
        obstacles[9] = setObstacle(obX + 1500, 0, 200, 200);
        obstacles[10] = setObstacle(obX + 900, 0, 60, 200);

        SDL_Rect player;
        player.x = 0;
        player.y = 0;
        player.h = unicorn->h;
        player.w = unicorn->w;

        while (!quit) {
                t2 = SDL_GetTicks();

                // w tym momencie t2-t1 to czas w milisekundach,
                // jaki uplynał od ostatniego narysowania ekranu
                // delta to ten sam czas w sekundach
                // here t2-t1 is the time in milliseconds since
                // the last screen was drawn
                // delta is the same time in seconds
                delta = (t2 - t1) * 0.001;
                t1 = t2;

                worldTime += delta;

                SDL_FillRect(screen, NULL, bialy); //bg

                scrollingOffset--;
                if (dash) scrollingOffset -= DASH_POWER;
                if (scrollingOffset < -1 * bg->w)
                {
                        scrollingOffset = 0;
                }
                player.x = -scrollingOffset;
                pkt++;
        
                if (dashFinish <= worldTime && dash) {
                        dash = false;
                        jumpAm = 1;
                }

                for (int i = 0; i < AM_OBST; i++) {
                        while (SDL_HasIntersection(&player, &obstacles[i])) {
                                player.x--;
                                scrollingOffset++;
                        }
                }

                //sprawdzanie czy bedzie zderzenie z przeszkoda
                player.x += 1;
                for (int i = 0;i < AM_OBST;i++)
                {
                        if (SDL_HasIntersection(&player, &obstacles[i]))
                        {
                                num_life--;
                                reset(x, y, player, scrollingOffset, allPoints, pkt);
                                if (num_life > 0)
                                {
                                        con(screen, renderer, scrtex, charset,quit);
                                }
                                else
                                {
                                        endMenu(screen, renderer, scrtex, charset, allPoints, quit, num_life);
                                }
                                break;

                        }

                }
                player.x -= 1;

                DrawSurface(screen, bg, scrollingOffset, bg->h / 2);
                DrawSurface(screen, bg, scrollingOffset + bg->w, bg->h / 2);
                DrawSurface(screen, unicorn, x, y);
                
                //rysowanie przeszkod
                for (int i = 0; i < AM_OBST; i++)
                {
                        int temp = scrollingOffset;

                        if (-scrollingOffset + SCREEN_WIDTH > LEVEL_WIDTH + obstacles[i].x)
                        {
                                temp = scrollingOffset + LEVEL_WIDTH;
                        }
                        DrawRectangle(screen, temp + obstacles[i].x, obstacles[i].y, obstacles[i].w, obstacles[i].h, niebieski, czarny);
                }

                fpsTimer += delta;

                if (fpsTimer > 0.5) {
                        fps = frames * 2;
                        frames = 0;
                        fpsTimer -= 0.5;
                };

                // tekst informacyjny / info text
                DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 50, czerwony, niebieski);
                sprintf(text, "UNICORN, czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
                DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
                sprintf(text, "Esc - wyjscie, n - restart");
                DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
                sprintf(text, "Punkty: %d", pkt);
                DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 42, text, charset);
                for (int i = 0;i < num_life;i++)
                {
                        DrawSurface(screen, heart, SCREEN_WIDTH-(i+0.8) * heart->h, 2*heart->w / 2);
                }

                SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, scrtex, NULL, NULL);
                SDL_RenderPresent(renderer);

                // obsługa zdarzeń (o ile jakieś zaszły) / handling of events (if there were any)
                while (SDL_PollEvent(&event)) {
                        switch (event.type) {
                        case SDL_KEYDOWN:
                                if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
                                else if (event.key.keysym.sym == SDLK_n) restart = 1;
                                else if (event.key.keysym.sym == SDLK_RIGHT && !autoPlay) right = 1;
                                else if (event.key.keysym.sym == SDLK_UP && !autoPlay) up = 1;
                                else if (event.key.keysym.sym == SDLK_DOWN && !autoPlay) down = 1;
                                else if (event.key.keysym.sym == SDLK_d) {
                                        jump = false;
                                        dash = false;
                                        jumpAm = 1;
                                        dashAm = 1;
                                        dashFinish = 0;
                                        jumpFinish = 0;
                                        autoPlay = !autoPlay;
                                }
                                else if (event.key.keysym.sym == SDLK_z && autoPlay && !dash && !jump && jumpAm > 0) {
                                        jump = true;
                                        dashAm = 1;
                                        jumpAm--;
                                        jumpFinish = worldTime + JUMP_TIME;
                                }
                                else if (event.key.keysym.sym == SDLK_x && autoPlay && !dash && dashAm > 0) {
                                        dash = true;
                                        jump = false;
                                        dashAm = 0;
                                        dashFinish = worldTime + DASH_TIME;
                                }
                                break;
                        case SDL_KEYUP:
                                if (event.key.keysym.sym == SDLK_z) jump = false;
                                break;
                        case SDL_QUIT:
                                quit = 1;
                                break;
                        };
                };
                frames++;

                if (restart == 1)
                {
                        num_life--;
                        reset(x, y, player, scrollingOffset, allPoints, pkt);
                        if (num_life <= 0)
                        {
                                endMenu(screen, renderer, scrtex, charset, allPoints, quit, num_life);
                        }
                        
                        restart = 0;
                }
                if (right == 1)
                {
                        scrollingOffset -= SKIP_SIZE;

                        right = 0;
                }
                if (up == 1)
                {
                        y -= JUMP_SIZE;
                        player.y -= JUMP_SIZE;

                        if (player.y < 0)
                        {
                                y = player.h / 2;
                                player.y = 0;
                        }
                        for (int i = 0; i < AM_OBST; i++) {
                                while (SDL_HasIntersection(&player, &obstacles[i])) {
                                        y += 1;
                                        player.y += 1;
                                }
                        }
                        y += 1;
                        player.y += 1;

                        up = 0;

                }
                if (autoPlay && jump) {
                        if (worldTime > jumpFinish) jump = false;

                        y -= JUMP_POWER;
                        player.y -= JUMP_POWER;

                        if (player.y < 0)
                        {
                                y = player.h / 2;
                                player.y = 0;
                        }
                        for (int i = 0; i < AM_OBST; i++) {
                                while (SDL_HasIntersection(&player, &obstacles[i])) {
                                        y += 1;
                                        player.y += 1;
                                }
                        }
                        y += 1;
                        player.y += 1;
                }
                if (down == 1)
                {
                        y += JUMP_SIZE;
                        player.y += JUMP_SIZE;

                        if (player.y + player.h > SCREEN_HEIGHT)
                        {
                                /*y = SCREEN_HEIGHT - player.h / 2;
                                player.y = SCREEN_HEIGHT - player.h;*/
                                num_life--;
                                reset(x, y, player, scrollingOffset, allPoints, pkt);
                                if (num_life > 0)
                                {
                                        con(screen, renderer, scrtex, charset,quit);
                                }
                                else
                                {
                                        endMenu(screen, renderer, scrtex, charset, allPoints, quit, num_life);
                                        /*quit=1;*/
                                }


                        }


                        for (int i = 0; i < AM_OBST; i++) {
                                while (SDL_HasIntersection(&player, &obstacles[i])) {
                                        y -= 1;
                                        player.y -= 1;

                                }
                        }

                        y -= 1;
                        player.y -= 1;

                        down = 0;
                }
                if (autoPlay && !jump && !dash) {
                        y += JUMP_POWER;
                        player.y += JUMP_POWER;

                        if (player.y + player.h > SCREEN_HEIGHT)
                        {

                                jumpAm = 2;
                                num_life--;
                                reset(x, y, player, scrollingOffset, allPoints, pkt);
                                if (num_life > 0)
                                {
                                        con(screen, renderer, scrtex, charset,quit);
                                }
                                else
                                {
                                        endMenu(screen, renderer, scrtex, charset, allPoints, quit, num_life);
                                }
                        }

                        for (int i = 0; i < AM_OBST; i++) {
                                while (SDL_HasIntersection(&player, &obstacles[i])) {
                                        y -= 1;
                                        player.y -= 1;
                                        jumpAm = 2;
                                        dashAm = 1;
                                }
                        }

                        y -= 1;
                        player.y -= 1;

                }

        };

        // zwolnienie powierzchni / freeing all surfaces
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        SDL_Quit();
        return 0;
};
