#include "Telemetry.h"
#include "inc/main.h"
#include <algorithm>

static float s_lastSpeed = 0.0f;

VehicleTelemetry Telemetry::GetData() {
    VehicleTelemetry data = { false, 0.0f, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false, 0.0f, 1000.0f, 200, 0, false, false };
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    Player player = PLAYER::GET_PLAYER_INDEX();
    
    data.playerHealth = ENTITY::GET_ENTITY_HEALTH(playerPed);
    
    Hash currentWeapon;
    if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &currentWeapon, true)) {
        data.weaponGroup = WEAPON::GET_WEAPONTYPE_GROUP(currentWeapon);
        
        // Weapon Groups to ignore (Unarmed, Melee, etc.)
        // 0xA2719660 = Unarmed
        if (currentWeapon != 0xA2719660 && data.weaponGroup != 0) {
            data.isAiming = (PLAYER::IS_PLAYER_FREE_AIMING(player) || PED::IS_PED_AIMING_FROM_COVER(playerPed));
            data.isShooting = PED::IS_PED_SHOOTING(playerPed) != 0;
        }
    }

    if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, false)) {
        Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);
        data.isInVehicle = true;
        
        float speed = ENTITY::GET_ENTITY_SPEED(vehicle);
        data.vehicleSpeed = speed;
        
        Vector3 speedVec = ENTITY::GET_ENTITY_SPEED_VECTOR(vehicle, true);
        data.lateralSpeed = speedVec.x;
        data.zVelocity = speedVec.z;
        data.isAirborne = ENTITY::IS_ENTITY_IN_AIR(vehicle) != 0;
        data.engineHealth = VEHICLE::GET_VEHICLE_ENGINE_HEALTH(vehicle);

        float speedDiff = s_lastSpeed - speed;
        if (speedDiff > 5.0f && !data.isAirborne) {
            data.collisionImpact = speedDiff;
        } else {
            data.collisionImpact = 0.0f;
        }
        s_lastSpeed = speed;
        
        if (speed < 5.0f) data.gear = 1;
        else if (speed < 12.0f) data.gear = 2;
        else if (speed < 22.0f) data.gear = 3;
        else if (speed < 35.0f) data.gear = 4;
        else data.gear = 5;

        data.rpm = (speed / (data.gear * 12.0f));
        if (data.rpm < 0.2f) data.rpm = 0.2f;
        if (data.rpm > 1.0f) data.rpm = 1.0f;
        
        data.throttle = CONTROLS::GET_CONTROL_NORMAL(0, 71); 
        data.brake = CONTROLS::GET_CONTROL_NORMAL(0, 72);
        data.wheelSpeed = speed;
        
        if (data.brake > 0.7f && speed > 5.0f) {
             static int counter = 0;
             if (++counter % 2 == 0) data.wheelSpeed = 0.0f;
        }
    }
    
    return data;
}