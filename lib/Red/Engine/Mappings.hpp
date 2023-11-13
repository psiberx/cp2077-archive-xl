#pragma once

#include "Framework.hpp"

namespace RED4ext
{
namespace game { struct RuntimeSystemLights; }
namespace nav { struct RuntimeSystemPathfinding; }
namespace work { struct WorkspotSystem; }
namespace world
{
struct RuntimeSystemRendering;
struct RuntimeSystemAudio;
struct RuntimeSystemNavigation;
struct RuntimeSystemMarkers;
struct RuntimeSystemPhysics;
struct AnimationSystem;
struct RuntimeSystemDebugRendering;
struct RuntimeSystemTriggers;
struct RuntimeSystemEntity;
struct RuntimeSystemScenes;
struct RuntimeSystemVisibility;
struct RuntimeEntityRegistry;
struct RuntimeSystemEnvironment;
struct geometryRuntimeSystemGeomDescription;
struct RuntimeSystemEffects;
struct RuntimeSystemSmartObjects;
struct RuntimeSystemEntityTransactor;
struct RuntimeSystemTraffic;
struct CorpseSystem;
struct RuntimeSystemWorldStreaming;
struct RuntimeSystemRemoteViews;
struct RuntimeSystemEntityAppearanceChanger;
struct NodeInstanceRegistry;
struct RuntimeSystemEntityTransforms;
struct uiRuntimeSystemUI;
struct RuntimeSystemMoverComponents;
struct RuntimeSystemTransformAnimator;
struct RuntimeSystemWeather;
struct RuntimeSystemBinkUpdate;
struct RuntimeSystemNodeStreaming;
struct RuntimeSystemFoliage;
struct RuntimeSystemDismemberment;
struct RuntimeSystemEffectAttachments;
struct RuntimeSystemRepellerComponents;
struct RuntimeSystemEntityVisualController;
struct RuntimeSystemDestruction;
struct RuntimeSystemStreamingQuery;
}

using worldRuntimeSystemRendering = world::RuntimeSystemRendering;
using worldRuntimeSystemAudio = world::RuntimeSystemAudio;
using worldRuntimeSystemNavigation = world::RuntimeSystemNavigation;
using worldRuntimeSystemMarkers = world::RuntimeSystemMarkers;
using worldRuntimeSystemPhysics = world::RuntimeSystemPhysics;
using worldAnimationSystem = world::AnimationSystem;
using worldRuntimeSystemDebugRendering = world::RuntimeSystemDebugRendering;
using worldRuntimeSystemTriggers = world::RuntimeSystemTriggers;
using worldRuntimeSystemEntity = world::RuntimeSystemEntity;
using worldRuntimeSystemScenes = world::RuntimeSystemScenes;
using worldRuntimeSystemVisibility = world::RuntimeSystemVisibility;
using worldRuntimeEntityRegistry = world::RuntimeEntityRegistry;
using worldRuntimeSystemEnvironment = world::RuntimeSystemEnvironment;
using worldgeometryRuntimeSystemGeomDescription = world::geometryRuntimeSystemGeomDescription;
using worldRuntimeSystemEffects = world::RuntimeSystemEffects;
using worldRuntimeSystemSmartObjects = world::RuntimeSystemSmartObjects;
using navRuntimeSystemPathfinding = nav::RuntimeSystemPathfinding;
using worldRuntimeSystemEntityTransactor = world::RuntimeSystemEntityTransactor;
using worldRuntimeSystemTraffic = world::RuntimeSystemTraffic;
using worldCorpseSystem = world::CorpseSystem;
using worldRuntimeSystemWorldStreaming = world::RuntimeSystemWorldStreaming;
using workWorkspotSystem = work::WorkspotSystem;
using worldRuntimeSystemRemoteViews = world::RuntimeSystemRemoteViews;
using worldRuntimeSystemEntityAppearanceChanger = world::RuntimeSystemEntityAppearanceChanger;
using worldNodeInstanceRegistry = world::NodeInstanceRegistry;
using worldRuntimeSystemEntityTransforms = world::RuntimeSystemEntityTransforms;
using worlduiRuntimeSystemUI = world::uiRuntimeSystemUI;
using worldRuntimeSystemMoverComponents = world::RuntimeSystemMoverComponents;
using worldRuntimeSystemTransformAnimator = world::RuntimeSystemTransformAnimator;
using worldRuntimeSystemWeather = world::RuntimeSystemWeather;
using worldRuntimeSystemBinkUpdate = world::RuntimeSystemBinkUpdate;
using worldRuntimeSystemNodeStreaming = world::RuntimeSystemNodeStreaming;
using worldRuntimeSystemFoliage = world::RuntimeSystemFoliage;
using worldRuntimeSystemDismemberment = world::RuntimeSystemDismemberment;
using worldRuntimeSystemEffectAttachments = world::RuntimeSystemEffectAttachments;
using worldRuntimeSystemRepellerComponents = world::RuntimeSystemRepellerComponents;
using worldRuntimeSystemEntityVisualController = world::RuntimeSystemEntityVisualController;
using worldRuntimeSystemDestruction = world::RuntimeSystemDestruction;
using gameRuntimeSystemLights = game::RuntimeSystemLights;
using worldRuntimeSystemStreamingQuery = world::RuntimeSystemStreamingQuery;
}

namespace Red
{
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemRendering, 0);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemAudio, 1);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemNavigation, 2);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemMarkers, 5);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemPhysics, 6);
RTTI_MAP_RUNTIME_SYSTEM(worldAnimationSystem, 7);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemDebugRendering, 8);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemTriggers, 9);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemEntity, 10);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemScenes, 11);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemVisibility, 12);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeEntityRegistry, 13);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemEnvironment, 14);
RTTI_MAP_RUNTIME_SYSTEM(worldgeometryRuntimeSystemGeomDescription, 16);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemEffects, 17);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemSmartObjects, 18);
RTTI_MAP_RUNTIME_SYSTEM(navRuntimeSystemPathfinding, 19);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemEntityTransactor, 20);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemTraffic, 23);
RTTI_MAP_RUNTIME_SYSTEM(worldCorpseSystem, 26);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemWorldStreaming, 27);
// RTTI_MAP_RUNTIME_SYSTEM(InternalFunctionalTestsRuntimeSystem, 29);
// RTTI_MAP_RUNTIME_SYSTEM(FunctionalTestsRuntimeSystem, 30);
RTTI_MAP_RUNTIME_SYSTEM(workWorkspotSystem, 31);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemRemoteViews, 33);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemEntityAppearanceChanger, 34);
RTTI_MAP_RUNTIME_SYSTEM(worldNodeInstanceRegistry, 35);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemEntityTransforms, 36);
RTTI_MAP_RUNTIME_SYSTEM(worlduiRuntimeSystemUI, 37);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemMoverComponents, 38);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemTransformAnimator, 39);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemWeather, 40);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemBinkUpdate, 41);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemNodeStreaming, 42);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemFoliage, 47);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemDismemberment, 48);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemEffectAttachments, 51);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemRepellerComponents, 52);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemEntityVisualController, 59);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemDestruction, 62);
RTTI_MAP_RUNTIME_SYSTEM(gameRuntimeSystemLights, 63);
RTTI_MAP_RUNTIME_SYSTEM(worldRuntimeSystemStreamingQuery, 64);
}
