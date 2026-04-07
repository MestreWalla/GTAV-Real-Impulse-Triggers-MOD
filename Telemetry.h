#pragma once
#include "inc/natives.h"
#include "inc/types.h"

struct VehicleTelemetry {
    bool isInVehicle;
    float rpm;
    int gear;
    float throttle;
    float brake;
    float wheelSpeed;
    float vehicleSpeed;
    float lateralSpeed;
    float zVelocity;
    bool isAirborne;
    float collisionImpact;
    float engineHealth;
    int playerHealth;
    unsigned int weaponGroup;
    bool isShooting;
    bool isAiming;
    
    // Additional combat events
    bool isMeleeing;
    bool isReloading;
    bool isWeaponSwitching;
    bool isBleeding;
    bool hasExplosionNearby;
    float explosionDistance;
    
    // Player state events
    bool isStunned;
    bool isDrowning;
    bool isPoisoned;
    bool isClimbing;
    bool isJumping;
    bool isFalling;
    bool isSwimming;
    bool isParachuting;
    float fallHeight;
    
    // Vehicle state events
    bool hasBlowoutTires;
    bool isVehicleOnFire;
    bool isInHelicopter;
    bool isInPlane;
    bool isInTrain;
    
    // Environmental
    bool hasLightningNearby;
    bool inPoliceChase;
};

class Telemetry {
public:
    static VehicleTelemetry GetData();
};
