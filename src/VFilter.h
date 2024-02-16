#include <string>
#include <cstdint>
#include "ConfigReader.h"



namespace cr
{
namespace video
{
/**
 * @brief Mask for PanTilt library params for encoding (serializing).
 */
struct VFilterParamsMask
{
    bool custom1{ true };
    bool custom2{ true };
    bool custom3{ true };
};

/**
 * @brief Class of PanTilt library parameters.
 */
class VFilterParams
{
public:

    /// Current filter mode, usually 0 - off, 1 - on.
    int mode{ 0 };
    /// Enhancement level for particular filter, as a percentage in range from 
    /// 0% to 100%.
    int level{ 0 };
    /// Processing time in microseconds. Read only parameter.
    int processingTimeMcSec{ 0.0f };
    /// Type of the filter. Depends on the implementation.
    int type{ 0 };
    /// VFilter custom parameter. Custom parameters used when particular image 
    /// filter has specific unusual parameter.
    float custom1{ 0.0f };
    /// VFilter custom parameter. Custom parameters used when particular image 
    /// filter has specific unusual parameter.
    float custom2{ 0.0f };
    /// VFilter custom parameter. Custom parameters used when particular image 
    /// filter has specific unusual parameter.
    float custom3{ 0.0f };

    /// Macro from ConfigReader to make params readable/writable from JSON.
    JSON_READABLE(mode, level, processingTimeMcSec, type,
        custom1, custom2, custom3)

    /// operator =
    VFilterParams& operator= (const VFilterParams& src);

    /**
     * @brief Encode (serialize) params.
     * @param data Pointer to buffer to store serialized params.
     * @param  bufferSize Size of buffer.
     * @param size Size of encoded (serialized) data. Will be <= bufferSize.
     * @param mask Pointer to mask structure. Used to exclude particular
     * params from encoding (from serialization).
     * @return TRUE if params encoded (serialized) or FALSE if:
     * 1. bufferSize less tha size of serialized params.
     * 2. Can be other errors depends on implementation.
     */
    bool encode(uint8_t* data, int bufferSize, int& size,
                                            VFilterParamsMask* mask = nullptr);

    /**
     * @brief Decode (deserialize) params.
     * @param data Pointer to buffer with encoded (serialized) params.
     * @param dataSize Size of encoded (serialized) params.
     * @return TRUE if params decoded, FALSE otherwise.
     */
    bool decode(uint8_t* data, int dataSize);
};

/**
 * @brief Enum of PanTilt library params.
 */
enum class VFilterParam
{
	/// Current filter mode, usually 0 - off, 1 - on.
	MODE = 1,
	/// Enhancement level for particular filter, as a percentage in range from 
	/// 0% to 100%.
	LEVEL,
	/// Processing time in microseconds. Read only parameter.
	PROCESSING_TIME_MCSEC,
	/// Type of the filter. Depends on the implementation.
	TYPE,
	/// VFilter custom parameter. Custom parameters used when particular image 
	/// filter has specific unusual parameter.
	CUSTOM_1,
	/// VFilter custom parameter. Custom parameters used when particular image 
	/// filter has specific unusual parameter.
	CUSTOM_2,
	/// VFilter custom parameter. Custom parameters used when particular image 
	/// filter has specific unusual parameter.
	CUSTOM_3
};

/**
 * @brief Enum of PanTilt library commands.
 */
enum class VFilterCommand
{

    /// Restart Pan-Tilt device.
    RESTART = 1,
    /// Stop Pan-Tilt device, block all running commands and left device in current state.
    STOP
};

/**
 * @brief Library PanTilt class with PanTilt methods.
 */
class VFilter
{
public:

    /**
     * @brief Class destructor.
     */
    virtual ~VFilter();

    /**
     * @brief Get the version of the PanTilt class.
     * @return A string representing the version: "Major.Minor.Patch"
     */
    static std::string getVersion();

    /**
     * @brief Set the value for a specific library parameter.
     * @param id The identifier of the library parameter.
     * @param value The value to set for the parameter.
     * @return TRUE if the parameter was successfully set, FALSE otherwise.
     */
    virtual bool setParam(VFilterParam id, float value) = 0;

    /**
     * @brief Get the value of a specific library parameter.
     * @param id The identifier of the library parameter.
     * @return The value of the specified parameter.
     */
    virtual float getParam(VFilterParam id) = 0;

    /**
     * @brief Get the structure containing all library parameters.
     * @param params Reference to a PanTiltParams structure.
     */
    virtual void getParams(VFilterParams& params) = 0;

    /**
     * @brief Execute a PanTilt command.
     * @param id The identifier of the library command to be executed.
     * @param arg1 The argument value used by the command.
     * @param arg2 The argument value used by the command.
     * @return TRUE if the command was executed successfully, FALSE otherwise.
     */
    virtual bool executeCommand(VFilterCommand id,
                                      float arg1 = 0.0f, float arg2 = 0.0f) = 0;

    /**
     * @brief Encode set param command.
     * @param data Pointer to data buffer. Must have size >= 11.
     * @param size Size of encoded data.
     * @param id PanTilt parameter id.
     * @param value PanTilt parameter value.
     */
    static void encodeSetParamCommand(uint8_t* data, int& size, VFilterParam id,
                                                                   float value);

    /**
     * @brief Encode command.
     * @param data Pointer to data buffer. Must have size >= 15.
     * @param size Size of encoded data.
     * @param id PanTilt command ID.
     * @param arg1 The argument value used by the command.
     * @param arg2 The argument value used by the command.
     */
    static void encodeCommand(uint8_t* data, int& size, VFilterCommand id, 
                                        float arg1 = 0.0f, float arg2 = 0.0f);

    /**
     * @brief Decode command.
     * @param data Pointer to command data.
     * @param size Size of data.
     * @param paramId Output command ID.
     * @param commandId Output command ID.
     * @param value Param or command value.
     * @return 0 - command decoded, 1 - set param command decoded, -1 - error.
     */
    static int decodeCommand(uint8_t* data,
        int size,
        VFilterParam& paramId,
        VFilterCommand& commandId,
        float& value);

    /**
     * @brief Decode and execute command.
     * @param data Pointer to command data.
     * @param size Size of data.
     * @return TRUE if command decoded and executed or FALSE if not.
     */
    virtual bool decodeAndExecuteCommand(uint8_t* data, int size) = 0;
};
}
}