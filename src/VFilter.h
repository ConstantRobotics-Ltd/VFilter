#include <string>
#include <cstdint>
#include "ConfigReader.h"
#include "Frame.h"



namespace cr
{
namespace video
{
/**
 * @brief Mask for VFilter library params for encoding (serializing).
 */
struct VFilterParamsMask
{
    bool mode{ true };
    bool level{ true };
    bool processingTimeMcSec{ true };
    bool type{ true };
    bool custom1{ true };
    bool custom2{ true };
    bool custom3{ true };
};



/**
 * @brief VFilter parameters class.
 */
class VFilterParams
{
public:

    /// Current filter mode: 0 - off, 1 - on. Depends on implementation.
    int mode{ 0 };
    /// Enhancement level for particular filter, as a percentage in range from 
    /// 0% to 100%. May have another meaning depends on implementation.
    float level{ 0 };
    /// Processing time in microseconds. Read only parameter.
    int processingTimeMcSec{ 0 };
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
    JSON_READABLE(VFilterParams, mode, level, type, custom1, custom2, custom3)

    /// operator =
    VFilterParams& operator= (const VFilterParams& src);

    /**
     * @brief Encode (serialize) params.
     * @param data Pointer to buffer to store serialized params.
     * @param bufferSize Size of buffer.
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
 * @brief Enum of VFilter params.
 */
enum class VFilterParam
{
	/// Current filter mode: 0 - off, 1 - on. Depends on implementation.
	MODE = 1,
	/// Enhancement level for particular filter, as a percentage in range from 
	/// 0% to 100%. May have another meaning depends on implementation.
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
 * @brief Enum of VFilter action commands.
 */
enum class VFilterCommand
{
    /// Reset image filter algorithm.
    RESET = 1,
    /// Enable filter.
    ON,
    /// Disable filter.
    OFF
};



/**
 * @brief Video filter interface class.
 */
class VFilter
{
public:

    /**
     * @brief Class destructor.
     */
    virtual ~VFilter();

    /**
     * @brief Get the version of the VFilter class.
     * @return A string representing the version: "Major.Minor.Patch"
     */
    static std::string getVersion();

    /**
     * @brief Initialize video filter. The particular filter should initialize
     * only supported parameters from VFilterParams class.
     * @param params Parameters class.
     * @return TRUE if the video filter is initialized or FALSE if not.
     */
    virtual bool initVFilter(VFilterParams& params) = 0;

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
     * @param params Reference to a VFilterParams structure.
     */
    virtual void getParams(VFilterParams& params) = 0;

    /**
     * @brief Execute a VFilter action command.
     * @param id The identifier of the library command to be executed.
     * @return TRUE if the command was executed successfully, FALSE otherwise.
     */
    virtual bool executeCommand(VFilterCommand id) = 0;

    /**
	 * @brief Process frame.
	 * @param frame Reference to frame.
	 * @return TRUE if frame processed or FALSE if not.
	 */
    virtual bool processFrame(cr::video::Frame& frame) = 0;

    /**
    * @brief Set mask for filter. 
    * @param mask Filter binary mask. Frame object. The filter must
    * support pixel formats for mask: GRAY, NV12, NV21, YV12 and YU12.
    * @return TRUE if mask was set or FALSE if not.
    */
    virtual bool setMask(cr::video::Frame mask) = 0;

    /**
     * @brief Encode set param command.
     * @param data Pointer to data buffer. Must have size >= 11.
     * @param size Size of encoded data.
     * @param id VFilter parameter id.
     * @param value VFilter parameter value.
     */
    static void encodeSetParamCommand(uint8_t* data, int& size,
                                      VFilterParam id, float value);

    /**
     * @brief Encode command.
     * @param data Pointer to data buffer. Must have size >= 15.
     * @param size Size of encoded data.
     * @param id VFilter command ID.
     * @param arg1 The argument value used by the command.
     * @param arg2 The argument value used by the command.
     */
    static void encodeCommand(uint8_t* data, int& size, VFilterCommand id);

    /**
     * @brief Decode command.
     * @param data Pointer to command data.
     * @param size Size of data.
     * @param paramId Output command ID.
     * @param commandId Output command ID.
     * @param value Param or command value.
     * @return 0 - command decoded, 1 - set param command decoded, -1 - error.
     */
    static int decodeCommand(uint8_t* data, int size, VFilterParam& paramId,
                                    VFilterCommand& commandId, float& value);

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