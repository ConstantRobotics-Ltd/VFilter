#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
#include "ConfigReader.h"
#include "VFilter.h"
#include "VFilterVersion.h"



namespace
{
using cr::video::VFilter;
using cr::video::VFilterCommand;
using cr::video::VFilterParam;
using cr::video::VFilterParams;
using cr::video::VFilterParamsMask;

/// Number of fields of VFilterParams.
constexpr int FIELDS_NUMBER{7};
/// Size of the encoded params with all fields, bytes.
constexpr int PARAMS_MAX_SIZE{32};
/// Size of the encoded params header (data type, version, mask), bytes.
constexpr int PARAMS_HEADER_SIZE{4};
/// Size of the encoded action command, bytes.
constexpr int COMMAND_SIZE{7};
/// Size of the encoded set param command, bytes.
constexpr int SET_PARAM_COMMAND_SIZE{11};
/// Size of the buffers used by the tests, bytes.
constexpr int BUFFER_SIZE{64};
/// Byte used to fill buffers: a write of the library is detected as a change.
constexpr uint8_t CANARY{0xAA};
/// Number of the random masks in the encode / decode test.
constexpr int RANDOM_MASKS_NUMBER{5000};
/// Number of the damaged samples in the damaged data test.
constexpr int DAMAGED_SAMPLES_NUMBER{100000};
/// Name of the JSON file used by the JSON test.
const char* const JSON_FILE_NAME{"VFilterParams.json"};

/// State of the test random generator. The tests use own deterministic
/// generator: results must be reproducible on all platforms.
uint32_t g_randomState{20240915U};
/// Set to true if a file created by the tests can not be removed.
bool g_fileRemoveFailed{false};



/// Get next random number in [0, limit).
int randomInt(int limit)
{
    g_randomState = g_randomState * 1664525U + 1013904223U;
    return static_cast<int>((g_randomState >> 8) %
                            static_cast<uint32_t>(limit));
}



/// Get next random 32 bits value.
uint32_t randomBits()
{
    uint32_t result = 0U;
    for (int i = 0; i < 4; ++i)
    {
        result = (result << 8) | static_cast<uint32_t>(randomInt(256));
    }
    return result;
}



/// Get the bits of a float value.
uint32_t bitsOf(float value)
{
    uint32_t bits = 0U;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}



/// Get a float value by its bits.
float floatOf(uint32_t bits)
{
    float value = 0.0f;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}



/// Get the bits of an int value.
uint32_t bitsOf(int value)
{
    uint32_t bits = 0U;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}



/// Get a random float value: NaN, infinity and negative zero included.
float randomFloat()
{
    const std::array<uint32_t, 5> special{
        {0x7FC00000U, 0x7F800000U, 0xFF800000U, 0x80000000U, 0x7FA12345U}};
    if (randomInt(8) == 0)
    {
        return floatOf(special[static_cast<size_t>(randomInt(5))]);
    }
    return floatOf(randomBits());
}



/// Get params with random values in all fields.
VFilterParams randomParams()
{
    VFilterParams params;
    params.mode = static_cast<int>(randomBits());
    params.level = randomFloat();
    params.processingTimeMcSec = static_cast<int>(randomBits());
    params.type = static_cast<int>(randomBits());
    params.custom1 = randomFloat();
    params.custom2 = randomFloat();
    params.custom3 = randomFloat();
    return params;
}



/// Get a mask with random flags.
VFilterParamsMask randomMask()
{
    VFilterParamsMask mask;
    mask.mode = randomInt(2) == 0;
    mask.level = randomInt(2) == 0;
    mask.processingTimeMcSec = randomInt(2) == 0;
    mask.type = randomInt(2) == 0;
    mask.custom1 = randomInt(2) == 0;
    mask.custom2 = randomInt(2) == 0;
    mask.custom3 = randomInt(2) == 0;
    return mask;
}



/// Get the flags of the mask in the order of the fields of VFilterParams.
std::array<bool, FIELDS_NUMBER> flagsOf(const VFilterParamsMask& mask)
{
    return {{mask.mode, mask.level, mask.processingTimeMcSec, mask.type,
             mask.custom1, mask.custom2, mask.custom3}};
}



/// Get the number of the fields selected by the mask.
int selectedNumber(const VFilterParamsMask& mask)
{
    int result = 0;
    for (const bool flag : flagsOf(mask))
    {
        result += flag ? 1 : 0;
    }
    return result;
}



/// Compare a field: the output must be equal to the input if the field is
/// selected and equal to zero if not. The number of mismatches is incremented
/// for each wrong field.
template <class T>
void checkField(const char* name, T in, T out, bool selected, int& errors)
{
    const T expected = selected ? in : T{};
    if (bitsOf(expected) != bitsOf(out))
    {
        std::cout << "Mismatch in field " << name << std::endl;
        ++errors;
    }
}



/// Compare all fields of two params objects (floats by their bits). The
/// result params must be equal to the input params for the selected fields
/// and zero for the others. Returns the number of mismatches.
int compareParams(const VFilterParams& in, const VFilterParams& out,
                  const VFilterParamsMask& mask)
{
    int errors = 0;
    checkField("mode", in.mode, out.mode, mask.mode, errors);
    checkField("level", in.level, out.level, mask.level, errors);
    checkField("processingTimeMcSec", in.processingTimeMcSec,
               out.processingTimeMcSec, mask.processingTimeMcSec, errors);
    checkField("type", in.type, out.type, mask.type, errors);
    checkField("custom1", in.custom1, out.custom1, mask.custom1, errors);
    checkField("custom2", in.custom2, out.custom2, mask.custom2, errors);
    checkField("custom3", in.custom3, out.custom3, mask.custom3, errors);
    return errors;
}



/// Check that two params objects are identical (all fields, by bits).
bool sameParams(const VFilterParams& a, const VFilterParams& b)
{
    const VFilterParamsMask all;
    return compareParams(a, b, all) == 0;
}



/// Print a failed check and return false.
bool fail(const char* message)
{
    std::cout << message << std::endl;
    return false;
}



/// Fill the buffer with the canary byte.
void fillCanary(std::array<uint8_t, BUFFER_SIZE>& buffer)
{
    buffer.fill(CANARY);
}



/// Check that the buffer has the canary byte at all positions from the start
/// position.
bool isCanary(const std::array<uint8_t, BUFFER_SIZE>& buffer, size_t start)
{
    for (size_t i = start; i < buffer.size(); ++i)
    {
        if (buffer[i] != CANARY)
        {
            return false;
        }
    }
    return true;
}



/// Encode the params with all fields into the buffer, return the size of the
/// encoded data (0 on failure).
int encodeAll(VFilterParams& params, std::array<uint8_t, BUFFER_SIZE>& buffer)
{
    int size = 0;
    if (!params.encode(buffer.data(), BUFFER_SIZE, size))
    {
        return 0;
    }
    return size;
}



/// Get the version string built from the version macros.
std::string versionString()
{
    return std::to_string(VFILTER_MAJOR_VERSION) + "." +
           std::to_string(VFILTER_MINOR_VERSION) + "." +
           std::to_string(VFILTER_PATCH_VERSION);
}



/// Removes the file in the destructor. A failed removal is reported and
/// remembered: the test application returns an error code.
class FileRemover
{
public:

    /// Class constructor. The file is removed by the destructor.
    explicit FileRemover(std::string name) : m_name(std::move(name))
    {
    }

    /// Class destructor. Does not throw.
    ~FileRemover()
    {
        std::error_code error;
        std::filesystem::remove(m_name, error);
        std::error_code existsError;
        if (error || std::filesystem::exists(m_name, existsError))
        {
            std::cout << "Can't remove file " << m_name << std::endl;
            g_fileRemoveFailed = true;
        }
    }

    FileRemover(const FileRemover&) = delete;
    FileRemover(FileRemover&&) = delete;
    FileRemover& operator=(const FileRemover&) = delete;
    FileRemover& operator=(FileRemover&&) = delete;

private:

    /// Name of the file to remove.
    std::string m_name;
};



/// Run one test and print the result.
bool runTest(const char* title, bool (*test)())
{
    std::cout << title << ":" << std::endl;
    const bool passed = test();
    std::cout << (passed ? "OK" : "ERROR") << std::endl << std::endl;
    return passed;
}



/// Copy constructor and assignment of params.
bool copyParametersTest()
{
    const VFilterParams params1 = randomParams();

    // Copy constructor.
    const VFilterParams params2 = params1;
    if (!sameParams(params1, params2))
    {
        return fail("Copy constructor: params are not equal");
    }

    // Assignment.
    VFilterParams params3;
    params3 = params1;
    if (!sameParams(params1, params3))
    {
        return fail("Assignment: params are not equal");
    }

    return true;
}



/// Encode and decode with all fields: the size is 32 bytes.
bool encodeDecodeTestWithoutMask()
{
    VFilterParams params1 = randomParams();

    // Without the mask and with the mask which selects all fields.
    for (int useMask = 0; useMask < 2; ++useMask)
    {
        VFilterParamsMask mask;
        std::array<uint8_t, BUFFER_SIZE> buffer;
        fillCanary(buffer);
        int size = 0;
        if (!params1.encode(buffer.data(), BUFFER_SIZE, size,
                            useMask != 0 ? &mask : nullptr))
        {
            return fail("Can't encode params");
        }
        if (size != PARAMS_MAX_SIZE)
        {
            return fail("Wrong size of the encoded params");
        }
        if (!isCanary(buffer, static_cast<size_t>(size)))
        {
            return fail("Encoder wrote after the encoded data");
        }

        VFilterParams params2;
        if (!params2.decode(buffer.data(), size))
        {
            return fail("Can't decode params");
        }
        if (compareParams(params1, params2, mask) != 0)
        {
            return false;
        }
    }

    return true;
}



/// Encode and decode with random masks: size, mask byte and fields.
bool encodeDecodeTestWithMask()
{
    for (int i = 0; i < RANDOM_MASKS_NUMBER; ++i)
    {
        VFilterParams params1 = randomParams();
        VFilterParamsMask mask = randomMask();
        std::array<uint8_t, BUFFER_SIZE> buffer;
        fillCanary(buffer);
        int size = 0;
        if (!params1.encode(buffer.data(), BUFFER_SIZE, size, &mask))
        {
            return fail("Can't encode params");
        }
        if (size != PARAMS_HEADER_SIZE + 4 * selectedNumber(mask))
        {
            return fail("Wrong size of the encoded params");
        }
        if (!isCanary(buffer, static_cast<size_t>(size)))
        {
            return fail("Encoder wrote after the encoded data");
        }

        // Decode into an object with other values: fields which are not in
        // the data must be zero.
        VFilterParams params2 = randomParams();
        if (!params2.decode(buffer.data(), size))
        {
            return fail("Can't decode params");
        }
        if (compareParams(params1, params2, mask) != 0)
        {
            return false;
        }
    }

    return true;
}



/// Buffer boundaries of the encoder: null pointer, small buffers, exact size.
bool encodeBufferTest()
{
    VFilterParams params = randomParams();
    VFilterParamsMask mask;

    // Null pointer: rejected, the size is set to 0.
    int size = 777;
    if (params.encode(nullptr, BUFFER_SIZE, size, &mask) || size != 0)
    {
        return fail("Null pointer is not rejected");
    }

    // Buffer is smaller than the size of the encoded params with all fields:
    // rejected, the size is set to 0, nothing is written.
    const std::array<int, 6> badSizes{
        {std::numeric_limits<int>::min(), -1, 0, 1, PARAMS_HEADER_SIZE,
         PARAMS_MAX_SIZE - 1}};
    for (const int bufferSize : badSizes)
    {
        std::array<uint8_t, BUFFER_SIZE> buffer;
        fillCanary(buffer);
        size = 777;
        if (params.encode(buffer.data(), bufferSize, size, &mask))
        {
            return fail("Small buffer is not rejected");
        }
        if (size != 0 || !isCanary(buffer, 0))
        {
            return fail("Size is not 0 or buffer is changed after an error");
        }
    }

    // Buffer of the exact size (heap buffer: sanitizers see every overrun).
    std::vector<uint8_t> exact(static_cast<size_t>(PARAMS_MAX_SIZE));
    size = 0;
    if (!params.encode(exact.data(), PARAMS_MAX_SIZE, size, &mask) ||
        size != PARAMS_MAX_SIZE)
    {
        return fail("Buffer of the exact size is not accepted");
    }

    return true;
}



/// Checks that the decoder rejects data and does not change the object.
class RejectChecker
{
public:

    /// Class constructor. The object has random values.
    RejectChecker() : m_object(randomParams()), m_before(m_object)
    {
    }

    /// Decode a copy of the data and check that it is rejected. The data of
    /// BUFFER_SIZE bytes is copied, the pointer can be nullptr. A failed check
    /// is printed and remembered.
    void check(const uint8_t* data, int size, const char* what)
    {
        std::array<uint8_t, BUFFER_SIZE> copy;
        copy.fill(0);
        if (data != nullptr)
        {
            std::memcpy(copy.data(), data, BUFFER_SIZE);
        }
        if (m_object.decode(data != nullptr ? copy.data() : nullptr, size))
        {
            std::cout << "Accepted: " << what << " (size " << size << ")"
                      << std::endl;
            m_passed = false;
        }
        else if (!sameParams(m_object, m_before))
        {
            std::cout << "Object changed after rejected call: " << what
                      << " (size " << size << ")" << std::endl;
            m_passed = false;
        }
    }

    /// Get the result: true if all checks passed.
    bool passed() const
    {
        return m_passed;
    }

private:

    /// Object which decodes the data.
    VFilterParams m_object;
    /// Copy of the object before the calls.
    VFilterParams m_before;
    /// Result of the checks.
    bool m_passed{true};
};



/// Decoder rejects data of wrong size, type and version. The object is not
/// changed after a rejected call.
bool decodeInvalidDataTest()
{
    VFilterParams source = randomParams();
    std::array<uint8_t, BUFFER_SIZE> valid;
    fillCanary(valid);
    const int validSize = encodeAll(source, valid);
    if (validSize != PARAMS_MAX_SIZE)
    {
        return fail("Can't encode params");
    }

    RejectChecker checker;

    // Null pointer, sizes out of range.
    checker.check(nullptr, validSize, "null pointer");
    checker.check(valid.data(), std::numeric_limits<int>::min(),
                  "INT_MIN size");
    checker.check(valid.data(), -1, "negative size");
    checker.check(valid.data(), 0, "zero size");
    checker.check(valid.data(), std::numeric_limits<int>::max(),
                  "INT_MAX size");

    // Every truncation.
    for (int size = 0; size < validSize; ++size)
    {
        checker.check(valid.data(), size, "truncated data");
    }

    // Trailing bytes.
    for (int extra = 1; extra <= 8; ++extra)
    {
        checker.check(valid.data(), validSize + extra, "trailing bytes");
    }

    // Every wrong value of the data type and of the version bytes.
    for (int pos = 0; pos < 3; ++pos)
    {
        for (int value = 0; value < 256; ++value)
        {
            if (value == valid[static_cast<size_t>(pos)])
            {
                continue;
            }
            std::array<uint8_t, BUFFER_SIZE> data = valid;
            data[static_cast<size_t>(pos)] = static_cast<uint8_t>(value);
            checker.check(data.data(), validSize, "wrong header byte");
        }
    }

    return checker.passed();
}



/// Every value of the mask byte: the size must be equal to the size defined
/// by the mask, bit 0 is not assigned to any field and must be zero.
bool decodeMaskTest()
{
    std::array<uint8_t, BUFFER_SIZE> valid;
    fillCanary(valid);
    VFilterParams source = randomParams();
    if (encodeAll(source, valid) != PARAMS_MAX_SIZE)
    {
        return fail("Can't encode params");
    }

    RejectChecker checker;
    bool result = true;
    for (int mask = 0; mask < 256; ++mask)
    {
        std::array<uint8_t, BUFFER_SIZE> data = valid;
        data[3] = static_cast<uint8_t>(mask);
        int fields = 0;
        for (int bit = 1; bit < 8; ++bit)
        {
            fields += ((mask >> bit) & 1) != 0 ? 1 : 0;
        }
        const int exactSize = PARAMS_HEADER_SIZE + 4 * fields;
        if ((mask & 1) != 0)
        {
            checker.check(data.data(), exactSize, "reserved mask bit");
            continue;
        }
        checker.check(data.data(), exactSize - 1, "truncated by the mask");
        checker.check(data.data(), exactSize + 1,
                      "extra byte after the mask size");
        VFilterParams accepted;
        if (!accepted.decode(data.data(), exactSize))
        {
            std::cout << "Exact size is not accepted, mask " << mask
                      << std::endl;
            result = false;
        }
    }

    return checker.passed() && result;
}



/// Damaged data: the decoder gives a deterministic result and a decoded
/// object is encoded to the same bytes.
bool damagedDataTest()
{
    int accepted = 0;
    for (int i = 0; i < DAMAGED_SAMPLES_NUMBER; ++i)
    {
        // Valid data with a random mask, then random byte replacements.
        VFilterParams source = randomParams();
        VFilterParamsMask mask = randomMask();
        std::array<uint8_t, BUFFER_SIZE> data;
        fillCanary(data);
        int size = 0;
        if (!source.encode(data.data(), BUFFER_SIZE, size, &mask))
        {
            return fail("Can't encode params");
        }
        const int replacements = randomInt(4);
        for (int k = 0; k < replacements; ++k)
        {
            data[static_cast<size_t>(randomInt(size))] =
                    static_cast<uint8_t>(randomInt(256));
        }
        // Random size: truncated, exact or longer than the data.
        const int dataSize = randomInt(PARAMS_MAX_SIZE + 4);

        // The result of two calls must be the same.
        VFilterParams first = randomParams();
        VFilterParams second = first;
        std::array<uint8_t, BUFFER_SIZE> copy1 = data;
        std::array<uint8_t, BUFFER_SIZE> copy2 = data;
        const bool ok1 = first.decode(copy1.data(), dataSize);
        const bool ok2 = second.decode(copy2.data(), dataSize);
        if (ok1 != ok2 || !sameParams(first, second) || copy1 != data ||
            copy2 != data)
        {
            return fail("Decoding is not deterministic or changed the data");
        }
        if (!ok1)
        {
            continue;
        }
        ++accepted;

        // Accepted data must be what the encoder produces for the decoded
        // object and the mask of the data.
        VFilterParamsMask dataMask;
        dataMask.mode = (data[3] & 0x80) != 0;
        dataMask.level = (data[3] & 0x40) != 0;
        dataMask.processingTimeMcSec = (data[3] & 0x20) != 0;
        dataMask.type = (data[3] & 0x10) != 0;
        dataMask.custom1 = (data[3] & 0x08) != 0;
        dataMask.custom2 = (data[3] & 0x04) != 0;
        dataMask.custom3 = (data[3] & 0x02) != 0;
        std::array<uint8_t, BUFFER_SIZE> encoded;
        fillCanary(encoded);
        int encodedSize = 0;
        if (!first.encode(encoded.data(), BUFFER_SIZE, encodedSize,
                          &dataMask) || encodedSize != dataSize ||
            std::memcmp(encoded.data(), data.data(),
                        static_cast<size_t>(dataSize)) != 0)
        {
            return fail("Accepted data is not canonical");
        }
    }
    std::cout << "Samples: " << DAMAGED_SAMPLES_NUMBER << ", accepted: "
              << accepted << std::endl;

    return true;
}



/// Encode and decode of the commands: every ID, random values.
bool commandsTest()
{
    // Set param command for every parameter ID.
    for (int id = static_cast<int>(VFilterParam::MODE);
         id <= static_cast<int>(VFilterParam::CUSTOM_3); ++id)
    {
        for (int k = 0; k < 100; ++k)
        {
            const float value = randomFloat();
            std::array<uint8_t, BUFFER_SIZE> data;
            fillCanary(data);
            int size = 0;
            VFilter::encodeSetParamCommand(
                        data.data(), size, static_cast<VFilterParam>(id),
                        value);
            if (size != SET_PARAM_COMMAND_SIZE ||
                !isCanary(data, static_cast<size_t>(size)))
            {
                return fail("Wrong size of the set param command");
            }
            VFilterParam paramId{VFilterParam::MODE};
            VFilterCommand commandId{VFilterCommand::OFF};
            float out = 5.0f;
            if (VFilter::decodeCommand(data.data(), size, paramId, commandId,
                                       out) != 1 ||
                static_cast<int>(paramId) != id || bitsOf(out) != bitsOf(value)
                || commandId != VFilterCommand::OFF)
            {
                return fail("Set param command is not decoded");
            }
        }
    }

    // Action command for every command ID.
    for (int id = static_cast<int>(VFilterCommand::RESET);
         id <= static_cast<int>(VFilterCommand::OFF); ++id)
    {
        std::array<uint8_t, BUFFER_SIZE> data;
        fillCanary(data);
        int size = 0;
        VFilter::encodeCommand(data.data(), size,
                               static_cast<VFilterCommand>(id));
        if (size != COMMAND_SIZE || !isCanary(data, static_cast<size_t>(size)))
        {
            return fail("Wrong size of the action command");
        }
        VFilterParam paramId{VFilterParam::TYPE};
        VFilterCommand commandId{VFilterCommand::RESET};
        float out = 5.0f;
        if (VFilter::decodeCommand(data.data(), size, paramId, commandId,
                                   out) != 0 ||
            static_cast<int>(commandId) != id ||
            paramId != VFilterParam::TYPE || bitsOf(out) != bitsOf(0.0f))
        {
            return fail("Action command is not decoded");
        }
    }

    // Encoders reject the null pointer: the size is set to 0.
    int size = 777;
    VFilter::encodeSetParamCommand(nullptr, size, VFilterParam::MODE, 1.0f);
    if (size != 0)
    {
        return fail("Set param encoder: size is not 0 for null pointer");
    }
    size = 777;
    VFilter::encodeCommand(nullptr, size, VFilterCommand::RESET);
    if (size != 0)
    {
        return fail("Command encoder: size is not 0 for null pointer");
    }

    return true;
}



/// Command decoder rejects invalid data with -1 and does not change the
/// output arguments.
bool commandsInvalidDataTest()
{
    bool result = true;
    VFilterParam paramId{VFilterParam::CUSTOM_2};
    VFilterCommand commandId{VFilterCommand::ON};
    float value = 3.5f;
    const auto rejected = [&](uint8_t* data, int size, const char* what)
    {
        const int code = VFilter::decodeCommand(
                    data, size, paramId, commandId, value);
        if (code != -1 || paramId != VFilterParam::CUSTOM_2 ||
            commandId != VFilterCommand::ON || bitsOf(value) != bitsOf(3.5f))
        {
            std::cout << "Not rejected or arguments changed: " << what
                      << " (size " << size << ", result " << code << ")"
                      << std::endl;
            result = false;
        }
    };

    std::array<uint8_t, BUFFER_SIZE> setParam;
    std::array<uint8_t, BUFFER_SIZE> action;
    setParam.fill(0);
    action.fill(0);
    int size = 0;
    VFilter::encodeSetParamCommand(setParam.data(), size, VFilterParam::LEVEL,
                                   1.0f);
    VFilter::encodeCommand(action.data(), size, VFilterCommand::ON);

    // Null pointer and every wrong size: only 7 and 11 bytes are valid.
    rejected(nullptr, SET_PARAM_COMMAND_SIZE, "null pointer");
    rejected(setParam.data(), std::numeric_limits<int>::min(), "INT_MIN size");
    rejected(setParam.data(), std::numeric_limits<int>::max(), "INT_MAX size");
    for (int s = -1; s <= 20; ++s)
    {
        if (s != SET_PARAM_COMMAND_SIZE)
        {
            rejected(setParam.data(), s, "set param command size");
        }
        if (s != COMMAND_SIZE)
        {
            rejected(action.data(), s, "action command size");
        }
    }

    // Every wrong value of the data type and of the version bytes.
    for (int pos = 0; pos < 3; ++pos)
    {
        for (int v = 0; v < 256; ++v)
        {
            if (v == setParam[static_cast<size_t>(pos)] ||
                (pos == 0 && v == action[0]))
            {
                continue;
            }
            std::array<uint8_t, BUFFER_SIZE> data = setParam;
            data[static_cast<size_t>(pos)] = static_cast<uint8_t>(v);
            rejected(data.data(), SET_PARAM_COMMAND_SIZE,
                     pos == 0 ? "wrong data type" : "wrong version");
            data = action;
            data[static_cast<size_t>(pos)] = static_cast<uint8_t>(v);
            rejected(data.data(), COMMAND_SIZE,
                     pos == 0 ? "wrong data type" : "wrong version");
        }
    }

    // IDs which are not in the VFilterParam enum (1 - 7) and in the
    // VFilterCommand enum (1 - 3).
    const std::array<int, 6> badParamIds{
        {std::numeric_limits<int>::min(), -1, 0, 8, 100,
         std::numeric_limits<int>::max()}};
    for (const int id : badParamIds)
    {
        std::array<uint8_t, BUFFER_SIZE> data = setParam;
        std::memcpy(&data[3], &id, sizeof(id));
        rejected(data.data(), SET_PARAM_COMMAND_SIZE, "unknown param ID");
    }
    const std::array<int, 7> badCommandIds{
        {std::numeric_limits<int>::min(), -1, 0, 4, 7, 100,
         std::numeric_limits<int>::max()}};
    for (const int id : badCommandIds)
    {
        std::array<uint8_t, BUFFER_SIZE> data = action;
        std::memcpy(&data[3], &id, sizeof(id));
        rejected(data.data(), COMMAND_SIZE, "unknown command ID");
    }

    return result;
}



/// The version string is equal to the version macros.
bool versionTest()
{
    if (VFilter::getVersion() != versionString())
    {
        return fail("Version string is not equal to the version macros");
    }
    if (VFilter::getVersion() != std::string(VFILTER_VERSION))
    {
        return fail("Version string is not equal to VFILTER_VERSION");
    }

    return true;
}



/// Read / write params from / to JSON.
bool readWriteJsonTest()
{
    // Params with random values (the processing time is not stored in JSON).
    VFilterParams params1 = randomParams();
    params1.level = static_cast<float>(randomInt(1000)) / 10.0f;
    params1.custom1 = static_cast<float>(randomInt(1000)) / 10.0f;
    params1.custom2 = static_cast<float>(randomInt(1000)) / 10.0f;
    params1.custom3 = static_cast<float>(randomInt(1000)) / 10.0f;

    // Save to JSON. The file is removed when the test is finished.
    const FileRemover remover(JSON_FILE_NAME);
    cr::utils::ConfigReader configReader1;
    if (!configReader1.set(params1, "Params"))
    {
        return fail("Can't set params");
    }
    if (!configReader1.writeToFile(JSON_FILE_NAME))
    {
        return fail("Can't write to file");
    }

    // Read params from file.
    cr::utils::ConfigReader configReader2;
    if (!configReader2.readFromFile(JSON_FILE_NAME))
    {
        return fail("Can't read from file");
    }
    VFilterParams params2;
    if (!configReader2.get(params2, "Params"))
    {
        return fail("Can't get params");
    }

    // Compare params (the processing time is not stored in JSON).
    VFilterParamsMask mask;
    mask.processingTimeMcSec = false;
    VFilterParams expected = params1;
    expected.processingTimeMcSec = 0;
    VFilterParams result = params2;
    result.processingTimeMcSec = 0;
    if (compareParams(expected, result, mask) != 0)
    {
        return false;
    }

    // A file which does not exist is not read.
    cr::utils::ConfigReader configReader3;
    if (configReader3.readFromFile("VFilterParamsNoSuchFile.json"))
    {
        return fail("A file which does not exist is read");
    }

    return true;
}



/// Invalid and incomplete JSON documents are rejected, the object is not
/// changed.
bool readInvalidJsonTest()
{
    const std::array<const char*, 14> documents{{
        "",
        "not a json",
        "{\"Params\":{\"mode\":1,\"level\":",
        "{\"Other\":{}}",
        "{\"Params\":5}",
        "{\"Params\":[1,2,3]}",
        "{\"Params\":{\"mode\":1,\"level\":1.0,\"custom1\":1.0,"
        "\"custom2\":2.0,\"custom3\":3.0}}",
        "{\"Params\":{\"mode\":\"1\",\"level\":1.0,\"type\":2,"
        "\"custom1\":1.0,\"custom2\":2.0,\"custom3\":3.0}}",
        "{\"Params\":{\"mode\":1,\"level\":\"x\",\"type\":2,"
        "\"custom1\":1.0,\"custom2\":2.0,\"custom3\":3.0}}",
        "{\"Params\":{\"mode\":null,\"level\":1.0,\"type\":2,"
        "\"custom1\":1.0,\"custom2\":2.0,\"custom3\":3.0}}",
        "{\"Params\":{\"mode\":1e30,\"level\":1.0,\"type\":2,"
        "\"custom1\":1.0,\"custom2\":2.0,\"custom3\":3.0}}",
        "{\"Params\":{\"mode\":4294967296,\"level\":1.0,\"type\":2,"
        "\"custom1\":1.0,\"custom2\":2.0,\"custom3\":3.0}}",
        "{\"Params\":{\"mode\":1,\"level\":true,\"type\":2,"
        "\"custom1\":1.0,\"custom2\":2.0,\"custom3\":3.0}}",
        "{\"Params\":{\"mode\":1,\"level\":1e400,\"type\":2,"
        "\"custom1\":1.0,\"custom2\":2.0,\"custom3\":3.0}}"}};

    VFilterParams object = randomParams();
    const VFilterParams before = object;
    bool result = true;
    for (const char* document : documents)
    {
        cr::utils::ConfigReader reader;
        if (reader.readFromString(document) && reader.get(object, "Params"))
        {
            std::cout << "Accepted: " << document << std::endl;
            result = false;
        }
        if (!sameParams(object, before))
        {
            std::cout << "Object changed by: " << document << std::endl;
            result = false;
            object = before;
        }
    }

    // Deeply nested document: rejected without a crash.
    const std::string deep(100000, '[');
    cr::utils::ConfigReader reader;
    if (reader.readFromString(deep) && reader.get(object, "Params"))
    {
        std::cout << "Accepted: deeply nested document" << std::endl;
        result = false;
    }

    return result;
}
}



int main()
{
    std::cout << "Test for VFilter library" << std::endl << std::endl;

    bool passed = true;
    passed = runTest("Copy parameters test", copyParametersTest) && passed;
    passed = runTest("Encode/decode params test without mask",
                     encodeDecodeTestWithoutMask) && passed;
    passed = runTest("Encode/decode params test with mask",
                     encodeDecodeTestWithMask) && passed;
    passed = runTest("Encode buffer test", encodeBufferTest) && passed;
    passed = runTest("Decode invalid data test", decodeInvalidDataTest) &&
             passed;
    passed = runTest("Decode mask test", decodeMaskTest) && passed;
    passed = runTest("Damaged data test", damagedDataTest) && passed;
    passed = runTest("Commands test", commandsTest) && passed;
    passed = runTest("Commands invalid data test", commandsInvalidDataTest) &&
             passed;
    passed = runTest("Version test", versionTest) && passed;
    passed = runTest("Read/Write JSON test", readWriteJsonTest) && passed;
    passed = runTest("Read invalid JSON test", readInvalidJsonTest) && passed;

    std::cout << (passed && !g_fileRemoveFailed ? "All tests passed"
                                                : "Tests failed")
              << std::endl;

    return (passed && !g_fileRemoveFailed) ? 0 : 1;
}
