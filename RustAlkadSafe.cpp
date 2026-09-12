// RustAlkadSafe.cpp
// Безопасная DLL-версия: не открывает и не изменяет другие процессы,
// не выполняет инъекцию и не обходит защиту.

#include <Windows.h>
#include <cmath>

struct Vector3 {
    float x, y, z;
};

struct CameraData {
    Vector3 pos;
    Vector3 viewAngles;
    float fov;
};

extern "C" __declspec(dllexport)
BOOL WorldToScreen(Vector3 world, Vector3* screen, CameraData cam,
                   float screenWidth, float screenHeight)
{
    if (!screen || screenWidth <= 0.0f || screenHeight <= 0.0f)
        return FALSE;

    Vector3 delta{
        world.x - cam.pos.x,
        world.y - cam.pos.y,
        world.z - cam.pos.z
    };

    constexpr float DEG_TO_RAD = 0.017453292519943295f;
    float pitch = cam.viewAngles.x * DEG_TO_RAD;
    float yaw   = cam.viewAngles.y * DEG_TO_RAD;

    float cosP = std::cos(pitch), sinP = std::sin(pitch);
    float cosY = std::cos(yaw),   sinY = std::sin(yaw);

    Vector3 rotated;
    rotated.x = delta.x * cosY - delta.z * sinY;
    rotated.y = delta.x * sinP * sinY + delta.y * cosP
              + delta.z * sinP * cosY;
    rotated.z = -delta.x * cosP * sinY + delta.y * sinP
              + delta.z * cosP * cosY;

    if (rotated.z < 0.001f)
        return FALSE;

    float fovRadians = cam.fov * 0.5f * DEG_TO_RAD;
    float tanHalfFov = std::tan(fovRadians);
    if (std::abs(tanHalfFov) < 0.000001f)
        return FALSE;

    float focal = 1.0f / tanHalfFov;

    float halfW = screenWidth * 0.5f;
    float halfH = screenHeight * 0.5f;

    screen->x = halfW * (1.0f + (rotated.x * focal) / rotated.z);
    screen->y = halfH * (1.0f - (rotated.y * focal) / rotated.z);
    screen->z = rotated.z;

    return TRUE;
}

extern "C" __declspec(dllexport)
const char* GetDllName()
{
    return "RustAlkadSafe";
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
    (void)hModule;
    (void)reserved;

    if (reason == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(hModule);

    return TRUE;
}
