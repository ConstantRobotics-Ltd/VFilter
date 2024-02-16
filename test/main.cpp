#include <iostream>
#include "PanTilt.h"



/**
 * @brief Copy params test.
 */
bool copyParametersTest();

/**
 * @brief Encode/decode params test without mask.
 */
bool encodeDecodeTestWithoutMask();

/**
 * @brief Encode/decode params test with mask.
 */
bool encodeDecodeTestWithMask();



int main(void)
{
	std::cout << "Test for PanTilt library" << std::endl << std::endl;

	std::cout << "Copy parameters test:" << std::endl;
	if (copyParametersTest())
	{
		std::cout << "OK" << std::endl;
	}
	else
	{
		std::cout << "ERROR" << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Encode/decode params test without mask:" << std::endl;
	if (encodeDecodeTestWithoutMask())
	{
		std::cout << "OK" << std::endl;
	}
	else
	{
		std::cout << "ERROR" << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Encode/decode params test with mask:" << std::endl;
	if (encodeDecodeTestWithMask())
	{
		std::cout << "OK" << std::endl;
	}
	else
	{
		std::cout << "ERROR" << std::endl;
	}
	std::cout << std::endl;

	return 1;
}



bool copyParametersTest()
{
	// Prepare random params.
	cr::pantilt::PanTiltParams params1;
	params1.panAngle = static_cast<float>(rand() % 255);
	params1.tiltAngle = static_cast<float>(rand() % 255);
	params1.panMotorPosition = rand() % 65535;
	params1.tiltMotorPosition = rand() % 65535;
	params1.panMotorSpeed = static_cast<float>(rand() % 255);
	params1.tiltMotorSpeed = static_cast<float>(rand() % 255);
	params1.isConnected = true;
	params1.isInitialized = true;
	params1.custom1 = static_cast<float>(rand() % 255);
	params1.custom2 = static_cast<float>(rand() % 255);
	params1.custom3 = static_cast<float>(rand() % 255);

	// Copy params.
	cr::pantilt::PanTiltParams params2 = params1;

	// Compare params.
	bool result = true;
	if (params1.panAngle != params2.panAngle)
	{
		std::cout << "[" << __LINE__ << "] " << "panAngle not equal" << std::endl;
		result = false;
	}
	if (params1.tiltAngle != params2.tiltAngle)
	{
		std::cout << "[" << __LINE__ << "] " << "tiltAngle not equal" << std::endl;
		result = false;
	}
	if (params1.panMotorPosition != params2.panMotorPosition)
	{
		std::cout << "[" << __LINE__ << "] " << "panMotorPosition not equal" << std::endl;
		result = false;
	}
	if (params1.tiltMotorPosition != params2.tiltMotorPosition)
	{
		std::cout << "[" << __LINE__ << "] " << "tiltMotorPosition not equal" << std::endl;
		result = false;
	}
	if (params1.panMotorSpeed != params2.panMotorSpeed)
	{
		std::cout << "[" << __LINE__ << "] " << "panMotorSpeed not equal" << std::endl;
		result = false;
	}
	if (params1.tiltMotorSpeed != params2.tiltMotorSpeed)
	{
		std::cout << "[" << __LINE__ << "] " << "tiltMotorSpeed not equal" << std::endl;
		result = false;
	}
	if (params1.isConnected != params2.isConnected)
	{
		std::cout << "[" << __LINE__ << "] " << "isConnected not equal" << std::endl;
		result = false;
	}
	if (params1.isInitialized != params2.isInitialized)
	{
		std::cout << "[" << __LINE__ << "] " << "isInitialized not equal" << std::endl;
		result = false;
	}
	if (params1.custom1 != params2.custom1)
	{
		std::cout << "[" << __LINE__ << "] " << "custom1 not equal" << std::endl;
		result = false;
	}
	if (params1.custom2 != params2.custom2)
	{
		std::cout << "[" << __LINE__ << "] " << "custom2 not equal" << std::endl;
		result = false;
	}
	if (params1.custom3 != params2.custom3)
	{
		std::cout << "[" << __LINE__ << "] " << "custom3 not equal" << std::endl;
		result = false;
	}

	return result;
}



bool encodeDecodeTestWithoutMask()
{
	// Prepare random params.
	cr::pantilt::PanTiltParams params1;
	params1.panAngle = static_cast<float>(rand() % 255);
	params1.tiltAngle = static_cast<float>(rand() % 255);
	params1.panMotorPosition = rand() % 65535;
	params1.tiltMotorPosition = rand() % 65535;
	params1.panMotorSpeed = static_cast<float>(rand() % 255);
	params1.tiltMotorSpeed = static_cast<float>(rand() % 255);
	params1.isConnected = true;
	params1.isInitialized = true;
	params1.custom1 = static_cast<float>(rand() % 255);
	params1.custom2 = static_cast<float>(rand() % 255);
	params1.custom3 = static_cast<float>(rand() % 255);

	// Encode (serialize) params.
	int bufferSize = 128;
	uint8_t buffer[128];
	int size = 0;
	if (!params1.encode(buffer, bufferSize, size))
	{
		std::cout << "[" << __LINE__ << "] " << "Can't encode params" << std::endl;
		return false;
	}

	// Decode (deserialize) params.
	cr::pantilt::PanTiltParams params2;
	if (!params2.decode(buffer, size))
	{
		std::cout << "[" << __LINE__ << "] " << "Can't decode params" << std::endl;
		return false;
	}

	// Compare params.
	bool result = true;
	if (params1.panAngle != params2.panAngle)
	{
		std::cout << "[" << __LINE__ << "] " << "panAngle not equal" << std::endl;
		result = false;
	}
	if (params1.tiltAngle != params2.tiltAngle)
	{
		std::cout << "[" << __LINE__ << "] " << "tiltAngle not equal" << std::endl;
		result = false;
	}
	if (params1.panMotorPosition != params2.panMotorPosition)
	{
		std::cout << "[" << __LINE__ << "] " << "panMotorPosition not equal" << std::endl;
		result = false;
	}
	if (params1.tiltMotorPosition != params2.tiltMotorPosition)
	{
		std::cout << "[" << __LINE__ << "] " << "tiltMotorPosition not equal" << std::endl;
		result = false;
	}
	if (params1.panMotorSpeed != params2.panMotorSpeed)
	{
		std::cout << "[" << __LINE__ << "] " << "panMotorSpeed not equal" << std::endl;
		result = false;
	}
	if (params1.tiltMotorSpeed != params2.tiltMotorSpeed)
	{
		std::cout << "[" << __LINE__ << "] " << "tiltMotorSpeed not equal" << std::endl;
		result = false;
	}
	if (params1.isConnected != params2.isConnected)
	{
		std::cout << "[" << __LINE__ << "] " << "isConnected not equal" << std::endl;
		result = false;
	}
	if (params1.isInitialized != params2.isInitialized)
	{
		std::cout << "[" << __LINE__ << "] " << "isInitialized not equal" << std::endl;
		result = false;
	}
	if (params1.custom1 != params2.custom1)
	{
		std::cout << "[" << __LINE__ << "] " << "custom1 not equal" << std::endl;
		result = false;
	}
	if (params1.custom2 != params2.custom2)
	{
		std::cout << "[" << __LINE__ << "] " << "custom2 not equal" << std::endl;
		result = false;
	}
	if (params1.custom3 != params2.custom3)
	{
		std::cout << "[" << __LINE__ << "] " << "custom3 not equal" << std::endl;
		result = false;
	}


	return result;
}



bool encodeDecodeTestWithMask()
{
	// Prepare random params.
	cr::pantilt::PanTiltParams params1;
	params1.panAngle = static_cast<float>(rand() % 255);
	params1.tiltAngle = static_cast<float>(rand() % 255);
	params1.panMotorPosition = rand() % 65535;
	params1.tiltMotorPosition = rand() % 65535;
	params1.panMotorSpeed = static_cast<float>(rand() % 255);
	params1.tiltMotorSpeed = static_cast<float>(rand() % 255);
	params1.isConnected = true;
	params1.isInitialized = true;
	params1.custom1 = static_cast<float>(rand() % 255);
	params1.custom2 = static_cast<float>(rand() % 255);
	params1.custom3 = static_cast<float>(rand() % 255);

	// Prepare mask.
	cr::pantilt::PanTiltParamsMask mask;
	mask.panAngle = false;
	mask.tiltAngle = true;
	mask.panMotorPosition = false;
	mask.tiltMotorPosition = true;
	mask.panMotorSpeed = false;
	mask.tiltMotorSpeed = true;
	mask.isConnected = false;
	mask.isInitialized = true;
	mask.custom1 = false;
	mask.custom2 = true;
	mask.custom3 = false;

	// Encode (serialize) params.
	int bufferSize = 128;
	uint8_t buffer[128];
	int size = 0;
	if (!params1.encode(buffer, bufferSize, size, &mask))
	{
		std::cout << "[" << __LINE__ << "] " << "Can't encode params" << std::endl;
		return false;
	}

	// Decode (deserialize) params.
	cr::pantilt::PanTiltParams params2;
	if (!params2.decode(buffer, size))
	{
		std::cout << "[" << __LINE__ << "] " << "Can't decode params" << std::endl;
		return false;
	}

	// Compare params.
	bool result = true;
	if (params2.panAngle != 0.0f)
	{
		std::cout << "[" << __LINE__ << "] " << "panAngle not equal" << std::endl;
		result = false;
	}
	if (params2.tiltAngle != params1.tiltAngle)
	{
		std::cout << "[" << __LINE__ << "] " << "tiltAngle not equal" << std::endl;
		result = false;
	}
	if (params2.panMotorPosition != 0)
	{
		std::cout << "[" << __LINE__ << "] " << "panMotorPosition not equal" << std::endl;
		result = false;
	}
	if (params2.tiltMotorPosition != params1.tiltMotorPosition)
	{
		std::cout << "[" << __LINE__ << "] " << "tiltMotorPosition not equal" << std::endl;
		result = false;
	}
	if (params2.panMotorSpeed != 0.0f)
	{
		std::cout << "[" << __LINE__ << "] " << "panMotorSpeed not equal" << std::endl;
		result = false;
	}
	if (params2.tiltMotorSpeed != params1.tiltMotorSpeed)
	{
		std::cout << "[" << __LINE__ << "] " << "tiltMotorSpeed not equal" << std::endl;
		result = false;
	}
	if (params2.isConnected != false)
	{
		std::cout << "[" << __LINE__ << "] " << "isConnected not equal" << std::endl;
		result = false;
	}
	if (params2.isInitialized != params1.isInitialized)
	{
		std::cout << "[" << __LINE__ << "] " << "isInitialized not equal" << std::endl;
		result = false;
	}
	if (params2.custom1 != 0.0f)
	{
		std::cout << "[" << __LINE__ << "] " << "custom1 not equal" << std::endl;
		result = false;
	}
	if (params2.custom2 != params1.custom2)
	{
		std::cout << "[" << __LINE__ << "] " << "custom2 not equal" << std::endl;
		result = false;
	}
	if (params2.custom3 != 0.0f)
	{
		std::cout << "[" << __LINE__ << "] " << "custom3 not equal" << std::endl;
		result = false;
	}

	return result;
}