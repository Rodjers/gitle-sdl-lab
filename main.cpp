#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_main.h>


typedef struct {
    float player_x;
    float player_y;
} GameState;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    GameState game_state;
    Uint64 last_step;
} AppState;

static TTF_Font *font = NULL;
static SDL_Texture *texture = NULL;

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
    }
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    AppState *as = (AppState *) SDL_calloc(1, sizeof(AppState));
    if (!as) {
        return SDL_APP_FAILURE;
    }

    // Init app state
    *appstate = as;

    GameState game_state = as->game_state;

    game_state.player_x = 50;
    game_state.player_y = 50;

    if (!SDL_CreateWindowAndRenderer("examples/renderer/clear", 640, 480, SDL_WINDOW_BORDERLESS + SDL_WINDOW_FULLSCREEN,
                                     &as->window, &as->renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

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
    /* Create the text */
    SDL_Surface *text;
    text = TTF_RenderText_Blended(font, "Hello World!", 0, {0, 255, 0, 0});
    if (text) {
        texture = SDL_CreateTextureFromSurface(as->renderer, text);
        SDL_DestroySurface(text);
    }
    if (!texture) {
        SDL_Log("Couldn't create text: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE; /* carry on with the program! */
}


/* This function runs once per frame, and is the heart of the program. */
uint64_t previousTick;

SDL_AppResult SDL_AppIterate(void *appstate) {
    uint64_t currentTick = SDL_GetTicks();
    if (currentTick - previousTick > 10) {
        previousTick = currentTick;
        std::cout << SDL_GetTicks() << std::endl;

        AppState *as = (AppState *) appstate;
        GameState game_state = as->game_state;

        // Clear screen
        SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); /* black, full alpha */
        SDL_RenderClear(as->renderer); /* start with a blank canvas. */

        SDL_FRect rect = {game_state.player_x, game_state.player_y, 40, 40};
        SDL_FRect textRect = {100, 100, 0, 0};
        SDL_GetTextureSize(texture, &textRect.w, &textRect.h);
        SDL_RenderTexture(as->renderer, texture, NULL, &textRect);
        SDL_SetRenderDrawColor(as->renderer, 255, 0, 0, SDL_ALPHA_OPAQUE); /* black, full alpha */

        SDL_RenderRect(as->renderer, &rect);

        // Draw
        SDL_RenderPresent(as->renderer);
    } else {
        SDL_Delay(SDL_GetTicks() - previousTick);
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
