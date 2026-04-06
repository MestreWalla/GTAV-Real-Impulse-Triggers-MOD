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
};

class Telemetry {
public:
    static VehicleTelemetry GetData();
};
