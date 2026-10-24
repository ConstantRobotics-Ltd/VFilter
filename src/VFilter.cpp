#include "VFilter.h"
#include "VFilterVersion.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>



namespace
{
// The encoded data stores int and float values as raw 4 bytes values in the
// byte order of the host.
static_assert(sizeof(int) == 4, "int must be 4 bytes");
static_assert(sizeof(float) == 4, "float must be 4 bytes");
// Major and minor version are stored in one byte each.
static_assert(VFILTER_MAJOR_VERSION >= 0 &&
              VFILTER_MAJOR_VERSION <= std::numeric_limits<uint8_t>::max(),
              "major version must fit in a byte");
static_assert(VFILTER_MINOR_VERSION >= 0 &&
              VFILTER_MINOR_VERSION <= std::numeric_limits<uint8_t>::max(),
              "minor version must fit in a byte");

/// Size of an int or float value in the encoded data, bytes.
constexpr int VALUE_SIZE{4};

/// Data type of the action command: first byte of the encoded data.
constexpr uint8_t COMMAND_DATA_TYPE{0x00};
/// Data type of the set param command: first byte of the encoded data.
constexpr uint8_t SET_PARAM_DATA_TYPE{0x01};
/// Data type of the encoded params: first byte of the encoded data.
constexpr uint8_t PARAMS_DATA_TYPE{0x02};
/// Position of the major version in the encoded data. The minor version
/// follows it.
constexpr int MAJOR_VERSION_POS{1};
/// Position of the minor version in the encoded data.
constexpr int MINOR_VERSION_POS{2};

/// Position of the command ID (parameter ID) in the encoded command.
constexpr int COMMAND_ID_POS{3};
/// Position of the parameter value in the encoded set param command.
constexpr int COMMAND_VALUE_POS{COMMAND_ID_POS + VALUE_SIZE};
/// Size of the encoded action command: data type, version and ID.
constexpr int COMMAND_SIZE{COMMAND_ID_POS + VALUE_SIZE};
/// Size of the encoded set param command: action command and value.
constexpr int SET_PARAM_COMMAND_SIZE{COMMAND_SIZE + VALUE_SIZE};
static_assert(COMMAND_SIZE == 7, "wrong size of the encoded command");
static_assert(SET_PARAM_COMMAND_SIZE == 11,
              "wrong size of the encoded set param command");

/// Number of fields of VFilterParams in the encoded params.
constexpr int FIELDS_NUMBER{7};
/// Position of the mask in the encoded params. The mask has one byte.
constexpr int PARAMS_MASK_POS{3};
/// Size of the encoded params header: data type, version and mask.
constexpr int PARAMS_HEADER_SIZE{PARAMS_MASK_POS + 1};
/// Maximum size of the encoded params (all fields are encoded).
constexpr int PARAMS_MAX_SIZE{PARAMS_HEADER_SIZE + FIELDS_NUMBER * VALUE_SIZE};
static_assert(PARAMS_MAX_SIZE == 32, "wrong size of the encoded params");

/// Bit of the mask which belongs to the first field. The bit of the next field
/// is the next less significant one.
constexpr int FIRST_MASK_BIT{0x80};
/// Bits of the mask byte which do not belong to any field. The encoder writes
/// zero, the decoder rejects the data with such a bit set: the data was not
/// produced by the encoder.
constexpr uint8_t RESERVED_MASK_BITS{0x01};
static_assert(((FIRST_MASK_BIT >> (FIELDS_NUMBER - 1)) >> 1) ==
              RESERVED_MASK_BITS, "wrong reserved bits of the mask");



/// Get the mask bit of the field with the given index in the field table.
uint8_t maskBit(int index)
{
    return static_cast<uint8_t>(FIRST_MASK_BIT >> index);
}



/// Get the table of the pointers to the VFilterParams fields in the order they
/// are stored in the encoded data (the same order as the bits of the mask).
/// The same table is used by the encoder and by the decoder, so the format
/// cannot diverge.
std::array<void*, FIELDS_NUMBER> makeFieldTable(
        rapidpixel::VFilterParams& params)
{
    return {{&params.mode, &params.level, &params.processingTimeMcSec,
             &params.type, &params.custom1, &params.custom2,
             &params.custom3}};
}



/// Get the table of the mask flags in the order of the field table.
std::array<bool, FIELDS_NUMBER> makeMaskTable(
        const rapidpixel::VFilterParamsMask& mask)
{
    return {{mask.mode, mask.level, mask.processingTimeMcSec, mask.type,
             mask.custom1, mask.custom2, mask.custom3}};
}
}



bool rapidpixel::VFilterParams::encode(uint8_t* data, int bufferSize, int& size,
                                       VFilterParamsMask* mask)
{
    // Check the pointer and the buffer size (the buffer must be able to store
    // all fields: the size of the encoded data depends on the mask).
    size = 0;
    if (data == nullptr || bufferSize < PARAMS_MAX_SIZE)
    {
        return false;
    }

    // Prepare params mask.
    VFilterParamsMask paramsMask;
    if (mask != nullptr)
    {
        paramsMask = *mask;
    }

    // Fill the header.
    data[0] = PARAMS_DATA_TYPE;
    data[MAJOR_VERSION_POS] = VFILTER_MAJOR_VERSION;
    data[MINOR_VERSION_POS] = VFILTER_MINOR_VERSION;

    // Copy selected params to the buffer and fill the bit mask for them.
    const auto fields = makeFieldTable(*this);
    const auto selected = makeMaskTable(paramsMask);
    uint8_t maskByte = 0x00;
    int pos = PARAMS_HEADER_SIZE;
    for (int i = 0; i < FIELDS_NUMBER; ++i)
    {
        if (!selected[static_cast<size_t>(i)])
        {
            continue;
        }
        maskByte = static_cast<uint8_t>(maskByte | maskBit(i));
        std::memcpy(&data[pos], fields[static_cast<size_t>(i)], VALUE_SIZE);
        pos += VALUE_SIZE;
    }
    data[PARAMS_MASK_POS] = maskByte;
    size = pos;

    return true;
}



bool rapidpixel::VFilterParams::decode(uint8_t* data, int dataSize)
{
    // Check the pointer and the size (the header must be complete).
    if (data == nullptr || dataSize < PARAMS_HEADER_SIZE)
    {
        return false;
    }

    // Check the data type and the version.
    if (data[0] != PARAMS_DATA_TYPE ||
        data[MAJOR_VERSION_POS] != VFILTER_MAJOR_VERSION ||
        data[MINOR_VERSION_POS] != VFILTER_MINOR_VERSION)
    {
        return false;
    }
    const uint8_t maskByte = data[PARAMS_MASK_POS];
    if ((maskByte & RESERVED_MASK_BITS) != 0)
    {
        return false;
    }

    // Decode into a temporary object: this object is changed only if the
    // whole data is valid. Fields not selected by the mask are set to zero.
    VFilterParams result;
    const auto fields = makeFieldTable(result);
    int pos = PARAMS_HEADER_SIZE;
    for (int i = 0; i < FIELDS_NUMBER; ++i)
    {
        void* field = fields[static_cast<size_t>(i)];
        if ((maskByte & maskBit(i)) == 0)
        {
            std::memset(field, 0, VALUE_SIZE);
            continue;
        }

        // Check that the field is inside the data (pos <= dataSize here).
        if (dataSize - pos < VALUE_SIZE)
        {
            return false;
        }
        std::memcpy(field, &data[pos], VALUE_SIZE);
        pos += VALUE_SIZE;
    }

    // The size of the encoded data is defined by the mask: extra bytes mean
    // that the data is damaged or is not the data of this class.
    if (pos != dataSize)
    {
        return false;
    }

    *this = result;

    return true;
}



rapidpixel::VFilter::~VFilter() = default;



std::string rapidpixel::VFilter::getVersion()
{
    return VFILTER_VERSION;
}



void rapidpixel::VFilter::encodeSetParamCommand(
        uint8_t* data, int& size, VFilterParam id, float value)
{
    size = 0;
    if (data == nullptr)
    {
        return;
    }

    // Fill header.
    data[0] = SET_PARAM_DATA_TYPE;
    data[MAJOR_VERSION_POS] = VFILTER_MAJOR_VERSION;
    data[MINOR_VERSION_POS] = VFILTER_MINOR_VERSION;

    // Fill data.
    const int paramId = static_cast<int>(id);
    std::memcpy(&data[COMMAND_ID_POS], &paramId, VALUE_SIZE);
    std::memcpy(&data[COMMAND_VALUE_POS], &value, VALUE_SIZE);
    size = SET_PARAM_COMMAND_SIZE;
}



void rapidpixel::VFilter::encodeCommand(
        uint8_t* data, int& size, VFilterCommand id)
{
    size = 0;
    if (data == nullptr)
    {
        return;
    }

    // Fill header.
    data[0] = COMMAND_DATA_TYPE;
    data[MAJOR_VERSION_POS] = VFILTER_MAJOR_VERSION;
    data[MINOR_VERSION_POS] = VFILTER_MINOR_VERSION;

    // Fill data.
    const int commandId = static_cast<int>(id);
    std::memcpy(&data[COMMAND_ID_POS], &commandId, VALUE_SIZE);
    size = COMMAND_SIZE;
}



int rapidpixel::VFilter::decodeCommand(
        uint8_t* data, int size, VFilterParam& paramId,
        VFilterCommand& commandId, float& value)
{
    // Check the pointer and the size (both commands have the header and ID).
    if (data == nullptr || size < COMMAND_SIZE)
    {
        return -1;
    }

    // Check version.
    if (data[MAJOR_VERSION_POS] != VFILTER_MAJOR_VERSION ||
        data[MINOR_VERSION_POS] != VFILTER_MINOR_VERSION)
    {
        return -1;
    }

    // Extract the ID.
    int rawId = 0;
    std::memcpy(&rawId, &data[COMMAND_ID_POS], VALUE_SIZE);

    // Check command type. The size of each command is fixed: a longer message
    // is an error. The ID must be in the range of the enum: an unknown ID is
    // not passed to the implementation. The output arguments are changed only
    // if the command is decoded.
    if (data[0] == COMMAND_DATA_TYPE)
    {
        if (size != COMMAND_SIZE ||
            rawId < static_cast<int>(VFilterCommand::RESET) ||
            rawId > static_cast<int>(VFilterCommand::OFF))
        {
            return -1;
        }
        commandId = static_cast<VFilterCommand>(rawId);
        value = 0.0f;
        return 0;
    }
    if (data[0] == SET_PARAM_DATA_TYPE)
    {
        if (size != SET_PARAM_COMMAND_SIZE ||
            rawId < static_cast<int>(VFilterParam::MODE) ||
            rawId > static_cast<int>(VFilterParam::CUSTOM_3))
        {
            return -1;
        }
        float paramValue = 0.0f;
        std::memcpy(&paramValue, &data[COMMAND_VALUE_POS], VALUE_SIZE);
        paramId = static_cast<VFilterParam>(rawId);
        value = paramValue;
        return 1;
    }

    return -1;
}
