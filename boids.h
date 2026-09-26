#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 730
#define BORDER_WIDTH 400 
#define BORDER_HEIGHT 200

#define FISH_WIDTH 18
#define FISH_HEIGHT 12
#define FISH_NUMBER 500
#define LEADER_NUMBER 55

#define TURNFACTOR 0.4 

#define NOISE_FACTOR 30

#define ADVOID_FACTOR 0.5 // 0.5 (0.01) si diminue poisson plus proche
#define MATCHING_FACTOR 0.5 // 0.5 si diminue on diminue l'ordre
#define CENTER_FACTOR 0.05 // 0.05 si diminue on augmente l'ordre

#define PROTECT_RADIUS 25 // (60) si diminue poisson plus proche
#define VISUAL_RADIUS 150 // (700) si diminue on diminue l'ordre (grosse influence)

#define MIN_SPEED 120 // 2.5
#define MAX_SPEED 150 // 5

#define DT 0.03

#define LEADER_FACTOR 0.5
#define TARGET_RADIUS 300 
#define TARGET_SPEED 0.025 // 0.01

#endif
