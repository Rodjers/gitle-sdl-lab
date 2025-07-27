#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_main.h>

typedef struct {
    float x = 50;
    float y = 50;
    float v_speed = 0;
    float h_speed = 0;
    float mass = 10;
    float drag = 1.7;
} Player;

typedef struct {
    float gravity = 2;
    float air_density = 1;
} World;

typedef struct {
    int64_t currentFrame = 0;
    int64_t currentFrameTick = 0;
    bool debug_enabled = false;
    Player player;
    World world;
} GameState;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    GameState game_state;
} AppState;

static TTF_Font *font = NULL;
AppState app_state_not_pointer;
AppState* app_state = &app_state_not_pointer;

void update_player_speed(Player * player, World * world) {
    player->v_speed = player->v_speed + (world->gravity - (player->v_speed * player->h_speed * player->drag));
}

void update_player_position(Player * player) {
   player->y = player->y + player->v_speed;
};

void update_game_state(GameState * game_state) {

    Player* player = &game_state->player;
    World* world = &game_state->world;
    update_player_speed(player, world);
    update_player_position(player);
}

void handle_key_press(GameState *game_state, const SDL_KeyboardEvent key) {

    switch (key.scancode) {
        case SDL_SCANCODE_W:
            game_state->player.y = game_state->player.y - 10;
            break;
        case SDL_SCANCODE_S:
            game_state->player.y = game_state->player.y + 10;
            break;
        case SDL_SCANCODE_A:
            game_state->player.x = game_state->player.x - 10;
            break;
        case SDL_SCANCODE_D:
            game_state->player.x = game_state->player.x + 10;
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
    int frameHeight = 400;
    int frameWidth = 500;
    SDL_GetRenderOutputSize(renderer, &viewport.w, &viewport.h);
    SDL_FRect frame = { (float)viewport.w - frameWidth, 0, (float)frameWidth, (float)frameHeight };
    SDL_RenderRect(renderer, &frame);

    render_text_at(renderer, ("Current frame: " + std::to_string(game_state->currentFrame)).c_str(), frame.x + 5, frame.y + 5);
    render_text_at(renderer, ("Current frame tick: " + std::to_string(game_state->currentFrame)).c_str(), frame.x + 5, frame.y + 35);
    render_text_at(renderer, ("Player X: " + std::to_string(game_state->player.x)).c_str(), frame.x + 5, frame.y + 65);
    render_text_at(renderer, ("Player Y: " + std::to_string(game_state->player.y)).c_str(), frame.x + 5, frame.y + 95);
    render_text_at(renderer, ("World gravity: " + std::to_string(game_state->world.gravity)).c_str(), frame.x + 5, frame.y + 125);

}
/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Init app state

    std::cout << app_state->game_state.world.gravity << std::endl;
    if (!SDL_CreateWindowAndRenderer("examples/renderer/clear", 640, 480, SDL_WINDOW_BORDERLESS + SDL_WINDOW_FULLSCREEN,
                                     &app_state->window, &app_state->renderer)) {
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

    return SDL_APP_CONTINUE; /* carry on with the program! */
}


/* This function runs once per frame, and is the heart of the program. */
uint64_t completedFrame;



SDL_AppResult SDL_AppIterate(void *appstate) {
    uint64_t currentTick = SDL_GetTicks();
    uint64_t currentFrameTick = currentTick - (currentTick % 10);
    if (currentFrameTick > completedFrame) {
        completedFrame = currentFrameTick;
        // std::cout << currentFrameTick << std::endl;

        GameState* game_state = &app_state->game_state;

        game_state->currentFrame++;
        game_state->currentFrameTick = currentFrameTick;

        //game_state->world.gravity = 1;
        //std::cout << game_state->world.gravity << std::endl;
        //std::cout << game_state->player.x << std::endl;
        update_game_state(game_state);

        // Clear screen
        SDL_SetRenderDrawColor(app_state->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); /* black, full alpha */
        SDL_RenderClear(app_state->renderer); /* start with a blank canvas. */

        // Render Player
        SDL_FRect rect = {game_state->player.x, game_state->player.y, 40, 40};
        SDL_SetRenderDrawColor(app_state->renderer, 255, 0, 0, SDL_ALPHA_OPAQUE); /* black, full alpha */
        SDL_RenderRect(app_state->renderer, &rect);

        // Render Hello World
        SDL_FRect textRect = {100, 100, 0, 0};
        static SDL_Texture *texture = NULL;
        SDL_Surface *text;
        text = TTF_RenderText_Blended(font, "Hello World!", 0, {0, 255, 0, 0});
        if (text) {
            texture = SDL_CreateTextureFromSurface(app_state->renderer, text);
            SDL_DestroySurface(text);
        }
        SDL_GetTextureSize(texture, &textRect.w, &textRect.h);
        SDL_RenderTexture(app_state->renderer, texture, NULL, &textRect);

        //Render Debug
        if (game_state->debug_enabled) {
            render_debug(game_state, app_state->renderer);
        }

        // Draw
        SDL_RenderPresent(app_state->renderer);
    } else {
        SDL_Delay(SDL_GetTicks() - currentFrameTick);
    }

    return SDL_APP_CONTINUE; /* carry on with the program! */
}


/* This function runs when a new event (mouse input, keypresses, etc.) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {

    GameState *game_state = &app_state->game_state;

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
