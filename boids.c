/* Importation des modules */
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "boids.h"


/* Création du type pour représenter un poisson */
struct rectangle_s{
  SDL_Rect rect;
  double Vx;
  double Vy;
  bool is_leader;
};

typedef struct rectangle_s rectangle_t;

/* Création du type pour représenter plusieurs poissons */
struct rectangle_list_s{
  rectangle_t* poissons;
  int number;
};

typedef struct rectangle_list_s rectangle_list_t;


/* Création du type pour représenter la cible invisible des leaders */
struct target_s{
  double x;
  double y;
  double angle;
};

typedef struct target_s target_t;


/* Constantes */
int game_is_running = false;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;


/* Fonction pour créer la fenêtre et le rendu */
int initialize_window(void){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }
    window = SDL_CreateWindow(
        "Modélisation",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );
    if (!window){
        fprintf(stderr, "Error creating SDL Window.\n");
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer){
        fprintf(stderr, "Error creating SDL Renderer.\n");
        return false;
    }
    return true;
}


/* Fonction pour détruire la fenêtre */
void destroy_window(void) {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}


/* Fonction pour récupérer les inputs claviers et fermer la modélisation */
void process_input(){
  SDL_Event event;
  while (SDL_PollEvent(&event)){
      switch (event.type){
          case SDL_QUIT:
              game_is_running = false;
              break;
          case SDL_KEYDOWN:
              if (event.key.keysym.sym == SDLK_ESCAPE){
                  game_is_running = false;
              }
              break;
      }
  }
}


/* Fonction pour importer une image */
SDL_Texture* load_image(const char path[], SDL_Renderer *renderer){
    SDL_Surface *tmp = NULL;
    SDL_Texture *texture = NULL;
    tmp = SDL_LoadBMP(path);
    texture = SDL_CreateTextureFromSurface(renderer, tmp);
    SDL_FreeSurface(tmp);
    return texture;
}


/* Fonction pour initialiser une liste de X poissons : type rectangle_list_t */
rectangle_list_t init_list_rectangle_t(int number){
  rectangle_t* groupe = malloc(sizeof(struct rectangle_s)*number);
  srand(time(NULL));
  for (int i = 0; i < number; i ++){
    /* Création du SDL_Rect */
    SDL_Rect rect = {(rand() % (WINDOW_WIDTH - FISH_WIDTH - BORDER_WIDTH)) + BORDER_WIDTH/2, (rand() % (WINDOW_HEIGHT - FISH_HEIGHT - BORDER_HEIGHT)) + BORDER_HEIGHT/2, FISH_WIDTH, FISH_HEIGHT};

    /* Initialisation de Vx et Vy */
    double theta = fmod((double)rand(),  2*M_PI);
    double Vx = cos(theta);
    double Vy = sin(theta);

    /* Création du poisson d'indice i et ajout dans la liste */
    if (i < LEADER_NUMBER){
      rectangle_t rectangle = {rect, Vx, Vy, true};
      groupe[i] = rectangle;
    }
    else{
      rectangle_t rectangle = {rect, Vx, Vy, false};
      groupe[i] = rectangle;
    }
  }
  rectangle_list_t result = {groupe, number};
  return result;
}


/* Fonction pour mettre a jour le rendu */
void render(rectangle_list_t groupe, SDL_Texture* background, SDL_Texture* fish, target_t target){
    SDL_RenderCopy(renderer, background, NULL, NULL);

    for (int i = 0; i < groupe.number; i ++){
      double angle = (180*atan2(groupe.poissons[i].Vy, groupe.poissons[i].Vx))/M_PI;
      if (angle < -90){
        SDL_RenderCopyEx(renderer, fish, NULL, &groupe.poissons[i].rect, angle, NULL, SDL_FLIP_VERTICAL);
        }
      else if (angle > 90){
        SDL_RenderCopyEx(renderer, fish, NULL, &groupe.poissons[i].rect, angle, NULL, SDL_FLIP_VERTICAL);
        }
      else{
        SDL_RenderCopyEx(renderer, fish, NULL, &groupe.poissons[i].rect, angle, NULL, SDL_FLIP_NONE);
        }
      }
    SDL_RenderPresent(renderer);
}


/* Calcul de l'ordre de Vicsek */
double calcul_order_1(rectangle_list_t groupe){
  double sum_x = 0.0;
  double sum_y = 0.0;

  for (int i = 0; i < groupe.number; i ++){
    double Vx = groupe.poissons[i].Vx;
    double Vy = groupe.poissons[i].Vy;
    double norme = sqrt(Vx*Vx + Vy*Vy);

    sum_x += Vx / norme;
    sum_y += Vy / norme;
  }

  sum_x /= groupe.number;
  sum_y /= groupe.number;

  return sqrt(sum_x*sum_x + sum_y*sum_y);
}

/* Calculer la vitesse du poisson i à l'instant t + 1 */
double* get_new_vitesse(rectangle_list_t groupe, int i, target_t target){
  double* result = malloc(sizeof(double)*2);
  result[0] = groupe.poissons[i].Vx;
  result[1] = groupe.poissons[i].Vy;

  double close_dx = 0;
  double close_dy = 0;

  double Vx_avg = 0;
  double Vy_avg = 0;

  double xpos_avg = 0;
  double ypos_avg = 0;
  int neighbors = 0;

  /* On récupère la position de ses voisins */
  for (int j = 0; j < groupe.number; j ++){
    if (i == j) continue;

    double diff_x = groupe.poissons[j].rect.x - groupe.poissons[i].rect.x;
    double diff_y = groupe.poissons[j].rect.y - groupe.poissons[i].rect.y;

    if (diff_x > WINDOW_WIDTH/2)  diff_x -= WINDOW_WIDTH;
    if (diff_x < -WINDOW_WIDTH/2) diff_x += WINDOW_WIDTH;
    if (diff_y > WINDOW_HEIGHT/2)  diff_y -= WINDOW_HEIGHT;
    if (diff_y < -WINDOW_HEIGHT/2) diff_y += WINDOW_HEIGHT;

    double square_distance = diff_x*diff_x + diff_y*diff_y;
    if (square_distance < PROTECT_RADIUS*PROTECT_RADIUS){
      double distance = sqrt(square_distance);
      if (distance > 0) {
        close_dx -= (diff_x / distance) * (PROTECT_RADIUS - distance);
        close_dy -= (diff_y / distance) * (PROTECT_RADIUS - distance);
      }
    }
    if (square_distance < VISUAL_RADIUS*VISUAL_RADIUS){
      Vx_avg += groupe.poissons[j].Vx;
      Vy_avg += groupe.poissons[j].Vy;

      xpos_avg += (groupe.poissons[i].rect.x + diff_x);
      ypos_avg += (groupe.poissons[i].rect.y + diff_y);

      neighbors += 1;
    }
  }

  if (neighbors > 0){
    Vx_avg /= neighbors;
    Vy_avg /= neighbors;
    xpos_avg /= neighbors;
    ypos_avg /= neighbors;

    result[0] += (Vx_avg - groupe.poissons[i].Vx)*MATCHING_FACTOR;
    result[1] += (Vy_avg - groupe.poissons[i].Vy)*MATCHING_FACTOR;

    result[0] += (xpos_avg - groupe.poissons[i].rect.x)*CENTER_FACTOR;
    result[1] += (ypos_avg - groupe.poissons[i].rect.y)*CENTER_FACTOR;
  }

  /* Influence du cercle de répulsion */
  result[0] += close_dx*ADVOID_FACTOR;
  result[1] += close_dy*ADVOID_FACTOR;

  /* Gestion du bruit */
  double noise_x = (((double)rand() / (double)RAND_MAX) * 2.0 - 1.0) * NOISE_FACTOR;
  double noise_y = (((double)rand() / (double)RAND_MAX) * 2.0 - 1.0) * NOISE_FACTOR;

  result[0] += noise_x;
  result[1] += noise_y;

  /* Gestion des leaders */
  if (groupe.poissons[i].is_leader){
    double dx = target.x - groupe.poissons[i].rect.x;
    double dy = target.y - groupe.poissons[i].rect.y;
    double distance = sqrt(dx*dx + dy*dy);
    double steer_x = (dx / distance) * MAX_SPEED - groupe.poissons[i].Vx;
    double steer_y = (dy / distance) * MAX_SPEED - groupe.poissons[i].Vy;
    if (distance > (PROTECT_RADIUS*PROTECT_RADIUS)/2){
      result[0] += steer_x * LEADER_FACTOR;
      result[1] += steer_y * LEADER_FACTOR;
    }
  }
  return result;
}


/* Mise a jour des vitesses et des positions de tous les poissons */
void update(rectangle_list_t groupe, target_t* target){
  double* Wx = malloc(sizeof(double)*groupe.number);
  double* Wy = malloc(sizeof(double)*groupe.number);
  for (int i = 0; i < groupe.number; i ++){
    double* result = get_new_vitesse(groupe, i, *target);
    Wx[i] = result[0];
    Wy[i] = result[1];
    free(result);
  }

  /* Mise a jour de la nouvelle position de chaque poisson */
  for (int i = 0; i < groupe.number; i ++){
    groupe.poissons[i].Vx = Wx[i];
    groupe.poissons[i].Vy = Wy[i];
    double speed = sqrt(Wx[i]*Wx[i] + Wy[i]*Wy[i]);

    /* Gestion de la vitesse min et max */
    if (speed > MAX_SPEED){
      groupe.poissons[i].Vx = (groupe.poissons[i].Vx/speed)*MAX_SPEED;
      groupe.poissons[i].Vy = (groupe.poissons[i].Vy/speed)*MAX_SPEED;
    }

    if (speed < MIN_SPEED){
      groupe.poissons[i].Vx = (groupe.poissons[i].Vx/speed)*MIN_SPEED;
      groupe.poissons[i].Vy = (groupe.poissons[i].Vy/speed)*MIN_SPEED;
    }

    groupe.poissons[i].rect.x += groupe.poissons[i].Vx * DT;
    groupe.poissons[i].rect.y += groupe.poissons[i].Vy * DT;

    /* Gestion des murs */
    if (groupe.poissons[i].rect.x < 0) {
      groupe.poissons[i].rect.x += WINDOW_WIDTH;
    }
    else if (groupe.poissons[i].rect.x >= WINDOW_WIDTH) {
      groupe.poissons[i].rect.x -= WINDOW_WIDTH;
    }

    if (groupe.poissons[i].rect.y < 0) {
      groupe.poissons[i].rect.y += WINDOW_HEIGHT;
    }
    else if (groupe.poissons[i].rect.y >= WINDOW_HEIGHT) {
      groupe.poissons[i].rect.y -= WINDOW_HEIGHT;
    }
  }

  /* Mise a jour de la cible */
  target->angle += TARGET_SPEED;
  target->x = WINDOW_WIDTH/2 + cosf(target->angle) * TARGET_RADIUS;
  target->y = WINDOW_HEIGHT/2 + sinf(target->angle) * TARGET_RADIUS;

  free(Wx);
  free(Wy);
}


/* Fonction principale */
int main(int argc, char *argv[]){
  game_is_running = initialize_window();
  SDL_Texture *background = load_image("background.bmp", renderer);
  SDL_Texture *fish = load_image("poisson.bmp", renderer);

  //Initialisation des poissons
  rectangle_list_t groupe = init_list_rectangle_t(FISH_NUMBER);
  target_t target = {300, 100, 0};
  render(groupe, background, fish, target);

  while (game_is_running){
    update(groupe, &target);
    SDL_Delay(30);
    render(groupe, background, fish, target);
    process_input();
  }
  destroy_window();
  free(groupe.poissons);

  return 0;
}
