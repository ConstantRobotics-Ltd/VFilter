#include "CustomVFilter.h"
#include "CustomVFilterVersion.h"
#include "Frame.h"
#include "VFilter.h"
#include <cmath>
#include <cstdint>
#include <mutex>
#include <new>
#include <string>



namespace
{
/// Limit of float values which can be converted to int. 2^31 is exactly
/// representable as float, so (-LIMIT <= value < LIMIT) is the exact range
/// of int.
constexpr float INT_RANGE_LIMIT{2147483648.0f};
/// Value of the filter mode: filter is disabled.
constexpr int MODE_OFF{0};
/// Value of the filter mode: filter is enabled.
constexpr int MODE_ON{1};
/// Minimum value of the filter level, percent.
constexpr float LEVEL_MIN{0.0f};
/// Maximum value of the filter level, percent.
constexpr float LEVEL_MAX{100.0f};
/// Value returned by getParam(...) if the parameter does not exist.
constexpr float NO_PARAM_VALUE{-1.0f};



/// Convert float param value to int. A conversion of NaN or of a value outside
/// of the int range is undefined behavior, so such values are rejected.
bool toInt(float value, int& result)
{
    if (!(value >= -INT_RANGE_LIMIT && value < INT_RANGE_LIMIT))
    {
        return false;
    }
    result = static_cast<int>(value);
    return true;
}



/// Check that the filter mode is valid.
bool isModeValid(int mode)
{
    return mode == MODE_OFF || mode == MODE_ON;
}



/// Check that the filter level is valid. The comparison is false for NaN, so
/// NaN is rejected.
bool isLevelValid(float level)
{
    return level >= LEVEL_MIN && level <= LEVEL_MAX;
}



/// Check that the filter params are valid. The type of the filter and the
/// processing time can have any value, a custom parameter must be finite.
bool areParamsValid(const cr::video::VFilterParams& params)
{
    return isModeValid(params.mode) && isLevelValid(params.level) &&
           std::isfinite(params.custom1) && std::isfinite(params.custom2) &&
           std::isfinite(params.custom3);
}



/// Check that the frame can be used as the filter mask: the frame has data and
/// has a pixel format supported for the mask.
bool isMaskValid(const cr::video::Frame& mask)
{
    if (mask.data == nullptr || mask.width <= 0 || mask.height <= 0 ||
        mask.size <= 0)
    {
        return false;
    }
    switch (mask.fourcc)
    {
    case cr::video::Fourcc::GRAY:
    case cr::video::Fourcc::NV12:
    case cr::video::Fourcc::NV21:
    case cr::video::Fourcc::YV12:
    case cr::video::Fourcc::YU12:
    {
        return true;
    }
    default:
    {
        return false;
    }
    }
}
}



cr::video::CustomVFilter::CustomVFilter() = default;



cr::video::CustomVFilter::~CustomVFilter() = default;



std::string cr::video::CustomVFilter::getVersion()
{
    return CUSTOM_VFILTER_VERSION;
}



bool cr::video::CustomVFilter::initVFilter(VFilterParams& params)
{
    // Check params: the filter params are not changed if a value is invalid.
    if (!areParamsValid(params))
    {
        return false;
    }

    // Lock mutex for setting parameters. The processing time is read only.
    const std::lock_guard<std::mutex> lock(m_paramsMutex);
    m_params = params;
    m_params.processingTimeMcSec = 0;

    return true;
}



bool cr::video::CustomVFilter::setParam(VFilterParam id, float value)
{
    // Lock mutex for setting parameters.
    const std::lock_guard<std::mutex> lock(m_paramsMutex);

    // Check param ID and value.
    switch (id)
    {
    case VFilterParam::MODE:
    {
        int mode = 0;
        if (!toInt(value, mode) || !isModeValid(mode))
        {
            return false;
        }
        m_params.mode = mode;
        return true;
    }
    case VFilterParam::LEVEL:
    {
        if (!isLevelValid(value))
        {
            return false;
        }
        m_params.level = value;
        return true;
    }
    case VFilterParam::PROCESSING_TIME_MCSEC:
    {
        // Read only parameter.
        return false;
    }
    case VFilterParam::TYPE:
    {
        int type = 0;
        if (!toInt(value, type))
        {
            return false;
        }
        m_params.type = type;
        return true;
    }
    case VFilterParam::CUSTOM_1:
    {
        // Custom parameter.
        if (!std::isfinite(value))
        {
            return false;
        }
        m_params.custom1 = value;
        return true;
    }
    case VFilterParam::CUSTOM_2:
    {
        // Custom parameter.
        if (!std::isfinite(value))
        {
            return false;
        }
        m_params.custom2 = value;
        return true;
    }
    case VFilterParam::CUSTOM_3:
    {
        // Custom parameter.
        if (!std::isfinite(value))
        {
            return false;
        }
        m_params.custom3 = value;
        return true;
    }
    }

    return false;
}



float cr::video::CustomVFilter::getParam(VFilterParam id)
{
    // Lock mutex for reading parameters.
    const std::lock_guard<std::mutex> lock(m_paramsMutex);

    // Check param ID.
    switch (id)
    {
    case VFilterParam::MODE:
    {
        return static_cast<float>(m_params.mode);
    }
    case VFilterParam::LEVEL:
    {
        return m_params.level;
    }
    case VFilterParam::PROCESSING_TIME_MCSEC:
    {
        return static_cast<float>(m_params.processingTimeMcSec);
    }
    case VFilterParam::TYPE:
    {
        return static_cast<float>(m_params.type);
    }
    case VFilterParam::CUSTOM_1:
    {
        // Custom parameter.
        return m_params.custom1;
    }
    case VFilterParam::CUSTOM_2:
    {
        // Custom parameter.
        return m_params.custom2;
    }
    case VFilterParam::CUSTOM_3:
    {
        // Custom parameter.
        return m_params.custom3;
    }
    }

    return NO_PARAM_VALUE;
}



void cr::video::CustomVFilter::getParams(VFilterParams& params)
{
    // Lock mutex for assigning parameters.
    const std::lock_guard<std::mutex> lock(m_paramsMutex);
    params = m_params;
}



bool cr::video::CustomVFilter::executeCommand(VFilterCommand id)
{
    // Lock mutex for changing parameters.
    const std::lock_guard<std::mutex> lock(m_paramsMutex);

    // Check command ID.
    switch (id)
    {
    case VFilterCommand::RESET:
    {
        // The example has no internal state to reset.
        return true;
    }
    case VFilterCommand::ON:
    {
        m_params.mode = MODE_ON;
        return true;
    }
    case VFilterCommand::OFF:
    {
        m_params.mode = MODE_OFF;
        return true;
    }
    }

    return false;
}



bool cr::video::CustomVFilter::processFrame(cr::video::Frame&)
{
    // A real filter processes the frame here according to the mode and the
    // level and returns TRUE without processing if the filter is disabled.
    // The example does not change the frame.
    return true;
}



bool cr::video::CustomVFilter::setMask(cr::video::Frame mask)
{
    // Check the mask.
    if (!isMaskValid(mask))
    {
        return false;
    }

    // Lock mutex for setting the mask. Copying of a frame allocates memory.
    const std::lock_guard<std::mutex> lock(m_paramsMutex);
    try
    {
        m_mask = mask;
    }
    catch (const std::bad_alloc&)
    {
        return false;
    }

    return true;
}



bool cr::video::CustomVFilter::decodeAndExecuteCommand(
        uint8_t* data, int size)
{
    // Decode command. The decoder checks the pointer, the size, the version
    // and the IDs.
    VFilterCommand commandId{VFilterCommand::RESET};
    VFilterParam paramId{VFilterParam::LEVEL};
    float value{0.0f};
    const int result = VFilter::decodeCommand(
                data, size, paramId, commandId, value);

    // Execute the decoded command.
    if (result == 0)
    {
        return executeCommand(commandId);
    }
    if (result == 1)
    {
        return setParam(paramId, value);
    }

    return false;
}
