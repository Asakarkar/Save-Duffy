#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define WINDOW_W 800
#define WINDOW_H 480
#define FRAME_W 64
#define FRAME_H 64
#define MAX_WORD 64
#define MAX_LIVES 6

const char *words[]={"puppy", "ball", "fetch", "park", "leash", "biscuit"};
const char *hints[]={
    "Small young dog, often very playful.",
    "A round toy used for throwing and catching.",
    "A command and game where you throw something and the dog returns it.",
    "A public outdoor place with grass — good for walks.",
    "A strap used to control a dog on walks.",
    "A tasty treat often given as a reward."
};
const int WORD_COUNT=sizeof(words)/sizeof(words[0]);
typedef enum {STATE_START, STATE_PLAY, STATE_WIN, STATE_DEAD} GameState;
typedef struct {
    SDL_Texture *tex;
    int frames;
    float frame_time;
}Anim;

typedef struct {
    SDL_Window *win;
    SDL_Renderer *ren;
    Anim anims[4];
    SDL_Texture *heart_full;
    SDL_Texture *heart_empty;
    TTF_Font *font;
    GameState state;
    int anim_index;
    int anim_frame;
    float anim_acc;                    // accumulate dt for frame change
    float anim_switch_acc;             // accumulate dt to switch to next animation
    float anim_switch_time;            // how long before switching to next anim
    const char *secret;
    int secret_index;
    int revealed[MAX_WORD];
    int word_len;
    int lives;
    int guessed[26];
} Game;

SDL_Texture* load_texture(SDL_Renderer *ren, const char*path)
{
    SDL_Surface *surf = IMG_Load(path);
    if (!surf) {
        printf("IMG_Load failed (%s): %s\n", path, IMG_GetError()); return NULL;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);
    return tex;
}

int all_revealed(Game *g) {
    for (int i=0;i<g->word_len;i++) if (!g->revealed[i]) return 0;
    return 1;
}

void start_new_round(Game *g) {
    int idx=rand()% WORD_COUNT;
    g->secret_index=idx;
    g->secret=words[idx];
    g->word_len =(int) strlen(g->secret);

    for (int i=0;i<g->word_len;i++) g->revealed[i] =0;

    for (int i=0;i<26;i++) g->guessed[i]=0;

    g->lives =MAX_LIVES;
    g->anim_index= 0;
    g->anim_frame = 0;
    g->anim_acc =0;
    g->anim_switch_acc= 0;
    g->state =STATE_PLAY;
}

int init_anim(Game *g, int idx, const char *path, float frame_time) {
    SDL_Texture *t = load_texture(g->ren, path);
    if (!t){ 
        printf("Failed to load %s\n",path); 
        g->anims[idx].tex =NULL; 
        g->anims[idx].frames= 1; 
        return 0; 
    }
    int w,h;
    SDL_QueryTexture(t, NULL, NULL, &w, &h);
    int frames =(w+FRAME_W-1)/FRAME_W;
    g->anims[idx].tex =t;
    g->anims[idx].frames= frames;
    g->anims[idx].frame_time= frame_time;
    return 1;
}

int init_game(Game *g) {
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0){
        printf("SDL_Init: %s\n", SDL_GetError()); 
        return 0; 
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0){
        printf("IMG_Init failed\n");
        }
    if (TTF_Init() != 0){ 
        printf("TTF_Init failed\n");
    }

    g->win = SDL_CreateWindow("Save the Dog - Guess Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_W, WINDOW_H, 0);
   
    if (!g->win){ 
        printf("SDL_CreateWindow: %s\n", SDL_GetError()); 
        return 0; 
    }
    g->ren = SDL_CreateRenderer(g->win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g->ren){ 
        printf("SDL_CreateRenderer: %s\n", SDL_GetError()); 
        return 0; 
    }

    g->heart_full = load_texture(g->ren, "assets/ui/heart_full.png");
    g->heart_empty = load_texture(g->ren, "assets/ui/heart_empty.png");
    g->font = TTF_OpenFont("assets/font.ttf", 20);
    if (!g->font) printf("TTF_OpenFont failed (assets/font.ttf): %s\n", TTF_GetError());

    init_anim(g, 0, "assets/sprites/dog_idle.png",0.35f);
    init_anim(g, 1, "assets/sprites/dog_tail.png",0.25f);
    init_anim(g, 2, "assets/sprites/dog_run.png",0.25f);
    init_anim(g, 3, "assets/sprites/dog_death.png",0.30f);

    g->anim_switch_time=2.5f;
    g->anim_index=0;
    g->anim_frame =0;
    g->anim_acc=0;
    g->anim_switch_acc=0;

    g->state = STATE_START;
    srand((unsigned int) time(NULL));
    return 1;
}

void cleanup_game(Game *g){
    for (int i=0;i<4;i++) if (g->anims[i].tex) SDL_DestroyTexture(g->anims[i].tex);
    if (g->heart_full) SDL_DestroyTexture(g->heart_full);
    if (g->heart_empty) SDL_DestroyTexture(g->heart_empty);
    if (g->font) TTF_CloseFont(g->font);
    if (g->ren) SDL_DestroyRenderer(g->ren);
    if (g->win) SDL_DestroyWindow(g->win);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void render_text(Game *g, const char *text, int x, int y){
    if (!g->font) return;
    SDL_Color col ={255,255,255,255};
    SDL_Surface *s= TTF_RenderText_Blended(g->font, text, col);
    if (!s) return;
    SDL_Texture *t=SDL_CreateTextureFromSurface(g->ren, s);
    SDL_Rect dst ={x,y,s->w,s->h};
    SDL_FreeSurface(s);
    SDL_RenderCopy(g->ren, t, NULL, &dst);
    SDL_DestroyTexture(t);
}

void render_text_wrapped(Game *g, const char *text, int x, int y, int wrap_w){
    if (!g->font) return;
    SDL_Color col={220,220,220,255};
    SDL_Surface *s =TTF_RenderText_Blended_Wrapped(g->font, text, col, wrap_w);
    if (!s) return;
    SDL_Texture *t =SDL_CreateTextureFromSurface(g->ren, s);
    SDL_Rect dst ={x,y,s->w,s->h};
    SDL_FreeSurface(s);
    SDL_RenderCopy(g->ren,t,NULL,&dst);
    SDL_DestroyTexture(t);
}

void render_ui(Game *g){
    SDL_Rect panel ={ WINDOW_W - 320, 0, 320, WINDOW_H };
    SDL_SetRenderDrawColor(g->ren, 26, 26, 26, 255);
    SDL_RenderFillRect(g->ren, &panel);

    render_text(g, "Guess the Word!", panel.x+12, 20);

    for (int i=0;i<MAX_LIVES;i++) {
        SDL_Rect dst={panel.x+12 + i*38, 60, 32, 32 };
        if (i<g->lives) { if (g->heart_full) SDL_RenderCopy(g->ren, g->heart_full, NULL, &dst); }
        else { if (g->heart_empty) SDL_RenderCopy(g->ren, g->heart_empty, NULL, &dst); }
    }

    const char *hint =hints[g->secret_index];
    render_text(g, "Hint:", panel.x+12, 110);
    render_text_wrapped(g, hint, panel.x+12, 136, panel.w - 24);

    int start_y=240;
    int char_x =panel.x + 12;
    int step =28;
    for (int i=0;i<g->word_len;i++){
        char buf[2]="_";
        if (g->revealed[i]) { buf[0] = (char)toupper(g->secret[i]); buf[1]=0;} render_text(g, buf, char_x + i*step, start_y);
    }

    render_text(g, "Guessed:", panel.x+12, start_y + 48);
    int gx = panel.x+12; int gy=start_y+76;
    for (int i=0;i<26;i++){
        if (g->guessed[i]){
            char buf[2] ={(char)('A'+i),0};
            render_text(g, buf, gx, gy);
            gx += 18;
            if (gx>panel.x+panel.w-20) {gx=panel.x+12; gy += 20; }
        }
    }
}

void render_game(Game *g, float dt) {
    SDL_SetRenderDrawColor(g->ren, 0, 0, 0, 255);
    SDL_RenderClear(g->ren);

    int anim_to_draw = g->anim_index;
    if (g->state==STATE_DEAD) anim_to_draw = 3;
    Anim *a = &g->anims[anim_to_draw];
    if (a->tex){
        g->anim_acc += dt;
        if (g->anim_acc >= a->frame_time) {
            g->anim_frame = (g->anim_frame + 1)%(a->frames>0? a->frames:1);
            g->anim_acc -= a->frame_time;
        }
        if (g->state==STATE_PLAY) {
            g->anim_switch_acc+=dt;
            if (g->anim_switch_acc>=g->anim_switch_time){
                g->anim_index = (g->anim_index + 1) % 3;
                g->anim_switch_acc -= g->anim_switch_time;
                g->anim_frame=0;
            }
        }

        int SCALE=3;
        SDL_Rect dst={
            WINDOW_W/3-(FRAME_W * SCALE) / 2,
            WINDOW_H/2-(FRAME_H * SCALE) / 2,
            FRAME_W*SCALE,
            FRAME_H*SCALE
        };
        SDL_Rect src= { g->anim_frame *FRAME_W, 0, FRAME_W, FRAME_H };
        SDL_RenderCopy(g->ren, a->tex, &src,&dst);
    }

    render_ui(g);

    if (g->state == STATE_START) {
        render_text(g, "Press SPACE to Start", WINDOW_W/2 - 120, WINDOW_H - 80);
    } else if (g->state == STATE_WIN) {
        render_text(g, "You saved the dog! :) Press SPACE", WINDOW_W/2 - 160, WINDOW_H - 80);
    } else if (g->state == STATE_DEAD) {
        render_text(g, "The dog died... Press SPACE to restart", WINDOW_W/2 - 200, WINDOW_H - 80);
    }

    SDL_RenderPresent(g->ren);
}

int main(int argc, char **argv) {
    Game game;
    memset(&game,0,sizeof(Game));
    if (!init_game(&game)) return 1;

    start_new_round(&game);

    int running = 1;
    Uint32 last = SDL_GetTicks();

    SDL_StartTextInput();
    while(running){
        Uint32 now=SDL_GetTicks();
        float dt=(now - last) / 1000.0f;
        last=now;

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type==SDL_QUIT) running = 0;
            else if (e.type==SDL_KEYDOWN) {
                if (e.key.keysym.sym==SDLK_ESCAPE) running =0;
                else if (game.state == STATE_START && e.key.keysym.sym == SDLK_SPACE) {
                    start_new_round(&game);
                } else if ((game.state==STATE_WIN || game.state == STATE_DEAD) && e.key.keysym.sym == SDLK_SPACE) {
                    start_new_round(&game);
                } else if (game.state== STATE_PLAY) {
                    SDL_Keycode k =e.key.keysym.sym;
                    if (k>=SDLK_a && k <= SDLK_z) {
                        char ch =(char)(k-SDLK_a+ 'a');
                        int idx=ch-'a';
                        if (!game.guessed[idx]) {
                            game.guessed[idx]=1;
                            int found=0;
                            for (int i=0;i<game.word_len;i++) {
                                if (game.secret[i]==ch) {
                                    game.revealed[i]= 1; found = 1;
                                }
                            }
                            if (!found){
                                game.lives--;
                                if (game.lives<= 0) {
                                    game.state =STATE_DEAD;
                                    game.anim_frame=0;
                                    game.anim_acc=0;
                                }
                            } else {
                                if (all_revealed(&game)) {
                                    game.state=STATE_WIN;
                                }
                            }
                        }
                    }
                }
            }
        }

        render_game(&game, dt);
        SDL_Delay(10);
    }
    SDL_StopTextInput();

    cleanup_game(&game);
    return 0;
}
