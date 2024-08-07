#pragma once
#include <string>
#include <cstdint>
#include <mutex>
#include "VFilter.h"



namespace cr
{
namespace video
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
     * @brief Initialize video filter. The particular filter should initialize
     * only supported parameters from VFilterParams class.
     * @param params Parameters class.
     * @return TRUE if the video filter is initialized or FALSE if not.
     */
    bool initVFilter(VFilterParams& params) override;

    /**
     * @brief Set the value for a specific library parameter.
     * @param id The identifier of the library parameter.
     * @param value The value to set for the parameter.
     * @return TRUE if the parameter was successfully set, FALSE otherwise.
     */
    bool setParam(VFilterParam id, float value) override;

    /**
     * @brief Get the value of a specific library parameter.
     * @param id The identifier of the library parameter.
     * @return The value of the specified parameter.
     */
    float getParam(VFilterParam id) override;

    /**
     * @brief Get the structure containing all library parameters.
     * @param params Reference to a PanTiltParams structure.
     */
    void getParams(VFilterParams& params) override;

    /**
     * @brief Execute a CustomVFilter command.
     * @param id The identifier of the library command to be executed.
     * @return TRUE if the command was executed successfully, FALSE otherwise.
     */
    bool executeCommand(VFilterCommand id) override;

    /**
     * @brief Process frame.
     * @param frame Source video frame.
     * @return TRUE if video frame was processed or FALSE if not.
     */
    bool processFrame(cr::video::Frame& frame) override;
    
    /**
    * @brief Set filter mask. Filter omits image segments, where 
    * filter mask pixel values equal 0.
    * @param mask Filter binary mask.
    * @return TRUE if video filter mask was set or FALSE if not.
    */
    bool setMask(cr::video::Frame mask) override;

    /**
     * @brief Decode and execute command.
     * @param data Pointer to command data.
     * @param size Size of data.
     * @return 0 - command decoded, 1 - set param command decoded, -1 - error.
     */
    bool decodeAndExecuteCommand(uint8_t* data, int size) override;

private:

    /// Parameters structure (default params).
    cr::video::VFilterParams m_params;
    /// Mutex for parameters access.
    std::mutex m_paramsMutex;
    /// Mask for filter.
    cr::video::Frame m_mask;
};
}
}