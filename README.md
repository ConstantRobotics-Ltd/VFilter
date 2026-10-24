![vfilter_web_logo](./static/vfilter_web_logo.png)



# **VFilter C++ interface library**

**v1.1.6**



# Table of contents

- [Overview](#overview)
- [Versions](#versions)
- [Library files](#library-files)
- [VFilter interface class description](#vfilter-interface-class-description)
  - [VFilter interface class declaration](#vfilter-interface-class-declaration)
  - [getVersion method](#getversion-method)
  - [initVFilter method](#initvfilter-method)
  - [setParam method](#setparam-method)
  - [getParam method](#getparam-method)
  - [getParams method](#getparams-method)
  - [executeCommand method](#executecommand-method)
  - [processFrame method](#processframe-method)
  - [setMask method](#setmask-method)
  - [encodeSetParamCommand method](#encodesetparamcommand-method)
  - [encodeCommand method](#encodecommand-method)
  - [decodeCommand method](#decodecommand-method)
  - [decodeAndExecuteCommand method](#decodeandexecutecommand-method)
- [Data structures](#data-structures)
  - [VFilterCommand enum](#vfiltercommand-enum)
  - [VFilterParam enum](#vfilterparam-enum)
- [VFilterParams class description](#vfilterparams-class-description)
  - [VFilterParams class declaration](#vfilterparams-class-declaration)
  - [Serialize VFilter params](#serialize-vfilter-params)
  - [Deserialize VFilter params](#deserialize-vfilter-params)
  - [Read params from JSON file and write to JSON file](#read-params-from-json-file-and-write-to-json-file)
- [Build and connect to your project](#build-and-connect-to-your-project)
  - [Build stand alone](#build-stand-alone)
  - [CMake options](#cmake-options)
  - [Connect to your project](#connect-to-your-project)
- [Portability](#portability)
- [How to make custom implementation](#how-to-make-custom-implementation)



# Overview

**VFilter** C++ library provides interface as well defines data structures for various video filters implementations. The **VFilter** interface represents video filter module which has video frames as input and output. The **VFilter.h** file contains declaration of data structures: [VFilterCommand](#vfiltercommand-enum) enum, [VFilterParam](#vfilterparam-enum) enum, [VFilterParams](#vfilterparams-class-declaration) class and [VFilter](#vfilter-interface-class-declaration) class declaration. The library depends on [Frame](https://rapidpixel.constantrobotics.com/docs/Service/Frame.html) library (describes data structures for video frames) and [ConfigReader](https://rapidpixel.constantrobotics.com/docs/Service/ConfigReader.html) library (provides methods to read/write JSON config files). The repository does not contain these libraries: they must be cloned next to the **VFilter** repository (see [Build and connect to your project](#build-and-connect-to-your-project) section). The library uses C++17 standard, compatible with Linux and Windows and doesn't have third-party dependencies to be installed in OS.



# Versions

**Table 1** - Library versions.

| Version | Release date | What's new                                                   |
| ------- | ------------ | ------------------------------------------------------------ |
| 1.0.0   | 20.02.2024   | First version of the library.                                |
| 1.1.0   | 22.02.2024   | - Added setMask(...) method.<br />- Added initVFilter(...) to initialize video filter.<br />- Documentation updated. |
| 1.1.1   | 25.02.2024   | - Parameter processingTimeMcSec excluded from JSON to read / write. |
| 1.1.2   | 24.03.2024   | - Frame class updated.<br />- ConfigReader class updated.<br />- Documentation updated. |
| 1.1.3   | 21.05.2024   | - Submodules updated.<br />- Documentation updated.          |
| 1.1.4   | 13.07.2024   | - Submodules updated.<br />- CMake updated.                  |
| 1.1.5   | 28.09.2026   | - Submodules updated.<br />- Added build flags.              |
| 1.1.6   | 17.10.2026   | - Submodules removed: the Frame and ConfigReader repositories must be cloned next to the VFilter repository.<br />- decodeCommand(...) returns -1 for every error (a set param command of a wrong size was reported as an action command), checks the exact size of the command, the version, the data type and the ID range, and changes the output arguments only if the command is decoded.<br />- VFilterParams::decode(...) does not change the object if the data is rejected, rejects nullptr, data with extra bytes and data with the reserved bit of the mask set.<br />- VFilterParams::encode(...), encodeSetParamCommand(...) and encodeCommand(...) reject nullptr and set the output size to 0 on error.<br />- Encode and decode code is table driven, the encoded data is the same.<br />- Example: values from outside are validated, level is stored as float, read only parameter processingTimeMcSec can not be set, commands ON and OFF change the mode, all methods are thread-safe.<br />- Strict compiler warnings, exploit mitigation compiler and linker options, standard C++ only.<br />- Test application checks all methods and returns a non-zero exit code on errors.<br />- Documentation updated. |



# Library files

The library supplied by source code only. The user would be given a set of files in the form of a CMake project (repository). The repository structure is shown below:

```xml
CMakeLists.txt ----------------- Main CMake file of the library.
README.md ---------------------- This document.
static ------------------------- Folder with images of this document.
src ---------------------------- Folder with source code of the library.
    CMakeLists.txt ------------- CMake file of the library.
    VFilter.cpp ---------------- C++ implementation file.
    VFilter.h ------------------ Main header file of the library.
    VFilterVersion.h ----------- Header file which includes version of the library.
    VFilterVersion.h.in -------- Service CMake file to generate version file.
test --------------------------- Folder for the test application.
    CMakeLists.txt ------------- CMake file for the test application.
    main.cpp ------------------- Source code file of the test application.
example ------------------------ Folder with source code of the custom VFilter implementation.
    CMakeLists.txt ------------- CMake file of the library.
    CustomVFilter.cpp ---------- Source code file of the library.
    CustomVFilter.h ------------ Header file which includes CustomVFilter class declaration.
    CustomVFilterVersion.h ----- Header file which includes version of the library.
    CustomVFilterVersion.h.in -- CMake service file to generate version file.
```



# VFilter interface class description



## VFilter interface class declaration

The **VFilter** interface class declared in **VFilter.h** file. Class declaration:

```cpp
namespace rapidpixel
{
/// Video filter interface class.
class VFilter
{
public:

    /// Class destructor.
    virtual ~VFilter();

    /// Get the version of the VFilter class.
    static std::string getVersion();

    /// Initialize video filter.
    virtual bool initVFilter(VFilterParams& params) = 0;

    /// Set the value for a specific library parameter.
    virtual bool setParam(VFilterParam id, float value) = 0;

    /// Get the value of a specific library parameter.
    virtual float getParam(VFilterParam id) = 0;

    /// Get the structure containing all library parameters.
    virtual void getParams(VFilterParams& params) = 0;

    /// Execute a VFilter action command.
    virtual bool executeCommand(VFilterCommand id) = 0;

    /// Process frame.
    virtual bool processFrame(rapidpixel::Frame& frame) = 0;

    /// Set mask for filter.
    virtual bool setMask(rapidpixel::Frame mask) = 0;

    /// Encode set param command.
    static void encodeSetParamCommand(uint8_t* data, int& size,
                                      VFilterParam id, float value);

    /// Encode action command.
    static void encodeCommand(uint8_t* data, int& size, VFilterCommand id);

    /// Decode command.
    static int decodeCommand(uint8_t* data, int size, VFilterParam& paramId,
                             VFilterCommand& commandId, float& value);

    /// Decode and execute command.
    virtual bool decodeAndExecuteCommand(uint8_t* data, int size) = 0;

protected:

    /// Default constructor. The class can be used as a base only.
    VFilter() = default;

    /// Copy constructor.
    VFilter(const VFilter& src) = default;

    /// Move constructor.
    VFilter(VFilter&& src) = default;

    /// Copy assignment operator.
    VFilter& operator=(const VFilter& src) = default;

    /// Move assignment operator.
    VFilter& operator=(VFilter&& src) = default;
};
}
```



## getVersion method

The **getVersion()** static method returns string of current class version. Method declaration:

```cpp
static std::string getVersion();
```

Method can be used without **VFilter** class instance:

```cpp
std::cout << "VFilter version: " << rapidpixel::VFilter::getVersion();
```

Console output:

```bash
VFilter version: 1.1.6
```



## initVFilter method

The **initVFilter(...)** method initializes video filter by a set of parameters. Particular video filter class should initialize only supported parameters from [VFilterParams](#vfilterparams-class-description) class. The thread safety of the method is defined by the implementation. Method declaration:

```cpp
virtual bool initVFilter(VFilterParams& params) = 0;
```

| Parameter | Value                                                        |
| --------- | ------------------------------------------------------------ |
| params    | [VFilterParams](#vfilterparams-class-description) class object. Particular video filter class may not support all params listed in [VFilterParams](#vfilterparams-class-description) class. If video filter doesn't support particular params, those params should have default values. The values are not checked by **VFilterParams** class: the implementation must validate them. |

**Returns:** TRUE if the video filter initialized or FALSE if not.



## setParam method

The **setParam(...)** method sets the parameter value. **VFilter** based library should provide thread-safe **setParam(...)** method call. This means that the **setParam(...)** method can be safely called from any thread. Method declaration:

```cpp
virtual bool setParam(VFilterParam id, float value) = 0;
```

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| id        | Parameter ID according to [VFilterParam](#vfilterparam-enum) enum. |
| value     | Parameter value. Value depends on parameter ID.              |

**Returns:** TRUE if the parameter was set or FALSE if not.



## getParam method

The **getParam(...)** method returns parameter value. **VFilter** based library should provide thread-safe **getParam(...)** method call. This means that the **getParam(...)** method can be safely called from any thread. Method declaration:

```cpp
virtual float getParam(VFilterParam id) = 0;
```

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| id        | Parameter ID according to [VFilterParam](#vfilterparam-enum) enum. |

**Returns:** parameter value or **-1** if the parameters doesn't exist (not supported in particular implementation).



## getParams method

The **getParams(...)** method is designed to obtain all video filter params. **VFilter** based library should provide thread-safe **getParams(...)** method call. This means that the **getParams(...)** method can be safely called from any thread. Method declaration:

```cpp
virtual void getParams(VFilterParams& params) = 0;
```

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| params    | Reference to [VFilterParams](#vfilterparams-class-description) object to store params. |



## executeCommand method

The **executeCommand(...)** method executes video filter action command. **VFilter** based library should provide thread-safe **executeCommand(...)** method call. This means that the **executeCommand(...)** method can be safely called from any thread. Method declaration:

```cpp
virtual bool executeCommand(VFilterCommand id) = 0;
```

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| id        | Command  ID according to [VFilterCommand](#vfiltercommand-enum) enum. |

**Returns:** TRUE if the command executed or FALSE if not.



## processFrame method

The **processFrame(...)** method designed to process frame. **VFilter** based library should provide thread-safe **processFrame(...)** method call. This means that the **processFrame(...)** method can be safely called from any thread. Method declaration:

```cpp
virtual bool processFrame(rapidpixel::Frame& frame) = 0;
```

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| frame     | [Frame](https://rapidpixel.constantrobotics.com/docs/Service/Frame.html) object for processing (input and result frame). Particular video filter implementation must keep original video frame resolution (width and height), **sourceId** and **frameId** fields. |

**Returns:** TRUE if frame processed or FALSE if not. If filter disabled the method should return TRUE without video frame processing.



## setMask method

The **setMask(...)** method designed to set video filter mask. The thread safety of the method is defined by the implementation. Method declaration:

```c++
virtual bool setMask(rapidpixel::Frame mask) = 0;
```

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| mask      | Filter mask is [Frame](https://rapidpixel.constantrobotics.com/docs/Service/Frame.html) object with **GRAY**, **NV12**, **NV21**, **YV12** or **YU12** pixel format. Filter omits image segments, where filter mask pixel values equal 0. |

**Returns:** TRUE if the filter mask was set or FALSE if not.



## encodeSetParamCommand method

The **encodeSetParamCommand(...)** static method encodes command to change any **VFilter** parameter value remote. To control any video filter remotely, the developer has to design his own protocol and according to it encode the command and deliver it over the communication channel. To simplify this, the **VFilter** class contains static methods for encoding the control command. The **VFilter** class provides two types of commands: a parameter change command (SET_PARAM) and an action command (COMMAND). **encodeSetParamCommand(...)** designed to encode SET_PARAM command. Method declaration:

```cpp
static void encodeSetParamCommand(uint8_t* data, int& size, VFilterParam id, float value);
```

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| data      | Pointer to data buffer for encoded command. Must not be nullptr. Must have size >= 11 (the method can not check the size of the buffer). |
| size      | Size of encoded data. Size will be 11 bytes. Size will be 0 (nothing is written) if **data** is nullptr. |
| id        | Parameter ID according to [VFilterParam](#vfilterparam-enum) enum. The ID is not checked by the method: [decodeCommand(...)](#decodecommand-method) rejects an ID which is not in the enum. |
| value     | Parameter value. The value is not checked: its meaning belongs to the video filter implementation. |

The encoded command has 11 bytes: data type (1 byte, 0x01), major and minor version of the library (1 byte each), parameter ID (4 bytes) and parameter value (4 bytes). Integer and float values are stored in the byte order of the host. **encodeSetParamCommand(...)** is static, thread-safe and can be used without **VFilter** class instance. This method used on client side (control system). Command encoding example:

```cpp
// Buffer for encoded data.
uint8_t data[11];
// Size of encoded data.
int size = 0;
// Parameter value.
float outValue = 20.0f;
// Encode command.
rapidpixel::VFilter::encodeSetParamCommand(
    data, size, rapidpixel::VFilterParam::LEVEL, outValue);
```



## encodeCommand method

The **encodeCommand(...)** static method encodes action command for **VFilter** remote control. To control any video filter remotely, the developer has to design his own protocol and according to it encode the command and deliver it over the communication channel. To simplify this, the **VFilter** class contains static methods for encoding the control command. The **VFilter** class provides two types of commands: a parameter change command (SET_PARAM) and an action command (COMMAND). **encodeCommand(...)** designed to encode COMMAND (action command). Method declaration:

```cpp
static void encodeCommand(uint8_t* data, int& size, VFilterCommand id);
```

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| data      | Pointer to data buffer for encoded command. Must not be nullptr. Must have size >= 7 (the method can not check the size of the buffer). |
| size      | Size of encoded data. Size will be 7 bytes. Size will be 0 (nothing is written) if **data** is nullptr. |
| id        | Command ID according to [VFilterCommand](#vfiltercommand-enum) enum. The ID is not checked by the method: [decodeCommand(...)](#decodecommand-method) rejects an ID which is not in the enum. |

The encoded command has 7 bytes: data type (1 byte, 0x00), major and minor version of the library (1 byte each) and command ID (4 bytes, byte order of the host). **encodeCommand(...)** is static, thread-safe and can be used without **VFilter** class instance. This method used on client side (control system). Command encoding example:

```cpp
// Buffer for encoded data.
uint8_t data[7];
// Size of encoded data.
int size = 0;
// Encode command.
rapidpixel::VFilter::encodeCommand(
    data, size, rapidpixel::VFilterCommand::RESET);
```



## decodeCommand method

The **decodeCommand(...)** static method decodes command on video filter side (on edge device). The method is thread-safe. Method declaration:

```cpp
static int decodeCommand(uint8_t* data, int size, VFilterParam& paramId, VFilterCommand& commandId, float& value);
```

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| data      | Pointer to input command. Must not be nullptr. The buffer must be valid for **size** bytes. |
| size      | Size of command. Must be exactly 11 bytes for SET_PARAM and exactly 7 bytes for COMMAND (action command). |
| paramId   | VFilter parameter ID according to [VFilterParam](#vfilterparam-enum) enum. After decoding SET_PARAM command the method will return parameter ID. |
| commandId | VFilter command ID according to [VFilterCommand](#vfiltercommand-enum) enum. After decoding COMMAND the method will return command ID. |
| value     | VFilter parameter value (after decoding SET_PARAM command). The value is 0 after decoding COMMAND. |

**Returns:** **0** - in case decoding COMMAND (action command), **1** - in case decoding SET_PARAM command or **-1** in case errors: **data** is nullptr, wrong size, wrong version (major or minor), wrong data type or ID which is not in [VFilterCommand](#vfiltercommand-enum) enum (for COMMAND) or [VFilterParam](#vfilterparam-enum) enum (for SET_PARAM). The output arguments are changed only if the command is decoded.



## decodeAndExecuteCommand method

The **decodeAndExecuteCommand(...)** method decodes and executes command encoded by [encodeSetParamCommand(...)](#encodesetparamcommand-method) and [encodeCommand(...)](#encodecommand-method) methods on video filter side (on edge device). The particular implementation of the VFilter must provide thread-safe **decodeAndExecuteCommand(...)** method call. This means that the **decodeAndExecuteCommand(...)** method can be safely called from any thread. The implementation should use [decodeCommand(...)](#decodecommand-method) to decode the command. Method declaration:

```cpp
virtual bool decodeAndExecuteCommand(uint8_t* data, int size) = 0;
```

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| data      | Pointer to input command.                                    |
| size      | Size of command. Must be 11 bytes for SET_PARAM or 7 bytes for COMMAND. |

**Returns:** TRUE if command decoded (SET_PARAM or COMMAND) and executed (action command or set param command) or FALSE if the command is not valid or not executed.



# Data structures



## VFilterCommand enum

Enum declaration:

```cpp
enum class VFilterCommand
{
    /// Reset video filter algorithm.
    RESET = 1,
    /// Enable filter.
    ON,
    /// Disable filter.
    OFF
};
```

**Table 2** - Action commands description.

| Command | Description                   |
| ------- | ----------------------------- |
| RESET   | Reset video filter algorithm. |
| ON      | Enable video filter.          |
| OFF     | Disable video filter.         |



## VFilterParam enum

Enum declaration:

```cpp
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
```

**Table 3** - Params description.

| Parameter             | Access       | Description                                                  |
| --------------------- | ------------ | ------------------------------------------------------------ |
| MODE                  | read / write | Filter mode, usually 0 - off (disabled), 1 - on (enabled). Can have another meaning depends on implementation. |
| LEVEL                 | read / write | Enhancement level for particular filter, as a percentage in range from 0% to 100%. Can have another meaning depends on implementation. The video filter should keep this value in memory. After enable video filter this value should be implemented. |
| PROCESSING_TIME_MCSEC | read only    | Processing time, microseconds. Read only parameter. Used to control performance of video filter. |
| TYPE                  | read / write | Type of the filter. Depends on the implementation.           |
| CUSTOM_1              | read / write | VFilter custom parameter. Custom parameters used when particular video filter has specific unusual parameter. |
| CUSTOM_2              | read / write | VFilter custom parameter. Custom parameters used when particular video filter has specific unusual parameter. |
| CUSTOM_3              | read / write | VFilter custom parameter. Custom parameters used when particular video filter has specific unusual parameter. |



# VFilterParams class description



## VFilterParams class declaration

The **VFilterParams** class is used to provide video filter parameters structure. Also **VFilterParams** provides possibility to write/read params from JSON files (**JSON_READABLE** macro) and provides methods to encode and decode params. **VFilterParams** class declared in **VFilter.h** file. The values of the fields are not checked against any range by the class (JSON, serialization): the video filter implementation must validate them before use. The class is not thread-safe: an object must not be accessed from several threads at the same time without external synchronization. Class declaration:

```cpp
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

    /// Encode (serialize) params.
    bool encode(uint8_t* data, int bufferSize, int& size,
                VFilterParamsMask* mask = nullptr);

    /// Decode (deserialize) params.
    bool decode(uint8_t* data, int dataSize);
};
```

**Table 4** - **VFilterParams** class fields description which is related to [VFilterParam enum](#vfilterparam-enum) description.

| Field               | type  | Description                                            |
| ------------------- | ----- | ------------------------------------------------------ |
| mode                | int   | Filter mode, usually 0 - off (disabled), 1 - on (enabled). Can have another meaning depends on implementation. |
| level               | float | Enhancement level for particular filter, as a percentage in range from 0% to 100%. Can have another meaning depends on implementation. The video filter should keep this value in memory. After enable video filter this value should be implemented. |
| processingTimeMcSec | int   | Processing time, microseconds. Read only parameter. Used to control performance of video filter. |
| type                | int   | Type of the filter. Depends on the implementation.     |
| custom1             | float | VFilter custom parameter. Custom parameters used when particular video filter has specific unusual parameter. |
| custom2             | float | VFilter custom parameter. Custom parameters used when particular video filter has specific unusual parameter. |
| custom3             | float | VFilter custom parameter. Custom parameters used when particular video filter has specific unusual parameter. |

**Note:** *VFilterParams class fields listed in Table 4 **have to** reflect params set/get by methods setParam(...) and getParam(...).*



## Serialize VFilter params

[VFilterParams](#vfilterparams-class-description) class provides method **encode(...)** to serialize VFilter params. Serialization of **VFilterParams** is necessary in case when video filter parameters have to be sent via communication channels. Method provides options to exclude particular parameters from serialization. To do this method inserts binary mask (1 byte) where each bit represents particular parameter and **decode(...)** method recognizes it. The encoded data has a header of 4 bytes: data type (0x02), major and minor version of the library (1 byte each) and the mask (1 byte). The header is followed by 4 bytes for each serialized parameter (int and float values in the byte order of the host) in the order: mode, level, processingTimeMcSec, type, custom1, custom2, custom3. The most significant bit of the mask belongs to mode, the least significant bit (0x01) is not used and is always 0. Method declaration:

```cpp
bool encode(uint8_t* data, int bufferSize, int& size, VFilterParamsMask* mask = nullptr);
```

| Parameter  | Value                                                        |
| ---------- | ------------------------------------------------------------ |
| data       | Pointer to data buffer. Must not be nullptr. Buffer size must be >= 32 bytes. |
| bufferSize | Data buffer size. Buffer size must be >= 32 bytes (size of the encoded params with all fields). |
| size       | Size of encoded data. Size will be 0 if the method fails.    |
| mask       | Parameters mask - pointer to **VFilterParamsMask** structure. **VFilterParamsMask** (declared in **VFilter.h** file) determines flags for each field (parameter) declared in [VFilterParams class](#vfilterparams-class-description). If user wants to exclude any parameters from serialization, he can put a pointer to the mask. If the user wants to exclude a particular parameter from serialization, he should set the corresponding flag to false in the **VFilterParamsMask** structure. If the pointer is nullptr all parameters are serialized. |

**Returns:** TRUE if params encoded (serialized) or FALSE if not (**data** is nullptr or buffer size < 32). If FALSE is returned nothing is written to the buffer.

**VFilterParamsMask** structure declaration:

```cpp
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
```

Example without parameters mask:

```cpp
// Prepare parameters.
rapidpixel::VFilterParams params;
params.level = 80.0f;

// Encode (serialize) params.
int bufferSize = 128;
uint8_t buffer[128];
int size = 0;
if (!params.encode(buffer, bufferSize, size))
{
    std::cout << "Can't encode params" << std::endl;
}
```

Example with parameters mask:

```cpp
// Prepare parameters.
rapidpixel::VFilterParams params;
params.level = 80.0f;

// Prepare mask.
rapidpixel::VFilterParamsMask mask;
// Exclude level.
mask.level = false;

// Encode (serialize) params.
int bufferSize = 128;
uint8_t buffer[128];
int size = 0;
if (!params.encode(buffer, bufferSize, size, &mask))
{
    std::cout << "Can't encode params" << std::endl;
}
```



## Deserialize VFilter params

[VFilterParams](#vfilterparams-class-description) class provides method **decode(...)** to deserialize params. Deserialization of **VFilterParams** is necessary in case when it is needed to receive params via communication channels. Method automatically recognizes which parameters were serialized by **encode(...)** method. The method checks the data type, the version (major and minor) and the size of the data: the size must be equal to the size defined by the mask in the data (4 bytes of the header and 4 bytes for each parameter selected by the mask). Parameters which are not in the data are set to 0. Values of the parameters are not checked against any range. Method declaration:

```cpp
bool decode(uint8_t* data, int dataSize);
```

| Parameter | Value                                          |
| --------- | ---------------------------------------------- |
| data      | Pointer to data buffer with serialized params. Must not be nullptr. The buffer must be valid for **dataSize** bytes. |
| dataSize  | Size of serialized params, bytes.              |

**Returns:** TRUE if params decoded (deserialized) or FALSE if not (**data** is nullptr, wrong data type, version or size, or the unused bit of the mask is set). If FALSE is returned the object is not changed.

Example:

```cpp
// Prepare parameters.
rapidpixel::VFilterParams params1;
params1.level = 80.0f;

// Encode (serialize) params.
int bufferSize = 128;
uint8_t buffer[128];
int size = 0;
if (!params1.encode(buffer, bufferSize, size))
{
    std::cout << "Can't encode params" << std::endl;
}

// Decode (deserialize) params.
rapidpixel::VFilterParams params2;
if (!params2.decode(buffer, size))
{
    std::cout << "Can't decode params" << std::endl;
}
```



## Read params from JSON file and write to JSON file

**VFilter** depends on open source [ConfigReader](https://rapidpixel.constantrobotics.com/docs/Service/ConfigReader.html) library which provides method to read params from JSON file and to write params to JSON file. The **readFromFile(...)** method returns false if the file does not exist or is not valid JSON. The **get(...)** method returns false and does not change the object if a parameter is missing or a parameter has a wrong type or a value out of range of the field (for example a string instead of a number). A real number in JSON is truncated if the field is int. Example of writing and reading params to JSON file:

```cpp
// Prepare params.
rapidpixel::VFilterParams params1;
params1.mode = 1;
params1.level = 10.5f;
params1.processingTimeMcSec = 10;
params1.type = 2;
params1.custom1 = 22.5f;
params1.custom2 = 23.25f;
params1.custom3 = 24.5f;

// Save to JSON.
rapidpixel::ConfigReader configReader1;
if (!configReader1.set(params1, "Params"))
{
    std::cout << "Can't set params" << std::endl;
}
if (!configReader1.writeToFile("VFilterParams.json"))
{
    std::cout << "Can't write to file" << std::endl;
}

// Read params from file.
rapidpixel::ConfigReader configReader2;
if (!configReader2.readFromFile("VFilterParams.json"))
{
    std::cout << "Can't read from file" << std::endl;
}
rapidpixel::VFilterParams params2;
if (!configReader2.get(params2, "Params"))
{
    std::cout << "Can't get params" << std::endl;
}
```

**VFilterParams.json** will look like:

```json
{
    "Params": {
        "mode": 1,
        "level": 10.5,
        "type": 2,
        "custom1": 22.5,
        "custom2": 23.25,
        "custom3": 24.5
    }
}
```



# Build and connect to your project



## Build stand alone

The repository does not contain sub-repositories. The libraries **VFilter** depends on (**Frame** and **ConfigReader**) must be cloned into the same folder as **VFilter**. The folder structure must be:

```bash
<work folder>
    VFilter
    Frame
    ConfigReader
```

Typical commands to build the library, the test application and the example (use the URLs of the repositories provided to you):

```bash
git clone <VFilter repository URL>
git clone <Frame repository URL>
git clone <ConfigReader repository URL>
cd VFilter
mkdir build
cd build
cmake ..
make
```

On Windows use `cmake --build . --config Release` instead of `make`. If **VFilter** is the top-level CMake project, the main CMakeLists.txt file adds the folders ../Frame and ../ConfigReader (in this order) before the library itself. If a folder does not exist CMake stops with an error.



## CMake options

**Table 5** - CMake options of the repository. In the options below ${PARENT} is the prefix defined by the top-level project (empty if **VFilter** is the top-level project).

| Option                              | Default                                           | Description                                                  |
| ----------------------------------- | ------------------------------------------------- | ------------------------------------------------------------ |
| ${PARENT}_VFILTER                   | ON                                                | Build the library.                                           |
| ${PARENT}_VFILTER_TEST              | ON (stand alone), OFF (included in other project) | Build the test application.                                  |
| ${PARENT}_VFILTER_EXAMPLE           | ON (stand alone), OFF (included in other project) | Build the example of the custom VFilter implementation.      |
| ${PARENT}_SUBMODULE_CACHE_OVERWRITE | ON                                                | Forced rewriting of the options above in the CMake cache. The top-level project sets it to OFF when it includes **VFilter** as a sub-repository. |
| CMAKE_BUILD_TYPE                    | Release (if not specified)                        | Build type.                                                  |



## Connect to your project

If you want to connect **VFilter** to your CMake project as source code you can make follow. For example, if your repository has structure:

```bash
CMakeLists.txt
src
    CMakeList.txt
    yourLib.h
    yourLib.cpp
```

Clone (or copy) the **VFilter**, **Frame** and **ConfigReader** repositories to **3rdparty** folder of your repository. Structure of your repository:

```bash
CMakeLists.txt
src
    CMakeList.txt
    yourLib.h
    yourLib.cpp
3rdparty
    VFilter
    Frame
    ConfigReader
```

**VFilter** does not add its dependencies when it is included in another project: the project must add **Frame** and **ConfigReader** by itself (before or after **VFilter**). If they are not added, the build stops with a compiler error like **ConfigReader.h: No such file or directory**.

Create CMakeLists.txt file in **3rdparty** folder. CMakeLists.txt should contain:

```cmake
cmake_minimum_required(VERSION 3.13)

################################################################################
## 3RD-PARTY
## dependencies for the project
################################################################################
project(3rdparty LANGUAGES CXX)

################################################################################
## SETTINGS
## basic 3rd-party settings before use
################################################################################
# To inherit the top-level architecture when the project is used as a submodule.
SET(PARENT ${PARENT}_YOUR_PROJECT_3RDPARTY)
# Disable self-overwriting of parameters inside included subdirectories.
SET(${PARENT}_SUBMODULE_CACHE_OVERWRITE OFF CACHE BOOL "" FORCE)

################################################################################
## CONFIGURATION
## 3rd-party submodules configuration
################################################################################
SET(${PARENT}_SUBMODULE_FRAME                           ON  CACHE BOOL "" FORCE)
if (${PARENT}_SUBMODULE_FRAME)
    SET(${PARENT}_FRAME                                 ON  CACHE BOOL "" FORCE)
    SET(${PARENT}_FRAME_TEST                            OFF CACHE BOOL "" FORCE)
endif()

SET(${PARENT}_SUBMODULE_CONFIG_READER                   ON  CACHE BOOL "" FORCE)
if (${PARENT}_SUBMODULE_CONFIG_READER)
    SET(${PARENT}_CONFIG_READER                         ON  CACHE BOOL "" FORCE)
    SET(${PARENT}_CONFIG_READER_EXAMPLES                OFF CACHE BOOL "" FORCE)
    SET(${PARENT}_CONFIG_READER_TESTS                   OFF CACHE BOOL "" FORCE)
endif()

SET(${PARENT}_SUBMODULE_VFILTER                         ON  CACHE BOOL "" FORCE)
if (${PARENT}_SUBMODULE_VFILTER)
    SET(${PARENT}_VFILTER                               ON  CACHE BOOL "" FORCE)
    SET(${PARENT}_VFILTER_TEST                          OFF CACHE BOOL "" FORCE)
    SET(${PARENT}_VFILTER_EXAMPLE                       OFF CACHE BOOL "" FORCE)
endif()

################################################################################
## INCLUDING SUBDIRECTORIES
## Adding subdirectories according to the 3rd-party configuration
################################################################################
if (${PARENT}_SUBMODULE_FRAME)
    add_subdirectory(Frame)
endif()

if (${PARENT}_SUBMODULE_CONFIG_READER)
    add_subdirectory(ConfigReader)
endif()

if (${PARENT}_SUBMODULE_VFILTER)
    add_subdirectory(VFilter)
endif()
```

File **3rdparty/CMakeLists.txt** adds folders **Frame**, **ConfigReader** and **VFilter** to your project and excludes test applications and examples (**VFilter** class test application and example of custom **VFilter** class implementation) from compiling (by default example and test application excluded from compiling if **VFilter** included as sub-repository). The structure of your repository will be:

```bash
CMakeLists.txt
src
    CMakeList.txt
    yourLib.h
    yourLib.cpp
3rdparty
    CMakeLists.txt
    VFilter
    Frame
    ConfigReader
```

Next you need include folder 3rdparty in main **CMakeLists.txt** file of your repository. Add string at the end of your main **CMakeLists.txt**:

```cmake
add_subdirectory(3rdparty)
```

Next you have to include **VFilter** library in your **src/CMakeLists.txt** file:

```cmake
target_link_libraries(${PROJECT_NAME} VFilter)
```

Done!



# Portability

The serialized data (encoded params and commands) stores int and float values as 4 bytes values in the byte order of the host. The sizes of int and float are checked at compile time (the build stops if they are not 4 bytes). The library is built and tested only on little-endian platforms: data serialized on a platform with another byte order is not compatible. The library is built and tested with MSVC 2022 (x86-64), GCC 12 and 13 (x86-64), Clang 15 and 18 (x86-64), GCC 13 and Clang 18 (aarch64, run under emulation). The library does not use SIMD instructions and has no platform specific code.



# How to make custom implementation

The **VFilter** class provides only an interface, data structures, and methods for encoding and decoding commands and params. To create your own implementation of the video filter, VFilter repository has to be included in your project (see [Build and connect to your project](#build-and-connect-to-your-project) section). The catalogue **example** (see [Library files](#library-files) section) includes an example of the design of the custom VFilter algorithm. All the methods of the VFilter interface class have to be included. Custom VFilter class declaration:

```cpp
namespace rapidpixel
{
/**
 * @brief Example of a custom video filter implementation.
 *
 * The class implements all methods of the VFilter interface. The values which
 * come from outside (parameters, mask) are validated before use. All methods
 * are protected by one mutex, so the methods can be called from any thread.
 */
class CustomVFilter : public rapidpixel::VFilter
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
    bool processFrame(rapidpixel::Frame& frame) override;

    /**
     * @brief Set filter mask. Filter omits image segments, where
     * filter mask pixel values equal 0.
     * @param mask Filter binary mask. Must have data and one of the pixel
     * formats: GRAY, NV12, NV21, YV12 or YU12.
     * @return TRUE if video filter mask was set or FALSE if not (the mask is
     * not valid or the memory can not be allocated).
     */
    bool setMask(rapidpixel::Frame mask) override;

    /**
     * @brief Decode and execute command.
     * @param data Pointer to command data.
     * @param size Size of data.
     * @return TRUE if command decoded and executed or FALSE if not.
     */
    bool decodeAndExecuteCommand(uint8_t* data, int size) override;

private:

    /// Parameters structure (default params). Protected by m_paramsMutex.
    rapidpixel::VFilterParams m_params;
    /// Mask for filter. Protected by m_paramsMutex.
    rapidpixel::Frame m_mask;
    /// Mutex to protect m_params and m_mask.
    std::mutex m_paramsMutex;
};
}
```