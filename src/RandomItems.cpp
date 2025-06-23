// RandomItems.cpp
// Plugin for Hitman Random Items mod. Spawns or adds random items at intervals.

#include "RandomItems.h"

#include <Logging.h>
#include <IconsMaterialDesign.h>
#include <Globals.h>

#include <Glacier/ZGameLoopManager.h>
#include <Glacier/ZItem.h>
#include <Glacier/ZScene.h>
#include <Glacier/THashMap.h>
#include <Glacier/ZInventory.h>
#include <Glacier/ZModule.h>
#include <Glacier/ZContentKitManager.h>

#include <Glacier/ZGameStats.h>



RandomItems::RandomItems()
  : m_CategoryEnabled(m_AllCategories.size(), true)
{

}

/**
 * Called when the game engine has finished initializing.
 * Registers the frame update delegate for continuous updates.
 */
void RandomItems::OnEngineInitialized() {
    Logger::Info("RandomItems has been initialized!");

    const ZMemberDelegate<RandomItems, void(const SGameUpdateEvent&)> s_Delegate(this, &RandomItems::OnFrameUpdate);
    Globals::GameLoopManager->RegisterFrameUpdate(s_Delegate, 1, EUpdateMode::eUpdatePlayMode);
}

/**
 * Destructor. Unregisters the frame update delegate to clean up resources.
 */
RandomItems::~RandomItems() {
    const ZMemberDelegate<RandomItems, void(const SGameUpdateEvent&)> s_Delegate(this, &RandomItems::OnFrameUpdate);
    Globals::GameLoopManager->UnregisterFrameUpdate(s_Delegate, 1, EUpdateMode::eUpdatePlayMode);
}

/**
 * Draws the toggle button in the main mod menu.
 */
void RandomItems::OnDrawMenu() {
    if (ImGui::Button(ICON_MD_LOCAL_FIRE_DEPARTMENT " Random Items")) {
        m_ShowMessage = !m_ShowMessage;
    }
}

/**
 * Renders the mod UI, including controls for start/stop, delay, spawn mode, and filters.
 * @param p_HasFocus Whether the UI window currently has input focus.
 */
void RandomItems::OnDrawUI(bool p_HasFocus) {
    if (m_ShowMessage && p_HasFocus) {
        if (ImGui::Begin(ICON_MD_LOCAL_FIRE_DEPARTMENT " Random Items", &m_ShowMessage)) {
            if (/*m_SpawnInWorld*/ false) {
                ImGui::SetWindowSize(ImVec2(611, 359));
            }
            else {
                ImGui::SetWindowSize(ImVec2(427, 300));
            }

            if (ImGui::Button(m_Running ? "Stop" : "Start")) {
                if (!m_Running) LoadRepositoryProps();
                m_Running = !m_Running;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Starting for the first time might freeze the game for a few seconds.");
            }
            ImGui::InputDouble("Delay (in s)", &m_DelaySeconds);
            ImGui::Checkbox("Spawn in world", &m_SpawnInWorld);
            /*if (m_SpawnInWorld) {
                ImGui::InputFloat3("Item position", m_HitmanItemPosition);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("The position where the item will be spawned relative to the player position.");
                }
            }*/
            ImGui::SeparatorText("Experimental");
            // ────────────── Category Filter Menu ──────────────
            if (ImGui::CollapsingHeader("Category Filters")) {
                ImGui::TextWrapped("Toggle which categories to include when rebuilding pool:");
                for (size_t i = 0; i < m_AllCategories.size(); ++i) {
                    // pull out a real bool from the vector<bool> proxy
                    bool enabled = m_CategoryEnabled[i];

                    // render the checkbox; ImGui will modify 'enabled' if clicked
                    if (ImGui::Checkbox(m_AllCategories[i].c_str(), &enabled)) {
                        // write it back into your vector<bool>
                        m_CategoryEnabled[i] = enabled;
                    }
                }
            }


            ImGui::Checkbox("Include items without title", &m_IncludeItemsWithoutTitle);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("This will include more items, increasing the time to build the item pool and including some buggy items that can't actually spawn.");
            }
            if (ImGui::Button("Rebuild Item Pool")) {
                LoadRepositoryProps();
            }
        }
        ImGui::End();
    }
}

/**
 * Called every frame when the game is updating. Accumulates time and spawns items at intervals.
 * @param p_UpdateEvent Contains timing information for this frame.
 */
void RandomItems::OnFrameUpdate(const SGameUpdateEvent& p_UpdateEvent) {
    if (!m_Running) return;

    m_ElapsedTime += p_UpdateEvent.m_GameTimeDelta.ToSeconds();
    if (m_ElapsedTime >= m_DelaySeconds)
    {
        GiveRandomItem();
        m_ElapsedTime = 0.0;
    }
}

/**
 * Retrieves the repository pair (title and ID) at the given index in the map.
 * @param s_Index Zero-based index into the repository properties map.
 * @return Pair of item title string and repository ID.
 * @throws std::out_of_range if index is invalid.
 */
std::pair<const std::string, ZRepositoryID> RandomItems::GetRepositoryPropFromIndex(int s_Index) {
    int s_CurrentIndex = 0;
    for (auto it = m_RepositoryProps.begin(); it != m_RepositoryProps.end(); ++it) {
        if (s_CurrentIndex == s_Index) {
            return *it;
        }
        ++s_CurrentIndex;
    }
    Logger::Error("repo index out of bounds");
    throw std::out_of_range("repo index out of bounds.");
}

/**
 * Loads and filters the repository of available items from the game resource.
 * Populates m_RepositoryProps based on category and title inclusion settings.
 */
void RandomItems::LoadRepositoryProps()
{
    Logger::Info("Loading repository (your game will freeze shortly)");

    // 1) Clear out any old entries
    m_RepositoryProps.clear();

    // 2) Build a list of only the categories the user has checked
    std::vector<std::string> s_IncludedCategories;
    for (size_t i = 0; i < m_AllCategories.size(); ++i) {
        if (m_CategoryEnabled[i]) {
            s_IncludedCategories.push_back(m_AllCategories[i]);
        }
    }

    // 3) Ensure the repository resource is loaded
    if (m_RepositoryResource.m_nResourceIndex == -1)
    {
        const auto s_ID = ResId<"[assembly:/repository/pro.repo].pc_repo">;
        Globals::ResourceManager->GetResourcePtr(m_RepositoryResource, s_ID, 0);
    }

    // 4) Only proceed if we have valid data
    if (m_RepositoryResource.GetResourceInfo().status == RESOURCE_STATUS_VALID)
    {
        // Raw map: ZRepositoryID → ZDynamicObject
        auto* s_RepositoryData = static_cast<
            THashMap<ZRepositoryID, ZDynamicObject, TDefaultHashMapPolicy<ZRepositoryID>>*
        >(m_RepositoryResource.GetResourceData());

        // 5) Iterate every entry in the repo
        for (auto it = s_RepositoryData->begin(); it != s_RepositoryData->end(); ++it)
        {
            const ZDynamicObject* s_DynamicObject = &it->second;
            const auto* s_Entries = s_DynamicObject->As<TArray<SDynamicObjectKeyValuePair>>();

            std::string s_Id;
            bool        s_HasTitle = false;
            bool        s_Included = true;
            std::string s_TitleToAdd;
            ZRepositoryID s_RepoIdToAdd("");

            // 6) Pull out each field we care about
            for (const auto& kv : *s_Entries)
            {
                std::string s_Key = kv.sKey.c_str();

                if (s_Key == "ID_")
                {
                    s_Id = ConvertDynamicObjectValueTString(kv.value);
                }
                else if (s_Key == "Title")
                {
                    s_HasTitle    = true;
                    std::string t = ConvertDynamicObjectValueTString(kv.value);
                    s_TitleToAdd  = t;
                    s_RepoIdToAdd = ZRepositoryID(s_Id.c_str());

                    // filter out blank titles if the user disabled them
                    if (t.empty() && !m_IncludeItemsWithoutTitle)
                        s_Included = false;
                }
                else if (s_Key == "InventoryCategoryIcon")
                {
                    // uppercase the category
                    std::string cat = ConvertDynamicObjectValueTString(kv.value);
                    std::transform(cat.begin(), cat.end(), cat.begin(), ::toupper);

                    // check it against our dynamic list
                    bool match = false;
                    for (auto& want : s_IncludedCategories)
                    {
                        std::string tmp = want;
                        std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
                        if (tmp == cat) { match = true; break; }
                    }
                    if (!match)
                        s_Included = false;
                }
                else if (s_Key == "IsHitmanSuit")
                {
                    // never spawn suits
                    s_Included = false;
                    break;
                }
            }

            // 7) If it passed all filters, add it to the pool
            if (s_Included && (s_HasTitle || m_IncludeItemsWithoutTitle))
            {
                m_RepositoryProps.insert({ s_TitleToAdd, s_RepoIdToAdd });
            }
        }
    }
}


/**
 * Converts a dynamic object value to its string representation.
 * Supports ZString, bool, and float64 types; others return the type name.
 * @param p_DynamicObject The dynamic object to convert.
 * @return Value as a UTF-8 encoded std::string.
 */
std::string RandomItems::ConvertDynamicObjectValueTString(const ZDynamicObject& p_DynamicObject)
{
    std::string s_Result;
    const IType* s_Type = p_DynamicObject.m_pTypeID->typeInfo();

    if (strcmp(s_Type->m_pTypeName, "ZString") == 0)
    {
        const auto s_Value = p_DynamicObject.As<ZString>();
        s_Result = s_Value->c_str();
    }
    else if (strcmp(s_Type->m_pTypeName, "bool") == 0)
    {
        if (*p_DynamicObject.As<bool>())
        {
            s_Result = "true";
        }
        else
        {
            s_Result = "false";
        }
    }
    else if (strcmp(s_Type->m_pTypeName, "float64") == 0)
    {
        double value = *p_DynamicObject.As<double>();

        s_Result = std::to_string(value).c_str();
    }
    else
    {
        s_Result = s_Type->m_pTypeName;
    }

    return s_Result;
}

/**
 * Chooses a random item from the repository pool and either spawns it in the world or adds it to inventory.
 */
void RandomItems::GiveRandomItem()
{
    if (m_RepositoryProps.size() == 0)
    {
        Logger::Info("loading repository props (your game might freeze shortly)");
        LoadRepositoryProps();
    }

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    size_t s_RandomIndex = std::rand() % m_RepositoryProps.size();
    auto s_PropPair = GetRepositoryPropFromIndex(s_RandomIndex);

    auto s_LocalHitman = SDK()->GetLocalPlayer();
    if (!s_LocalHitman) {
        Logger::Error("No local hitman.");
        return;
    }

    if(m_SpawnInWorld) {
        Logger::Info("Spawning in world: {}", s_PropPair.first);
        ZSpatialEntity* s_HitmanSpatial = s_LocalHitman.m_ref.QueryInterface<ZSpatialEntity>();

        const auto s_Scene = Globals::Hitman5Module->m_pEntitySceneContext->m_pScene;
        if (!s_Scene) {
            Logger::Warn("no scene loaded");
            return;
        }

        const auto s_ItemSpawnerID = ResId<"[modules:/zitemspawner.class].pc_entitytype">;
        const auto s_ItemRepoKeyID = ResId<"[modules:/zitemrepositorykeyentity.class].pc_entitytype">;

        TResourcePtr<ZTemplateEntityFactory> s_Resource, s_Resource2;

        Globals::ResourceManager->GetResourcePtr(s_Resource, s_ItemSpawnerID, 0);
        Globals::ResourceManager->GetResourcePtr(s_Resource2, s_ItemRepoKeyID, 0);

        if (!s_Resource)
        {
            Logger::Error("resource not loaded");
            return;
        }

        ZEntityRef s_ItemSpawnerEntity, s_ItemRepoKey;

        Functions::ZEntityManager_NewEntity->Call(Globals::EntityManager, s_ItemSpawnerEntity, "", s_Resource, s_Scene.m_ref, nullptr, -1);
        Functions::ZEntityManager_NewEntity->Call(Globals::EntityManager, s_ItemRepoKey, "", s_Resource2, s_Scene.m_ref, nullptr, -1);

        if (!s_ItemSpawnerEntity)
        {
            Logger::Error("failed to spawn item spawner");
            return;
        }

        if (!s_ItemRepoKey)
        {
            Logger::Error("failed to spawn item repo key entity");
            return;
        }

        const auto s_ItemSpawner = s_ItemSpawnerEntity.QueryInterface<ZItemSpawner>();

        s_ItemSpawner->m_ePhysicsMode = ZItemSpawner::EPhysicsMode::EPM_KINEMATIC;
        s_ItemSpawner->m_rMainItemKey.m_ref = s_ItemRepoKey;
        s_ItemSpawner->m_rMainItemKey.m_pInterfaceRef = s_ItemRepoKey.QueryInterface<ZItemRepositoryKeyEntity>();
        s_ItemSpawner->m_rMainItemKey.m_pInterfaceRef->m_RepositoryId = s_PropPair.second;
        s_ItemSpawner->m_bUsePlacementAttach = false;
        s_ItemSpawner->m_eDisposalTypeOverwrite = EDisposalType::DISPOSAL_HIDE;
        s_ItemSpawner->SetWorldMatrix(s_HitmanSpatial->GetWorldMatrix());

        Functions::ZItemSpawner_RequestContentLoad->Call(s_ItemSpawner);
    }  else {
        Logger::Info("Adding to inventory: {} {}", s_PropPair.first, s_PropPair.second.ToString());
        const TArray<TEntityRef<ZCharacterSubcontroller>>* s_Controllers = &s_LocalHitman.m_pInterfaceRef->m_pCharacter.m_pInterfaceRef->m_rSubcontrollerContainer.m_pInterfaceRef->m_aReferencedControllers;
        auto* s_Inventory = static_cast<ZCharacterSubcontrollerInventory*>(s_Controllers->operator[](6).m_pInterfaceRef);

        TArray<ZRepositoryID> s_ModifierIds;
        Functions::ZCharacterSubcontrollerInventory_AddDynamicItemToInventory->Call(s_Inventory, s_PropPair.second, "", &s_ModifierIds, 2);
    }
}

// Macro to register plugin with the Hitman mod framework
DECLARE_ZHM_PLUGIN(RandomItems);
