#pragma once

#include <IPluginInterface.h>

#include <Glacier/SGameUpdateEvent.h>
#include <Glacier/ZResource.h>
#include <Glacier/ZResourceID.h>

class RandomItems : public IPluginInterface {
public:
    void OnEngineInitialized() override;
    RandomItems();
    ~RandomItems() override;
    void OnDrawMenu() override;
    void OnDrawUI(bool p_HasFocus) override;

private:
    void OnFrameUpdate(const SGameUpdateEvent& p_UpdateEvent);
    void LoadRepositoryProps();
    void GiveRandomItem();
    std::pair<const std::string, ZRepositoryID> GetRepositoryPropFromIndex(int s_Index);
    std::string ConvertDynamicObjectValueTString(const ZDynamicObject& p_DynamicObject);

private:
    double m_ElapsedTime = 0;
    double m_DelaySeconds = 2;
    bool m_Running = false;
    bool m_ShowMessage = false;
    bool m_SpawnInWorld = true;
    bool m_IncludeItemsWithoutTitle = false;
    float m_HitmanItemPosition[3] = { 0, 1, 0 };
    TResourcePtr<ZTemplateEntityFactory> m_RepositoryResource;
    std::multimap<std::string, ZRepositoryID> m_RepositoryProps;

private:
    // Full list of categories the user can toggle
    const std::vector<std::string> m_AllCategories {
        "assaultrifle", "sniperrifle", "melee",       "explosives",
        "tool",         "pistol",      "shotgun",     "suitcase",
        "smg",          "distraction", "poison",      "container"
    };

    // Parallel flags: true = category is enabled
    // Initialized to all-true via the ctor (see below)
    std::vector<bool> m_CategoryEnabled;
};

DEFINE_ZHM_PLUGIN(RandomItems)
