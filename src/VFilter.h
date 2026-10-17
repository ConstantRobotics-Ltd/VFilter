#pragma once
#include <cstdint>
#include <string>
#include "ConfigReader.h"
#include "Frame.h"



namespace cr
{
namespace video
{
/**
 * @brief Mask for VFilter library params for encoding (serializing).
 *
 * Each flag enables serialization of the VFilterParams field with the same
 * name (see VFilterParams::encode(...)).
 */
struct VFilterParamsMask
{
    /// Serialize mode field.
    bool mode{true};
    /// Serialize level field.
    bool level{true};
    /// Serialize processingTimeMcSec field.
    bool processingTimeMcSec{true};
    /// Serialize type field.
    bool type{true};
    /// Serialize custom1 field.
    bool custom1{true};
    /// Serialize custom2 field.
    bool custom2{true};
    /// Serialize custom3 field.
    bool custom3{true};
};



/**
 * @brief VFilter parameters class.
 *
 * The values of the fields are only transported by the class (JSON,
 * serialization): they are not checked against any range. The video filter
 * implementation must validate the values before use.
 *
 * @note The class is not thread-safe: an object must not be accessed from
 * several threads at the same time without external synchronization.
 */
class VFilterParams
{
public:

    /// Filter mode: 0 - off, 1 - on. Depends on implementation.
    int mode{1};
    /// Enhancement level for particular filter, as a percentage from
    /// 0% to 100%. May have another meaning depends on implementation.
    float level{0.0f};
    /// Processing time in microseconds. Read only parameter.
    int processingTimeMcSec{0};
    /// Type of the filter. Depends on the implementation.
    int type{0};
    /// VFilter custom parameter. Custom parameters used when particular image
    /// filter has specific unusual parameter.
    float custom1{0.0f};
    /// VFilter custom parameter. Custom parameters used when particular image
    /// filter has specific unusual parameter.
    float custom2{0.0f};
    /// VFilter custom parameter. Custom parameters used when particular image
    /// filter has specific unusual parameter.
    float custom3{0.0f};

    /// Macro from ConfigReader to make params readable / writable from JSON.
    JSON_READABLE(VFilterParams, mode, level, type, custom1, custom2, custom3)

    /**
     * @brief Encode (serialize) params.
     *
     * Integer and float fields are stored as raw 4 bytes values in the byte
     * order of the host. The size of the encoded data is 4 bytes of header
     * plus 4 bytes for each field selected by the mask.
     *
     * @param data Pointer to buffer to store serialized params. Must not be
     * nullptr. The buffer must be valid for bufferSize bytes.
     * @param bufferSize Size of buffer, bytes. Must be >= 32 bytes (size of
     * the encoded data with all fields selected).
     * @param size Size of encoded (serialized) data, bytes. Will be
     * <= bufferSize. Set to 0 if the method fails.
     * @param mask Pointer to mask structure. Used to exclude particular
     * params from encoding (from serialization). If nullptr all params are
     * encoded.
     * @return TRUE if params encoded (serialized) or FALSE if:
     * 1. data is nullptr.
     * 2. bufferSize less than 32.
     * If FALSE is returned nothing is written to the buffer.
     */
    bool encode(uint8_t* data, int bufferSize, int& size,
                VFilterParamsMask* mask = nullptr);

    /**
     * @brief Decode (deserialize) params encoded by encode(...) method.
     *
     * The method checks the data type, the version (major and minor) and the
     * size of the data (it must be equal to the size defined by the mask in
     * the data) before reading each field. Fields not selected by the mask in
     * the data are set to zero. Values of the fields are not checked against
     * any range.
     *
     * @param data Pointer to buffer with encoded (serialized) params. Must
     * not be nullptr. The buffer must be valid for dataSize bytes.
     * @param dataSize Size of encoded (serialized) params, bytes.
     * @return TRUE if params decoded or FALSE if not (data is nullptr, wrong
     * data type, version or size, unknown flag in the mask). If FALSE is
     * returned the object is not changed.
     */
    bool decode(uint8_t* data, int dataSize);
};



/**
 * @brief Enum of VFilter params.
 *
 * CUSTOM_3 is the last value: VFilter::decodeCommand(...) accepts the IDs
 * from MODE to CUSTOM_3.
 */
enum class VFilterParam
{
    /// Filter mode: 0 - off, 1 - on. Depends on implementation.
    MODE = 1,
    /// Enhancement level for particular filter, as a percentage from
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
 *
 * OFF is the last value: VFilter::decodeCommand(...) accepts the IDs from
 * RESET to OFF.
 */
enum class VFilterCommand
{
    /// Reset video filter algorithm.
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
     * @note Thread-safe.
     * @return A string representing the version: "Major.Minor.Patch".
     */
    static std::string getVersion();

    /**
     * @brief Initialize video filter. The particular filter should initialize
     * only supported parameters from VFilterParams class.
     * @note Thread safety is defined by the implementation.
     * @param params Parameters class.
     * @return TRUE if the video filter is initialized or FALSE if not.
     */
    virtual bool initVFilter(VFilterParams& params) = 0;

    /**
     * @brief Set the value for a specific library parameter.
     * @note The implementation must be thread-safe.
     * @param id The identifier of the library parameter.
     * @param value The value to set for the parameter.
     * @return TRUE if the parameter was successfully set, FALSE otherwise.
     */
    virtual bool setParam(VFilterParam id, float value) = 0;

    /**
     * @brief Get the value of a specific library parameter.
     * @note The implementation must be thread-safe.
     * @param id The identifier of the library parameter.
     * @return The value of the specified parameter or -1 if the parameter
     * is not supported by the implementation.
     */
    virtual float getParam(VFilterParam id) = 0;

    /**
     * @brief Get the structure containing all library parameters.
     * @note The implementation must be thread-safe.
     * @param params Parameters class.
     */
    virtual void getParams(VFilterParams& params) = 0;

    /**
     * @brief Execute a VFilter action command.
     * @note The implementation must be thread-safe.
     * @param id The identifier of the library command to be executed.
     * @return TRUE if the command was executed successfully, FALSE otherwise.
     */
    virtual bool executeCommand(VFilterCommand id) = 0;

    /**
     * @brief Process frame.
     *
     * The implementation must keep the resolution (width and height),
     * sourceId and frameId of the frame. If the filter is disabled the method
     * returns TRUE without processing the frame.
     *
     * @note The implementation must be thread-safe.
     * @param frame Reference to frame (input and result frame).
     * @return TRUE if frame processed or FALSE if not.
     */
    virtual bool processFrame(cr::video::Frame& frame) = 0;

    /**
     * @brief Set mask for filter.
     * @note Thread safety is defined by the implementation.
     * @param mask Filter binary mask. Frame object. The filter must
     * support pixel formats for mask: GRAY, NV12, NV21, YV12 and YU12.
     * @return TRUE if mask was set or FALSE if not.
     */
    virtual bool setMask(cr::video::Frame mask) = 0;

    /**
     * @brief Encode set param command.
     *
     * The ID is not checked by the method: VFilter::decodeCommand(...)
     * rejects an ID which is not in the VFilterParam enum. The value is not
     * checked: its meaning belongs to the video filter implementation.
     *
     * @note Thread-safe.
     * @param data Pointer to data buffer. Must not be nullptr. The buffer
     * must be valid for 11 bytes.
     * @param size Size of encoded data. Size will be 11 bytes. Set to 0 if
     * data is nullptr (nothing is written).
     * @param id VFilter parameter id.
     * @param value VFilter parameter value.
     */
    static void encodeSetParamCommand(uint8_t* data, int& size,
                                      VFilterParam id, float value);

    /**
     * @brief Encode action command.
     *
     * The ID is not checked by the method: VFilter::decodeCommand(...)
     * rejects an ID which is not in the VFilterCommand enum.
     *
     * @note Thread-safe.
     * @param data Pointer to data buffer. Must not be nullptr. The buffer
     * must be valid for 7 bytes.
     * @param size Size of encoded data. Size will be 7 bytes. Set to 0 if
     * data is nullptr (nothing is written).
     * @param id VFilter command ID.
     */
    static void encodeCommand(uint8_t* data, int& size, VFilterCommand id);

    /**
     * @brief Decode command.
     *
     * The method checks the pointer, the size (exactly 7 bytes for action
     * command and 11 bytes for set param command), the version (major and
     * minor), the data type and the ID (it must be in the enum). The output
     * arguments are changed only if the command is decoded.
     *
     * @note Thread-safe.
     * @param data Pointer to command data. Must not be nullptr. The buffer
     * must be valid for size bytes.
     * @param size Size of data, bytes.
     * @param paramId Output parameter ID in case decoding set param command.
     * @param commandId Output command ID in case decoding action command.
     * @param value Parameter value in case decoding set parameter command.
     * Set to 0 in case decoding action command.
     * @return 0 - action command decoded, 1 - set param command decoded,
     * -1 - error.
     */
    static int decodeCommand(uint8_t* data, int size, VFilterParam& paramId,
                             VFilterCommand& commandId, float& value);

    /**
     * @brief Decode and execute command.
     * @note The implementation must be thread-safe.
     * @param data Pointer to command data.
     * @param size Size of data.
     * @return TRUE if command decoded and executed or FALSE if not.
     */
    virtual bool decodeAndExecuteCommand(uint8_t* data, int size) = 0;

protected:

    /**
     * @brief Default constructor. The class can be used as a base only.
     */
    VFilter() = default;

    /**
     * @brief Copy constructor.
     * @param src Source object.
     */
    VFilter(const VFilter& src) = default;

    /**
     * @brief Move constructor.
     * @param src Source object.
     */
    VFilter(VFilter&& src) = default;

    /**
     * @brief Copy assignment operator.
     * @param src Source object.
     * @return Reference to this object.
     */
    VFilter& operator=(const VFilter& src) = default;

    /**
     * @brief Move assignment operator.
     * @param src Source object.
     * @return Reference to this object.
     */
    VFilter& operator=(VFilter&& src) = default;
};
}
}
