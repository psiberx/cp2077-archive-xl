#pragma once

namespace Red::AddressLib
{
constexpr uint32_t Main = 240386859;

constexpr uint32_t ExecuteProcess = 2203918127; // CProcessRunner::Run
constexpr uint32_t InvokeSCC = 2746819158;
constexpr uint32_t LogChannel = 3083410640;
constexpr uint32_t UnpackFloat = 88935777;
constexpr uint32_t GetScriptGameInstance = 1559305924;

constexpr uint32_t AISpotPersistentDataArray_Reserve = 3842971216;
constexpr uint32_t AIWorkspotManager_RegisterSpots = 2515346652;

constexpr uint32_t AnimatedComponent_InitializeAnimations = 2855474741;

constexpr uint32_t AppearanceChanger_ComputePlayerGarment = 3243419919;
constexpr uint32_t AppearanceChanger_GetBaseMeshOffset = 4219677283;
constexpr uint32_t AppearanceChanger_GetSuffixes = 63057648;
constexpr uint32_t AppearanceChanger_GetSuffixValue = 3294244838;
constexpr uint32_t AppearanceChanger_RegisterPart = 3169139695;
constexpr uint32_t AppearanceChanger_SelectAppearanceName = 2770550105;

constexpr uint32_t AppearanceChangeSystem_ChangeAppearance1 = 735526026;
constexpr uint32_t AppearanceChangeSystem_ChangeAppearance2 = 386815609;

constexpr uint32_t AppearanceDefinition_ExtractPartComponents = 39206067;

constexpr uint32_t AppearanceNameVisualTagsPreset_GetVisualTags = 1186798404;

constexpr uint32_t AppearanceResource_FindAppearanceDefinition = 549398675;
constexpr uint32_t AppearanceResource_PostLoad = 3141736993;

constexpr uint32_t AsyncFileAPI_CloseAndFreeSystemHandle = 3883147390;
constexpr uint32_t AsyncFileAPI_Instance = 2124810438;
constexpr uint32_t AsyncFileAPI_OpenSystemHandle = 582815077;

constexpr uint32_t AttachmentSlots_InitializeSlots = 3224838039;
constexpr uint32_t AttachmentSlots_IsSlotEmpty = 4231927464;
constexpr uint32_t AttachmentSlots_IsSlotSpawning = 1283201918;
constexpr uint32_t AttachmentSlots_UpdateWithExclusiveLock = 2050568765;

constexpr uint32_t BufferReader_MakeType0 = 1299073554;
constexpr uint32_t BufferReader_MakeType1 = 3578095989;

constexpr uint32_t Camera_ProjectPoint = 1517361120;

constexpr uint32_t CBaseEngine_InitEngine = 3273923080;
constexpr uint32_t CBaseEngine_InvokeSCC = 2746819158;
constexpr uint32_t CBaseEngine_LoadGatheredResources = 3729789488;
constexpr uint32_t CBaseEngine_LoadScripts = 3570081113; // CBaseEngine::LoadScripts
constexpr uint32_t CBaseEngine_MainLoopTick = 1875906465;
constexpr uint32_t CBaseEngine_Shutdown = 2857110166;

constexpr uint32_t CBaseFunction_InitializeLocalStack = 314117590;
constexpr uint32_t CBaseFunction_InternalExecute = 2744524784; // rtti::Function::InternalCallNative

constexpr uint32_t CBaseStack_ResolveContext = 1745688837;

constexpr uint32_t CGameFramework_InitializeGameInstance = 4007601841;

constexpr uint32_t CharacterCustomizationFeetController_CheckState = 3015323410;

constexpr uint32_t CharacterCustomizationGenitalsController_CheckState = 2347184335;
constexpr uint32_t CharacterCustomizationGenitalsController_OnAttach = 1782982010;

constexpr uint32_t CharacterCustomizationHairstyleController_CheckState = 2652844338;
constexpr uint32_t CharacterCustomizationHairstyleController_OnDetach = 2249860539;

constexpr uint32_t CharacterCustomizationHelper_GetHairColor = 3414699684;

constexpr uint32_t CharacterCustomizationState_FinalizePart = 716262863;
constexpr uint32_t CharacterCustomizationState_GetArmsAppearances1 = 110771572;
constexpr uint32_t CharacterCustomizationState_GetArmsAppearances2 = 4059182261;
constexpr uint32_t CharacterCustomizationState_GetBodyAppearances1 = 4014159024;
constexpr uint32_t CharacterCustomizationState_GetBodyAppearances2 = 56573295;
constexpr uint32_t CharacterCustomizationState_GetHeadAppearances1 = 4051123539;
constexpr uint32_t CharacterCustomizationState_GetHeadAppearances2 = 3766105236;

constexpr uint32_t CharacterCustomizationSystem_EnsureState = 787099588;
constexpr uint32_t CharacterCustomizationSystem_GetResource = 4275058446;
constexpr uint32_t CharacterCustomizationSystem_Initialize = 2343126786;
constexpr uint32_t CharacterCustomizationSystem_InitializeAppOption = 1092645213;
constexpr uint32_t CharacterCustomizationSystem_InitializeMorphOption = 3776734053;
constexpr uint32_t CharacterCustomizationSystem_InitializeOptionsFromState = 1168449474;
constexpr uint32_t CharacterCustomizationSystem_InitializeSwitcherOption = 2229107513;
constexpr uint32_t CharacterCustomizationSystem_Uninitialize = 402202441;

constexpr uint32_t CMesh_AddStubAppearance = 555290091;
constexpr uint32_t CMesh_FindAppearance = 3007126651;
constexpr uint32_t CMesh_GetAppearance = 773462733;
constexpr uint32_t CMesh_LoadMaterialsAsync = 479081020;
constexpr uint32_t CMesh_PostLoad = 2272530537;
constexpr uint32_t CMesh_ShouldPreloadAppearances = 503977943;

constexpr uint32_t CNamePool_GetStringView = 2878476560;
constexpr uint32_t CNamePool_RegisterName = 627643257;

constexpr uint32_t CommunitySystem_Update = 1559565515;

constexpr uint32_t EngineConfig_GetInstance = 3182170505;

constexpr uint32_t EngineConfigStore_GetVar = 3758823966;

constexpr uint32_t EngineTime_Frequency = 3151104823;

constexpr uint32_t Entity_Assemble = 2182550867;
constexpr uint32_t Entity_Attach = 4248638169;
constexpr uint32_t Entity_Detach = 2263681375;
constexpr uint32_t Entity_Dispose = 2515274237;
constexpr uint32_t Entity_Initialize = 3490519617;
constexpr uint32_t Entity_Reassemble = 1560690857;
constexpr uint32_t Entity_RequestComponents = 2296260874;
constexpr uint32_t Entity_ResolveRecordID = 525805301;
constexpr uint32_t Entity_Uninitialize = 3596356635;

constexpr uint32_t EntityBuilder_ExtractComponentsJob = 489494088;
constexpr uint32_t EntityBuilder_ScheduleExtractComponentsJob = 437791594;

constexpr uint32_t EntityID_ToStringDEBUG = 3457552837;

constexpr uint32_t EntitySpawner_SpawnEntity = 3068279668;
constexpr uint32_t EntitySpawner_SpawnFromTemplate = 2509382878;

constexpr uint32_t EntitySpawnerToken_dtor = 552341017;
constexpr uint32_t EntitySpawnerToken_GetWaitHandle = 2091718968;

constexpr uint32_t EntityTemplate_FindAppearance = 36838056;
constexpr uint32_t EntityTemplate_OnLoad = 2741376473;

constexpr uint32_t ExtensionLoader_AddConfig = 1350506587;
constexpr uint32_t ExtensionLoader_Configure = 660606521;

constexpr uint32_t ExtensionService_Bootstrap = 1523060616;
constexpr uint32_t ExtensionService_Shutdown = 1255673638;

constexpr uint32_t FactoryIndex_LoadFactoryAsync = 1931943018;
constexpr uint32_t FactoryIndex_ResolveResource = 3040549301;

constexpr uint32_t FileSystem_Close = 633872857;
constexpr uint32_t FileSystem_Instance = 3788966949;

constexpr uint32_t GameApplication_InitResourceDepot = 2923109755;

constexpr uint32_t GarmentAssembler_ExtractComponentsJob = 809178766;
constexpr uint32_t GarmentAssembler_FindState = 2594581880;
constexpr uint32_t GarmentAssembler_OnGameDetach = 709304039;
constexpr uint32_t GarmentAssembler_ProcessGarment = 29053464;
constexpr uint32_t GarmentAssembler_ProcessMorphedMesh = 1567972572;
constexpr uint32_t GarmentAssembler_ProcessSkinnedMesh = 1663588463;
constexpr uint32_t GarmentAssembler_RemoveItem = 1863723270;

constexpr uint32_t GarmentAssemblerState_AddCustomItem = 3128897273;
constexpr uint32_t GarmentAssemblerState_AddItem = 2294423397;
constexpr uint32_t GarmentAssemblerState_ChangeCustomItem = 956641309;
constexpr uint32_t GarmentAssemblerState_ChangeItem = 3740082313;

constexpr uint32_t IComponent_Toggle = 3370191745;

constexpr uint32_t IMaterial_GetRenderData = 3016235445;

constexpr uint32_t ImpostorComponent_OnAttach = 4002093843;

constexpr uint32_t InkDrawArea_GetBoundingRect = 446829303;

constexpr uint32_t InkLayer_AttachLibraryInstance = 2019635589;
constexpr uint32_t InkLayer_AttachWidgetTree = 2050958960;
constexpr uint32_t InkLayer_RegisterListener = 4792499;

constexpr uint32_t InkLogicController_OnArrangeChildrenComplete = 3589084925;
constexpr uint32_t InkLogicController_OnInitialize = 2671382832;
constexpr uint32_t InkLogicController_OnUninitialize = 1159862457;

constexpr uint32_t inkPointerHandler_Create = 630009171;
constexpr uint32_t inkPointerHandler_Override = 4279316794;
constexpr uint32_t inkPointerHandler_Reset = 3695579637;

constexpr uint32_t InkSpawner_ControllerInjectionEnabled = 2890273101;
constexpr uint32_t InkSpawner_FinishAsyncSpawn = 4097515464;

constexpr uint32_t InkSystem_Instance = 2635337807;
constexpr uint32_t InkSystem_ProcessCharacterEvent = 4284882427;
constexpr uint32_t InkSystem_ProcessInputEvents = 673454961;

constexpr uint32_t InkWidget_Draw = 1737035665;
constexpr uint32_t InkWidget_GetLayer = 1667176558;
constexpr uint32_t InkWidget_IsVisible = 3227716428;
constexpr uint32_t InkWidget_ScriptAddChild = 3558023194;
constexpr uint32_t InkWidget_ScriptReparent = 492509908;
constexpr uint32_t InkWidget_SetLayer = 2516523266;
constexpr uint32_t InkWidget_SpawnFromLibrary = 111687851;
constexpr uint32_t InkWidget_TriggerEvent = 3357288308;

constexpr uint32_t InkWidgetContext_AddWidget = 2526158623;
constexpr uint32_t InkWidgetContext_Clone = 3823045541;

constexpr uint32_t InkWidgetLibrary_AsyncSpawnFromExternal = 3860807819;
constexpr uint32_t InkWidgetLibrary_AsyncSpawnFromLocal = 2342073570;
constexpr uint32_t InkWidgetLibrary_FindExternalLibrary = 2756653218;
constexpr uint32_t InkWidgetLibrary_SpawnFromExternal = 506278179;
constexpr uint32_t InkWidgetLibrary_SpawnFromLocal = 1158555307;
constexpr uint32_t InkWidgetLibrary_SpawnRoot = 3160221133;

constexpr uint32_t InkWindow_ctor = 1410271027;
constexpr uint32_t InkWindow_dtor = 1413416760;
constexpr uint32_t InkWindow_IsPointerVisible = 2091454561;
constexpr uint32_t InkWindow_SetPointerVisibility = 3741389418;
constexpr uint32_t InkWindow_SetPointerWidget = 621352754;
constexpr uint32_t InkWindow_TogglePointerInput = 1797656531;

constexpr uint32_t InkWorldLayer_UpdateComponents = 2254060789;

constexpr uint32_t IPlacedComponent_SetTransform = 1828854026;

constexpr uint32_t IScriptable_CheckClass = 2487884020;
constexpr uint32_t IScriptable_CheckExactClass = 1797004238;

constexpr uint32_t ISerializable_Clone = 2601784524;

constexpr uint32_t ItemFactoryAppearanceChangeRequest_LoadAppearance = 3392610574;
constexpr uint32_t ItemFactoryAppearanceChangeRequest_LoadTemplate = 1291460507;

constexpr uint32_t ItemFactoryRequest_LoadAppearance = 3659799256;

constexpr uint32_t JobHandle_Wait = 1576079097;

constexpr uint32_t JournalManager_LoadJournal = 1964512489;
constexpr uint32_t JournalManager_TrackQuest = 3274382593;

constexpr uint32_t JournalRootFolderEntry_Initialize = 1945256833;

constexpr uint32_t JournalTree_ProcessJournalIndex = 2052144225;

constexpr uint32_t Localization_LoadLipsyncs = 1488657506;
constexpr uint32_t Localization_LoadOnScreens = 3550098299;
constexpr uint32_t Localization_LoadSubtitles = 772484645;
constexpr uint32_t Localization_LoadVoiceOvers = 4223669659;

constexpr uint32_t LootResourceData_Read = 647500563;

constexpr uint32_t MappinSystem_GetMappinData = 3603701972;
constexpr uint32_t MappinSystem_GetPoiData = 2918455690;
constexpr uint32_t MappinSystem_OnStreamingWorldLoaded = 140387944;

constexpr uint32_t MeshAppearance_LoadMaterialSetupAsync = 1419388740;
constexpr uint32_t MeshAppearance_ReleaseReference = 1679234269;

constexpr uint32_t MeshMaterialBuffer_LoadMaterialAsync = 1609519068;

constexpr uint32_t MorphTargetManager_ApplyMorphTarget = 633871992;

constexpr uint32_t MorphTargetMesh_PostLoad = 1523127443;

constexpr uint32_t NodeRef_Create = 3491172781;

constexpr uint32_t ObjectSerializer_Prepare = 1944983789;
constexpr uint32_t ObjectSerializer_ReadFromStrean = 125247621;
constexpr uint32_t ObjectSerializer_WriteToBuffer = 2646156567;

constexpr uint32_t PersistencySystem_SetPersistentStateData = 546121377;

constexpr uint32_t PersistentObject_InitializeState = 3442350580;

constexpr uint32_t PhysicsGeometry_Destruct = 4084208396;
constexpr uint32_t PhysicsGeometry_FixOrientation = 3480428029;
constexpr uint32_t PhysicsGeometry_GetBoundingBox = 1323310044;

constexpr uint32_t PhysicsProxy_GetActorPhysxPtr = 814094301;
constexpr uint32_t PhysicsProxy_GetActorWorldTransform = 391385890;
constexpr uint32_t PhysicsProxy_GetHitObject = 2394822845;
constexpr uint32_t PhysicsProxy_GetNumActors = 1627852665;
constexpr uint32_t PhysicsProxy_GetNumShapes = 472845746;
constexpr uint32_t PhysicsProxy_GetShapeGeometry = 578361242;
constexpr uint32_t PhysicsProxy_GetShapeLocalTransform = 2380600610;

constexpr uint32_t PhysicsScene_SyncRaycastMultiple = 430713883;

constexpr uint32_t PhysicsTraceResult_GetHitObject = 2394822845;

constexpr uint32_t PlayerSystem_OnPlayerSpawned = 2050111212; // cp::PlayerSystem::OnPlayerMainObjectSpawned

constexpr uint32_t PointOfInterestMappin_SetPhase = 803085170;

constexpr uint32_t QuestLoader_PhasePreloadCheck = 1567104722;
constexpr uint32_t QuestLoader_ProcessPhaseResource = 790570700;

constexpr uint32_t QuestPhaseInstance_ExecuteGraph = 779107884;
constexpr uint32_t QuestPhaseInstance_ExecuteNode = 3227858325;
constexpr uint32_t QuestPhaseInstance_Initialize = 3423879019;

constexpr uint32_t QuestRootInstance_Start = 797843833;

constexpr uint32_t QuestsSystem_CreateContext = 3144298192;
constexpr uint32_t QuestsSystem_OnGameRestored = 2048921710;

constexpr uint32_t RenderData_Release = 2996308699;

constexpr uint32_t RenderDataPtr_Release = 2996308699;

constexpr uint32_t RenderProxy_SetHighlightParams = 1093803822;
constexpr uint32_t RenderProxy_SetMaterialOverride = 429336746;
constexpr uint32_t RenderProxy_SetScanningState = 2838044016;
constexpr uint32_t RenderProxy_SetVisibility = 1790971865;
constexpr uint32_t RenderProxy_UpdateVisibility = 2124419179;

constexpr uint32_t ResourceBank_ForgetResource = 2288918343;

constexpr uint32_t ResourceDepot_CheckResource = 43194193;
constexpr uint32_t ResourceDepot_DestructArchive = 1988169744;
constexpr uint32_t ResourceDepot_DestructArchives = 3517657864;
constexpr uint32_t ResourceDepot_InitializeArchives = 2892763485;
constexpr uint32_t ResourceDepot_LoadArchives = 3573367139;
constexpr uint32_t ResourceDepot_RequestResource = 2450934495;

constexpr uint32_t ResourceLoader_OnUpdate = 3955625134;
constexpr uint32_t ResourceLoader_RequestResource = 2365013187; // res::ResourceLoader::IssueLoadingRequest

constexpr uint32_t ResourcePath_Create = 3998356057;

constexpr uint32_t ResourceReference_EnsureLoaded = 2017464375;

constexpr uint32_t ResourceSerializer_Deserialize = 2901686778;
constexpr uint32_t ResourceSerializer_ExecutePostLoad = 1350901460;
constexpr uint32_t ResourceSerializer_Load = 2577814646;
constexpr uint32_t ResourceSerializer_OnDependenciesReady = 3892979844;
constexpr uint32_t ResourceSerializer_OnResourceReady = 1147149338;
constexpr uint32_t ResourceSerializer_ScheduleFinalizeJobs = 4272634361;
constexpr uint32_t ResourceSerializer_SchedulePostLoadJobs = 3599579551;

constexpr uint32_t RuntimeEntityRegistry_RegisterEntity = 2840271332; // world::RuntimeEntityRegistry::RegisterEntity
constexpr uint32_t RuntimeEntityRegistry_UnregisterEntity = 3878623943;

constexpr uint32_t RuntimeScene_AttachEntity = 3800374702;
constexpr uint32_t RuntimeScene_DetachEntity = 1732384813;

constexpr uint32_t RuntimeSystemEntity_AttachEntity = 1576214475;
constexpr uint32_t RuntimeSystemEntity_DetachEntity = 2597397686;

constexpr uint32_t RuntimeSystemWeather_GetEnvironmentDefinition = 1471948702;
constexpr uint32_t RuntimeSystemWeather_GetWeatherState = 2679119876;
constexpr uint32_t RuntimeSystemWeather_SetWeatherByIndex = 2987664085;
constexpr uint32_t RuntimeSystemWeather_SetWeatherByName = 2334794340;

constexpr uint32_t RuntimeSystemWorldStreaming_LoadWorldJob = 2872059340;

constexpr uint32_t SerializableMap_Instance = 2558660817;
constexpr uint32_t SerializableMap_CollectSerializables = 2056396508;

constexpr uint32_t ScriptBinder_CreateBitfield = 487007291;
constexpr uint32_t ScriptBinder_CreateClass = 1040327137;
constexpr uint32_t ScriptBinder_CreateEnum = 4171309343;
constexpr uint32_t ScriptBinder_CreateFunction = 777921665;
constexpr uint32_t ScriptBinder_CreateProperties = 3630769678;
constexpr uint32_t ScriptBinder_ResolveNatives = 2952800760;
constexpr uint32_t ScriptBinder_ResolveTypes = 1666326197;
constexpr uint32_t ScriptBinder_TranslateBytecode = 3442875632; // CScriptDataBinder::LoadOpcodes

constexpr uint32_t ScriptBundle_Collect = 1354309996;
constexpr uint32_t ScriptBundle_ctor = 3991604425;
constexpr uint32_t ScriptBundle_dtor = 3993832650;
constexpr uint32_t ScriptBundle_Read = 3761639893;
constexpr uint32_t ScriptBundle_Validate = 3482458695;

constexpr uint32_t ScriptOpCodes_Handlers = 961751128;
constexpr uint32_t ScriptOpCodes_Handlers_Cast = 2439126103;
constexpr uint32_t ScriptOpCodes_Handlers_New = 2669615409;
constexpr uint32_t ScriptOpCodes_Register = 2070546748;

constexpr uint32_t ScriptValidator_CompareType = 4224523662;
constexpr uint32_t ScriptValidator_CompareTypeName = 1156189659;
constexpr uint32_t ScriptValidator_Validate = 898639042;
constexpr uint32_t ScriptValidator_ValidateProperty = 560604283;

constexpr uint32_t SoundBankManager_ReadSoundBanksJson = 3961524856;

constexpr uint32_t StatsDataSystem_CheckStatFlag = 2954893634;
constexpr uint32_t StatsDataSystem_GetStatFlags = 3123320294;
constexpr uint32_t StatsDataSystem_GetStatRange = 1444748215;
constexpr uint32_t StatsDataSystem_InitializeParams = 3652194890;
constexpr uint32_t StatsDataSystem_InitializeRecords = 1299190886;

constexpr uint32_t StreamingSector_dtor = 3420392384;
constexpr uint32_t StreamingSector_OnReady = 3972601611;
constexpr uint32_t StreamingSector_PostLoad = 3972601611;

constexpr uint32_t StreamingSystem_CollectNodeParallel = 288107567;

constexpr uint32_t StreamingWorld_Serialize = 410718963;

constexpr uint32_t TagSystem_AssignTag = 1239092911;
constexpr uint32_t TagSystem_UnassignTag = 1875775378;

constexpr uint32_t TPPRepresentationComponent_IsAffectedSlot = 678894266;
constexpr uint32_t TPPRepresentationComponent_OnAttach = 4129169021;
constexpr uint32_t TPPRepresentationComponent_OnItemEquipped = 4010810747;
constexpr uint32_t TPPRepresentationComponent_OnItemUnequipped = 1933319146;
constexpr uint32_t TPPRepresentationComponent_RegisterAffectedItem = 3037343626;

constexpr uint32_t Transform_ApplyToBox = 4207022512;

constexpr uint32_t TweakDB_CreateRecord = 838931066; // game::data::AddRecord
constexpr uint32_t TweakDB_Init = 4053739225;
constexpr uint32_t TweakDB_Load = 63182599; // game::data::TweakDB::LoadOptimized
constexpr uint32_t TweakDB_TryLoad = 464332086;
constexpr uint32_t TweakDBID_Derive = 326438016;

constexpr uint32_t VehicleController_ToggleAuxLights = 1883248814;
constexpr uint32_t VehicleGarageComponent_LoadSummonSetup = 1159928678;
constexpr uint32_t VehicleSystem_ToggleGarageVehicle = 3027836941;

constexpr uint32_t VisionModeSystem_SetBraindanceMode = 1070077985;

constexpr uint32_t VisualController_LoadDependencies = 2912168594;

constexpr uint32_t WorkspotSystem_GetWorkspotInstance = 2234523611;

constexpr uint32_t WorldNodeInstance_Attach = 3515291207;
constexpr uint32_t WorldNodeInstance_Detach = 3473806907;
constexpr uint32_t WorldNodeInstance_Initialize = 624308217;
constexpr uint32_t WorldNodeInstance_SetVisibility = 2290487415;
}
