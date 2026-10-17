#pragma once
#include <cstdint>
#include <mutex>
#include <string>
#include "VFilter.h"



namespace cr
{
namespace video
{
/**
 * @brief Example of a custom video filter implementation.
 *
 * The class implements all methods of the VFilter interface. The values which
 * come from outside (parameters, mask) are validated before use. All methods
 * are protected by one mutex, so the methods can be called from any thread.
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
    ~CustomVFilter() override;

    /**
     * @brief Copy constructor is deleted: the class owns a mutex.
     */
    CustomVFilter(const CustomVFilter&) = delete;

    /**
     * @brief Move constructor is deleted: the class owns a mutex.
     */
    CustomVFilter(CustomVFilter&&) = delete;

    /**
     * @brief Copy assignment operator is deleted: the class owns a mutex.
     * @return Reference to this object.
     */
    CustomVFilter& operator=(const CustomVFilter&) = delete;

    /**
     * @brief Move assignment operator is deleted: the class owns a mutex.
     * @return Reference to this object.
     */
    CustomVFilter& operator=(CustomVFilter&&) = delete;

    /**
     * @brief Get the version of the CustomVFilter class.
     * @return A string representing the version: "Major.Minor.Patch".
     */
    static std::string getVersion();

    /**
     * @brief Initialize video filter. The method checks the parameters: mode
     * must be 0 or 1, level must be in range [0, 100] and custom parameters
     * must be finite. The processing time is read only and is set to 0.
     * @param params Parameters class.
     * @return TRUE if the video filter is initialized or FALSE if not (the
     * parameters of the filter are not changed).
     */
    bool initVFilter(VFilterParams& params) override;

    /**
     * @brief Set the value for a specific library parameter. The method
     * checks the value according to the parameter ID (see initVFilter(...)).
     * The processing time is read only.
     * @param id The identifier of the library parameter.
     * @param value The value to set for the parameter.
     * @return TRUE if the parameter was successfully set, FALSE otherwise.
     */
    bool setParam(VFilterParam id, float value) override;

    /**
     * @brief Get the value of a specific library parameter.
     * @param id The identifier of the library parameter.
     * @return The value of the specified parameter or -1 if the parameter
     * does not exist.
     */
    float getParam(VFilterParam id) override;

    /**
     * @brief Get the structure containing all library parameters.
     * @param params Reference to a VFilterParams structure.
     */
    void getParams(VFilterParams& params) override;

    /**
     * @brief Execute a CustomVFilter command. The command ON sets the mode 1,
     * the command OFF sets the mode 0.
     * @param id The identifier of the library command to be executed.
     * @return TRUE if the command was executed successfully, FALSE otherwise.
     */
    bool executeCommand(VFilterCommand id) override;

    /**
     * @brief Process frame. The example does not change the frame.
     * @param frame Source video frame.
     * @return TRUE if video frame was processed or FALSE if not.
     */
    bool processFrame(cr::video::Frame& frame) override;

    /**
     * @brief Set filter mask. Filter omits image segments, where
     * filter mask pixel values equal 0.
     * @param mask Filter binary mask. Must have data and one of the pixel
     * formats: GRAY, NV12, NV21, YV12 or YU12.
     * @return TRUE if video filter mask was set or FALSE if not (the mask is
     * not valid or the memory can not be allocated).
     */
    bool setMask(cr::video::Frame mask) override;

    /**
     * @brief Decode and execute command.
     * @param data Pointer to command data.
     * @param size Size of data.
     * @return TRUE if command decoded and executed or FALSE if not.
     */
    bool decodeAndExecuteCommand(uint8_t* data, int size) override;

private:

    /// Parameters structure (default params). Protected by m_paramsMutex.
    cr::video::VFilterParams m_params;
    /// Mask for filter. Protected by m_paramsMutex.
    cr::video::Frame m_mask;
    /// Mutex to protect m_params and m_mask.
    std::mutex m_paramsMutex;
};
}
}
