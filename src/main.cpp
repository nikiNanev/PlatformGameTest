#define SDL_MAIN_USE_CALLBACKS // This is necessary for the new callbacks API. To use the legacy API, don't define this.
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_init.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#include <cmath>
#include <string_view>
#include <filesystem>
#include <thread>
#include <iostream>

#include <memory>

struct PlayerTexture
{
    SDL_Texture *idleTex;
    SDL_FRect srcIdle, destIdle;
};

struct AppContext
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    PlayerTexture *playerTexture;
    int iteration;
    SDL_AppResult app_quit = SDL_APP_CONTINUE;
};

struct Window
{
    uint32_t width, height;
};

SDL_AppResult SDL_Fail()
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    // init the library, here we make a window so we only need the Video capabilities.
    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "[Error]", "Could not initialize video and audio", nullptr);
        return SDL_Fail();
    }

    // init TTF
    if (not TTF_Init())
    {
        return SDL_Fail();
    }

    // create a window
    auto winProps = std::make_unique<struct Window>(Window{.width = 600, .height = 600});
    SDL_Window *window = SDL_CreateWindow("Tiger Sample", winProps->width, winProps->height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    if (not window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "[Error]", "The window was not initialized!", window);
        return SDL_Fail();
    }

    // create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (not renderer)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "[Error]", "The renderer was not initialized!", window);
        return SDL_Fail();
    }

    int logW = 640, logH = 320;
    SDL_SetRenderLogicalPresentation(renderer, logW, logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    // load the font
#if __ANDROID__
    std::filesystem::path basePath = ""; // on Android we do not want to use basepath. Instead, assets are available at the root directory.
#else
    auto basePathPtr = SDL_GetBasePath();
    if (not basePathPtr)
    {
        return SDL_Fail();
    }
    const std::filesystem::path basePath = basePathPtr;
#endif

    // load the idle
    auto idle_surface = IMG_Load((basePath / "idle.png").string().c_str());
    SDL_Texture *idleTexture = SDL_CreateTextureFromSurface(renderer, idle_surface);

    SDL_SetTextureScaleMode(idleTexture, SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(idle_surface);

    // print some information about the window
    SDL_ShowWindow(window);
    {
        int width, height, bbwidth, bbheight;
        SDL_GetWindowSize(window, &width, &height);
        SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
        SDL_Log("Window size: %ix%i", width, height);
        SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
        if (width != bbwidth)
        {
            SDL_Log("This is a highdpi environment.");
        }
    }

    auto playerTexture = new PlayerTexture();
    playerTexture->idleTex = idleTexture;

    const SDL_FRect srcImageTex{
        .x = 0,
        .y = 0,
        .w = 32,
        .h = 32};

    const SDL_FRect destImageTex{
        .x = 0,
        .y = 0,
        .w = 32,
        .h = 32};

    playerTexture->srcIdle = srcImageTex;
    playerTexture->destIdle = destImageTex;

    // set up the application data
    *appstate = new AppContext{
        .window = window,
        .renderer = renderer,
        .playerTexture = playerTexture,
        .iteration = 0};

    SDL_SetRenderVSync(renderer, -1); // enable vysnc

    SDL_Log("Application started successfully!");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    auto *app = (AppContext *)appstate;

    if (event->type == SDL_EVENT_QUIT)
    {
        app->app_quit = SDL_APP_SUCCESS;
    }

    switch (event->type)
    {
    case SDL_EVENT_KEY_DOWN:
    {
        if (event->key.key == SDLK_ESCAPE)
        {
            app->app_quit = SDL_APP_SUCCESS;
        }
    }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    auto *app = (AppContext *)appstate;

    // draw a color
    auto time = SDL_GetTicks() / 1000.f;

    int seconds = static_cast<int>(std::round(time));

    if (seconds > app->iteration)
    {
        std::cout << "time: " << static_cast<int>(std::round(time)) << std::endl;
        app->iteration++;
    }

    SDL_Color bgColor = {.r = 10, .g = 40, .b = 30, .a = 255};

    SDL_SetRenderDrawColor(app->renderer, bgColor.r, bgColor.g, bgColor.b, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app->renderer);

    // Renderer uses the painter's algorithm to make the text appear above the image, we must render the image first.
    SDL_RenderTexture(app->renderer, app->playerTexture->idleTex, &app->playerTexture->srcIdle, &app->playerTexture->destIdle);

    SDL_RenderPresent(app->renderer);

    return app->app_quit;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    auto *app = (AppContext *)appstate;
    if (app)
    {
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        SDL_DestroyTexture(app->playerTexture->idleTex);

        delete app;
    }
    TTF_Quit();

    SDL_Log("Application quit successfully!");
    SDL_Quit();
}