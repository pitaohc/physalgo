#include "raylib.h"
#include "rlgl.h"

#include <cstdint>
#include <memory>
#include <vector>

#include "core/BruteForce.h"
#include "core/MultiBoxPruning.h"
#include "core/SweepAndPrune.h"
#include "sim/World.h"

namespace {

Vector3 toVector3(const Vec3& v) {
    return {v.x, v.y, v.z};
}

Color bodyColor(bool overlapping) {
    return overlapping ? Color{235, 90, 90, 255} : Color{90, 170, 235, 255};
}

const char* spawnModeName(SpawnMode m) {
    switch (m) {
        case SpawnMode::Random:   return "Random";
        case SpawnMode::AxisX:    return "Axis-X";
        case SpawnMode::AxisY:    return "Axis-Y";
        case SpawnMode::AxisZ:    return "Axis-Z";
        case SpawnMode::Diagonal: return "Diagonal";
        case SpawnMode::Stacked:  return "Stacked";
    }
    return "?";
}

int spawnCountFor(SpawnMode m) {
    return m == SpawnMode::Random ? 1600 : 100;
}

}

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;
    const float axisHeight = 70.0f;

    InitWindow(screenWidth, screenHeight, "SAP Broad-Phase Lab (3D)");
    SetTargetFPS(60);
    rlSetClipPlanes(0.1, 10000.0);

    const Box3 bounds{{-500.0f, -300.0f, -300.0f},
                      {500.0f, 300.0f, 300.0f}};

    World world(bounds);

    auto brute = std::make_unique<BruteForce>();
    auto sap = std::make_unique<SweepAndPrune>();
    auto mbp = std::make_unique<MultiBoxPruning>();
    SweepAndPrune* sapView = sap.get();
    std::vector<BroadPhase*> activePhases;
    activePhases.push_back(sap.get());
    activePhases.push_back(brute.get());
    activePhases.push_back(mbp.get());

    int phaseIndex = 0;
    world.setBroadPhase(activePhases[phaseIndex]);

    SpawnMode spawnModes[] = {SpawnMode::Random, SpawnMode::AxisX, SpawnMode::AxisY,
                              SpawnMode::AxisZ, SpawnMode::Diagonal, SpawnMode::Stacked};
    int spawnIndex = 0;
    world.spawnScenario(spawnModes[spawnIndex], spawnCountFor(spawnModes[spawnIndex]));

    const Vector3 center = toVector3(bounds.center());

    Camera3D perspectiveCam = {0};
    perspectiveCam.position = {900.0f, 700.0f, 900.0f};
    perspectiveCam.target = center;
    perspectiveCam.up = {0.0f, 1.0f, 0.0f};
    perspectiveCam.fovy = 45.0f;
    perspectiveCam.projection = CAMERA_PERSPECTIVE;

    Camera3D orthoCam = {0};
    orthoCam.position = {center.x, center.y + 1000.0f, center.z};
    orthoCam.target = center;
    orthoCam.up = {0.0f, 0.0f, 1.0f};
    orthoCam.fovy = 760.0f;
    orthoCam.projection = CAMERA_ORTHOGRAPHIC;

    bool ortho = false;
    bool paused = false;
    bool showAxis = true;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) paused = !paused;
        if (IsKeyPressed(KEY_A)) {

            phaseIndex = (phaseIndex + 1) % activePhases.size();
            world.setBroadPhase(activePhases[phaseIndex]);
        }
        if (IsKeyPressed(KEY_C)) ortho = !ortho;
        if (IsKeyPressed(KEY_E)) showAxis = !showAxis;
        if (IsKeyPressed(KEY_F)) {
            spawnIndex = (spawnIndex + 1) % 6;
            world.spawnScenario(spawnModes[spawnIndex], spawnCountFor(spawnModes[spawnIndex]));
        }
        if (IsKeyPressed(KEY_R)) {
            world.spawnScenario(spawnModes[spawnIndex], spawnCountFor(spawnModes[spawnIndex]));
        }

        if (!paused) {
            world.step(GetFrameTime());
        } else {
            world.step(0.0f);
        }

        if (!ortho) {
            UpdateCamera(&perspectiveCam, CAMERA_ORBITAL);
        }
        Camera3D camera = ortho ? orthoCam : perspectiveCam;

        std::vector<bool> overlapping(world.bodies().size(), false);
        for (const Pair& p : world.pairs()) {
            overlapping[static_cast<std::size_t>(p.a)] = true;
            overlapping[static_cast<std::size_t>(p.b)] = true;
        }

        BeginDrawing();
        ClearBackground(Color{18, 18, 24, 255});

        BeginMode3D(camera);

        DrawGrid(20, 50.0f);

        DrawCubeWiresV(center, toVector3(bounds.max - bounds.min),
                       Color{60, 60, 80, 255});

        for (const Pair& p : world.pairs()) {
            DrawLine3D(toVector3(world.bodies()[static_cast<std::size_t>(p.a)].position),
                       toVector3(world.bodies()[static_cast<std::size_t>(p.b)].position),
                       Color{235, 90, 90, 120});
        }

        for (std::size_t i = 0; i < world.bodies().size(); ++i) {
            const Body& b = world.bodies()[i];
            const Vector3 pos = toVector3(b.position);
            const Vector3 size = toVector3(b.halfExtents * 2.0f);
            DrawCubeV(pos, size, bodyColor(overlapping[i]));
            DrawCubeWiresV(pos, size, Color{20, 20, 30, 255});
        }

        EndMode3D();

        if (showAxis) {
            const int y = screenHeight - static_cast<int>(axisHeight * 0.5f);
            const int x0 = 16;
            const int x1 = screenWidth - 16;
            const float wx0 = bounds.min.x;
            const float wx1 = bounds.max.x;

            DrawLine(x0, y, x1, y, Color{90, 90, 110, 255});

            for (const SweepAndPrune::Endpoint& e : sapView->xEndpoints()) {
                const Color c = e.isMin ? Color{255, 180, 60, 255}
                                        : Color{80, 220, 160, 255};
                const float t = (e.value - wx0) / (wx1 - wx0);
                const int x = x0 + static_cast<int>(t * (x1 - x0));
                DrawLine(x, y - 12, x, y + 12, c);
            }
            DrawText("x endpoints:  min (orange)   max (green)",
                     x0, y + 20, 16, Color{150, 150, 170, 255});
        }

        DrawText(TextFormat("algorithm : %s  (A to switch)", world.broadPhase()->name()),
                 16, 14, 18, RAYWHITE);
        DrawText(TextFormat("spawn     : %s  (F to cycle)", spawnModeName(spawnModes[spawnIndex])),
                 16, 36, 18, RAYWHITE);
        DrawText(TextFormat("bodies    : %d", static_cast<int>(world.bodies().size())),
                 16, 58, 18, RAYWHITE);
        DrawText(TextFormat("pairs     : %d", static_cast<int>(world.pairs().size())),
                 16, 80, 18, RAYWHITE);
        DrawText(TextFormat("broadphase: %.1f us", world.broadPhaseMicros()),
                 16, 102, 18, RAYWHITE);
        DrawText(TextFormat("%d FPS", GetFPS()), screenWidth - 90, 14, 18, RAYWHITE);
        DrawText("Space pause   A algorithm   C camera   E axis   F spawn   R respawn",
                 16, screenHeight - 26, 16, Color{140, 140, 160, 255});

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
