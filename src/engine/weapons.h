#ifndef WEAPONS_H
#define WEAPONS_H

#include "types.h"

typedef enum weapon_type {
    WEAPON_SHOTGUN = 0,
    WEAPON_PISTOL,
    WEAPON_REVOLVER,
    WEAPON_SMG,
    WEAPON_ROCKET_LAUNCHER
} Weapon_type;

typedef enum projectile_type {
    PROJECTILE_SMALL,
    PROJECTILE_LARGE,
    PROJECTILE_ROCKET,
} Projectile_type;

typedef struct weapon {
    float32 fire_rate, recoil;
    float32 projectile_speed;
    Projectile_type projectile;
} Weapon;

extern Weapon weapons[5];

#endif
