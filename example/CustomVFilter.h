#pragma once
#include <string>
#include <cstdint>
#include <mutex>
#include "VFilter.h"



namespace cr::pantilt
{
/**
 * @brief Custom pan-tilt device controller class.
 */
class CustomVFilter : public cr::video::VFilter
{
public:

    /**
     * @brief Class constructor.
     */
    CustomVFilter();

    /**
     * @brief Class destructor.
     */
    ~CustomVFilter();

    /**
     * @brief Get the version of the PanTilt class.
     * @return A string representing the version: "Major.Minor.Patch"
     */
    static std::string getVersion();

    /**
     * @brief Decode and execute command.
     * @param data Pointer to command data.
     * @param size Size of data.
     * @return 0 - command decoded, 1 - set param command decoded, -1 - error.
     */
    bool decodeAndExecuteCommand(uint8_t* data, int size);

private:

    /// Parameters structure (default params).
    cr::video::VFilterParams m_params;
    /// Mutex for parameters access.
    std::mutex m_paramsMutex;
};
}