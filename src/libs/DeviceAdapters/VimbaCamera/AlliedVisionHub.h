
#ifndef ALLIEDVISIONHUB_H
#define ALLIEDVISIONHUB_H

#include "AlliedVisionDeviceBase.h"
#include "DeviceBase.h"
#include "Loader/LibLoader.h"

#include <memory>
///////////////////////////////////////////////////////////////////////////////
// STATIC VARIABLES
///////////////////////////////////////////////////////////////////////////////
static constexpr const char *g_hubName = "AlliedVisionHub";

/**
 * @brief Class that represents a HUB of supported devices
 */
class AlliedVisionHub : public AlliedVisionDeviceBase<HubBase<AlliedVisionHub>, AlliedVisionHub>
{
    ///////////////////////////////////////////////////////////////////////////////
    // PUBLIC
    ///////////////////////////////////////////////////////////////////////////////
public:
    /**
     * @brief Contructor of a HUB
     */
    AlliedVisionHub();

    /**
     * @brief Destructor of a HUB
     */
    virtual ~AlliedVisionHub() = default;

    /**
     * @brief SDK getter
     * @return Pointer to SDK
     */
    std::shared_ptr<VimbaXApi> &getSDK();

    ///////////////////////////////////////////////////////////////////////////////
    // uMANAGER API METHODS
    ///////////////////////////////////////////////////////////////////////////////
    int DetectInstalledDevices() override;
    int Initialize() override;
    int Shutdown() override;
    void GetName(char *name) const override;
    bool Busy() override;

    ///////////////////////////////////////////////////////////////////////////////
    // PRIVATE
    ///////////////////////////////////////////////////////////////////////////////
private:
    std::shared_ptr<VimbaXApi> m_sdk; //<! Shared pointer to the SDK
};

#endif
