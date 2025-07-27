#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_main.h>


typedef struct {
    int64_t currentFrame = 0;
    int64_t currentFrameTick = 0;
    bool debug_enabled = false;
    float player_x;
    float player_y;
} GameState;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    GameState game_state;
} AppState;

static TTF_Font *font = NULL;

void handle_key_press(GameState *game_state, const SDL_KeyboardEvent key) {

    switch (key.scancode) {
        case SDL_SCANCODE_W:
            game_state->player_y = game_state->player_y - 10;
            break;
        case SDL_SCANCODE_S:
            game_state->player_y = game_state->player_y + 10;
            break;
        case SDL_SCANCODE_A:
            game_state->player_x = game_state->player_x - 10;
            break;
        case SDL_SCANCODE_D:
            game_state->player_x = game_state->player_x + 10;
            break;
        case SDL_SCANCODE_F1:
            game_state->debug_enabled = !game_state->debug_enabled;
            break;
        default:
            break;
    }
}

typedef struct {
    int w;
    int h;
} Viewport;

void render_text_at(SDL_Renderer* renderer, const char* text, float x, float y) {

    SDL_Texture* texture;
    SDL_FRect textRect = {x, y, 0, 0};
    SDL_Surface* outputText = TTF_RenderText_Blended(font, text, 0, {255, 255, 255, 0});
    if (outputText) {
        texture= SDL_CreateTextureFromSurface(renderer, outputText);
        SDL_DestroySurface(outputText);
    }
    SDL_GetTextureSize(texture, &textRect.w, &textRect.h);
    SDL_RenderTexture(renderer, texture, NULL, &textRect);
}

void render_debug(GameState* game_state, SDL_Renderer* renderer) {
    Viewport viewport = { 0, 0 };
    int frameHeight = 200;
    int frameWidth = 500;
    SDL_GetRenderOutputSize(renderer, &viewport.w, &viewport.h);
    SDL_FRect frame = { (float)viewport.w - frameWidth, 0, (float)frameWidth, (float)frameHeight };
    SDL_RenderRect(renderer, &frame);
    std::string currentFrameString = "Current frame: " + std::to_string(game_state->currentFrame);
    std::string currentFrameTickString = "Current frame tick: " + std::to_string(game_state->currentFrameTick);

    const char* currentFrameText = currentFrameString.c_str();
    const char* currentFrameTickText = currentFrameTickString.c_str();
    render_text_at(renderer, currentFrameText, frame.x + 5, frame.y + 5);
    render_text_at(renderer, currentFrameTickText, frame.x + 5, frame.y + 45);

}
/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Init app state
    AppState *as = (AppState *) SDL_calloc(1, sizeof(AppState));
    if (!as) {
        return SDL_APP_FAILURE;
    }
    *appstate = as;
    GameState game_state = as->game_state;

    if (!SDL_CreateWindowAndRenderer("examples/renderer/clear", 640, 480, SDL_WINDOW_BORDERLESS + SDL_WINDOW_FULLSCREEN,
                                     &as->window, &as->renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Init Player
    game_state.player_x = 50;
    game_state.player_y = 50;

    // Init fonts
    if (!TTF_Init()) {
        SDL_Log("Couldn't initialise SDL_ttf: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    font = TTF_OpenFont("/usr/share/fonts/noto/NotoSans-Light.ttf", 32.0f);
    if (!font) {
        SDL_Log("Couldn't open font: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE; /* carry on with the program! */
}


/* This function runs once per frame, and is the heart of the program. */
uint64_t completedFrame;


SDL_AppResult SDL_AppIterate(void *appstate) {
    uint64_t currentTick = SDL_GetTicks();
    uint64_t currentFrameTick = currentTick - (currentTick % 10);
    if (currentFrameTick > completedFrame) {
        completedFrame = currentFrameTick;
        std::cout << currentFrameTick << std::endl;

        AppState *as = (AppState *) appstate;
        GameState* game_state = &as->game_state;

        game_state->currentFrame++;
        game_state->currentFrameTick = currentFrameTick;

        // Clear screen
        SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); /* black, full alpha */
        SDL_RenderClear(as->renderer); /* start with a blank canvas. */

        // Render Player
        SDL_FRect rect = {game_state->player_x, game_state->player_y, 40, 40};
        SDL_SetRenderDrawColor(as->renderer, 255, 0, 0, SDL_ALPHA_OPAQUE); /* black, full alpha */
        SDL_RenderRect(as->renderer, &rect);

        // Render Hello World
        SDL_FRect textRect = {100, 100, 0, 0};
        static SDL_Texture *texture = NULL;
        SDL_Surface *text;
        text = TTF_RenderText_Blended(font, "Hello World!", 0, {0, 255, 0, 0});
        if (text) {
            texture = SDL_CreateTextureFromSurface(as->renderer, text);
            SDL_DestroySurface(text);
        }
        SDL_GetTextureSize(texture, &textRect.w, &textRect.h);
        SDL_RenderTexture(as->renderer, texture, NULL, &textRect);

        //Render Debug
        if (game_state->debug_enabled) {
            render_debug(game_state, as->renderer);
        }

        // Draw
        SDL_RenderPresent(as->renderer);
    } else {
        SDL_Delay(SDL_GetTicks() - currentFrameTick);
    }

    return SDL_APP_CONTINUE; /* carry on with the program! */
}


/* This function runs when a new event (mouse input, keypresses, etc.) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {

    GameState *game_state = &((AppState *) appstate)->game_state;

    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        case SDL_EVENT_KEY_DOWN:
            if (event->key.scancode == SDL_SCANCODE_Q) {
                return SDL_APP_SUCCESS;
            }
            handle_key_press(game_state, event->key);
        default:
            break;
    }
    return SDL_APP_CONTINUE; /* carry on with the program! */
}
/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    /* SDL will clean up the window/renderer for us. */
}
