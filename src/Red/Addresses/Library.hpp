#pragma once

namespace Red::AddressLib
{
constexpr uint32_t Main = 3545369307;

constexpr uint32_t AnimatedComponent_InitializeAnimations = 2855474741;

constexpr uint32_t AppearanceChanger_ComputePlayerGarment = 3243419919;
constexpr uint32_t AppearanceChanger_GetBaseMeshOffset = 4219677283;
constexpr uint32_t AppearanceChanger_GetSuffixValue = 1317875529;
constexpr uint32_t AppearanceChanger_RegisterPart = 3169139695;
constexpr uint32_t AppearanceChanger_SelectAppearanceName = 2770550105;

constexpr uint32_t AppearanceChangeSystem_ChangeAppearance1 = 735526026;
constexpr uint32_t AppearanceChangeSystem_ChangeAppearance2 = 386815609;

constexpr uint32_t AppearanceDefinition_ExtractPartComponents = 39206067;

constexpr uint32_t AppearanceNameVisualTagsPreset_GetVisualTags = 1186798404;

constexpr uint32_t AppearanceResource_OnLoad = 3141736993;
constexpr uint32_t AppearanceResource_FindAppearanceDefinition = 549398675;

constexpr uint32_t AttachmentSlots_InitializeSlots = 3224838039;
constexpr uint32_t AttachmentSlots_IsSlotEmpty = 4231927464;
constexpr uint32_t AttachmentSlots_IsSlotSpawning = 1283201918;

constexpr uint32_t CBaseEngine_LoadGatheredResources = 3729789488;

constexpr uint32_t CharacterCustomizationFeetController_CheckState = 3015323410;

constexpr uint32_t CharacterCustomizationGenitalsController_OnAttach = 1782982010;
constexpr uint32_t CharacterCustomizationGenitalsController_CheckState = 2347184335;

constexpr uint32_t CharacterCustomizationHairstyleController_OnDetach = 2249860539;
constexpr uint32_t CharacterCustomizationHairstyleController_CheckState = 2652844338;

constexpr uint32_t CharacterCustomizationHelper_GetHairColor = 3414699684;

constexpr uint32_t CharacterCustomizationState_FinalizePart = 716262863;
constexpr uint32_t CharacterCustomizationState_GetHeadAppearances1 = 4051123539;
constexpr uint32_t CharacterCustomizationState_GetHeadAppearances2 = 3766105236;
constexpr uint32_t CharacterCustomizationState_GetBodyAppearances1 = 4014159024;
constexpr uint32_t CharacterCustomizationState_GetBodyAppearances2 = 56573295;
constexpr uint32_t CharacterCustomizationState_GetArmsAppearances1 = 110771572;
constexpr uint32_t CharacterCustomizationState_GetArmsAppearances2 = 4059182261;

constexpr uint32_t CharacterCustomizationSystem_Initialize = 2341291776;
constexpr uint32_t CharacterCustomizationSystem_Uninitialize = 402202441;
constexpr uint32_t CharacterCustomizationSystem_GetResource = 4275058446;
constexpr uint32_t CharacterCustomizationSystem_EnsureState = 787099588;
constexpr uint32_t CharacterCustomizationSystem_InitializeAppOption = 1092645213;
constexpr uint32_t CharacterCustomizationSystem_InitializeMorphOption = 3776734053;
constexpr uint32_t CharacterCustomizationSystem_InitializeSwitcherOption = 2229107513;
constexpr uint32_t CharacterCustomizationSystem_InitializeOptionsFromState = 1168449474;

constexpr uint32_t CMesh_OnLoad = 2272530537;
constexpr uint32_t CMesh_GetAppearance = 773462733;
constexpr uint32_t CMesh_FindAppearance = 3007126651;
constexpr uint32_t CMesh_LoadMaterialsAsync = 701648326;
constexpr uint32_t CMesh_AddStubAppearance = 555290091;
constexpr uint32_t CMesh_ShouldPreloadAppearances = 503977943;

constexpr uint32_t MeshMaterialBuffer_LoadMaterialAsync = 1609519068;

constexpr uint32_t Entity_ReassembleAppearance = 1560690857;

constexpr uint32_t EntityBuilder_ExtractComponentsJob = 525669984;

constexpr uint32_t EntityTemplate_OnLoad = 2741376473;
constexpr uint32_t EntityTemplate_FindAppearance = 36838056;

constexpr uint32_t FactoryIndex_LoadFactoryAsync = 1886854234;
constexpr uint32_t FactoryIndex_ResolveResource = 3040549301;

constexpr uint32_t GameApplication_InitResourceDepot = 2923109755;

constexpr uint32_t GarmentAssembler_FindState = 2594581880;
constexpr uint32_t GarmentAssembler_RemoveItem = 1863723270;
constexpr uint32_t GarmentAssembler_ProcessGarment = 29053464;
constexpr uint32_t GarmentAssembler_ExtractComponentsJob = 1164187413;
constexpr uint32_t GarmentAssembler_ProcessSkinnedMesh = 1663588463;
constexpr uint32_t GarmentAssembler_ProcessMorphedMesh = 1567972572;
constexpr uint32_t GarmentAssembler_OnGameDetach = 709304039;

constexpr uint32_t GarmentAssemblerState_AddItem = 2294423397;
constexpr uint32_t GarmentAssemblerState_AddCustomItem = 3128897273;
constexpr uint32_t GarmentAssemblerState_ChangeItem = 3740082313;
constexpr uint32_t GarmentAssemblerState_ChangeCustomItem = 956641309;

constexpr uint32_t InkSpawner_FinishAsyncSpawn = 2488155854;

constexpr uint32_t InkWidgetLibrary_AsyncSpawnFromExternal = 1396063719;
constexpr uint32_t InkWidgetLibrary_AsyncSpawnFromLocal = 118698863;
constexpr uint32_t InkWidgetLibrary_SpawnFromExternal = 506278179;
constexpr uint32_t InkWidgetLibrary_SpawnFromLocal = 1158555307;

constexpr uint32_t InkWorldLayer_UpdateComponents = 310722176;

constexpr uint32_t ItemFactoryAppearanceChangeRequest_LoadTemplate = 1291460507;
constexpr uint32_t ItemFactoryAppearanceChangeRequest_LoadAppearance = 3392610574;

constexpr uint32_t ItemFactoryRequest_LoadAppearance = 3659799256;

constexpr uint32_t JobHandle_Wait = 1576079097;

constexpr uint32_t JournalManager_LoadJournal = 1964512489;
constexpr uint32_t JournalManager_TrackQuest = 3274382593;

constexpr uint32_t JournalRootFolderEntry_Initialize = 1945256833;

constexpr uint32_t JournalTree_ProcessJournalIndex = 873338560;

constexpr uint32_t Localization_LoadOnScreens = 3550098299;
constexpr uint32_t Localization_LoadSubtitles = 772484645;
constexpr uint32_t Localization_LoadVoiceOvers = 4259845555;
constexpr uint32_t Localization_LoadLipsyncs = 1488657506;

constexpr uint32_t MappinSystem_GetMappinData = 3299551353;
constexpr uint32_t MappinSystem_GetPoiData = 620961393;
constexpr uint32_t MappinSystem_OnStreamingWorldLoaded = 140387944;

constexpr uint32_t MeshAppearance_LoadMaterialSetupAsync = 1419388740;

constexpr uint32_t QuestLoader_ProcessPhaseResource = 790570700;

constexpr uint32_t QuestsSystem_OnGameRestored = 2048921710;

constexpr uint32_t QuestRootInstance_Start = 797843833;

constexpr uint32_t ResourceDepot_InitializeArchives = 2885423437;
constexpr uint32_t ResourceDepot_LoadArchives = 2517385486;
constexpr uint32_t ResourceDepot_RequestResource = 2450934495;
constexpr uint32_t ResourceDepot_CheckResource = 43194193;

constexpr uint32_t ResourceLoader_RequestResource = 2365013187; // res::ResourceLoader::IssueLoadingRequest
constexpr uint32_t ResourceLoader_OnUpdate = 1303056161;

constexpr uint32_t ResourcePath_Create = 3998356057;

constexpr uint32_t ResourceSerializer_Deserialize = 2901686778;

constexpr uint32_t StreamingSector_OnReady = 3972601611;

constexpr uint32_t StreamingWorld_OnLoad = 410718963;

constexpr uint32_t TPPRepresentationComponent_OnAttach = 4129169021;
constexpr uint32_t TPPRepresentationComponent_OnItemEquipped = 4010810747;
constexpr uint32_t TPPRepresentationComponent_OnItemUnequipped = 1933319146;
constexpr uint32_t TPPRepresentationComponent_RegisterAffectedItem = 3037343626;
constexpr uint32_t TPPRepresentationComponent_IsAffectedSlot = 684464829;

constexpr uint32_t TweakDB_Load = 3602585178; // game::data::TweakDB::LoadOptimized
}
