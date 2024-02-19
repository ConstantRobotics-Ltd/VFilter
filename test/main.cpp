#include <iostream>
#include "VFilter.h"



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
	std::cout << "Test for VFilter library" << std::endl << std::endl;

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
	cr::video::VFilterParams params1;
	params1.mode = rand() % 255;
	params1.level = rand() % 255;
	params1.processingTimeMcSec = rand() % 255;
	params1.type = rand() % 255;
	params1.custom1 = static_cast<float>(rand() % 255);
	params1.custom2 = static_cast<float>(rand() % 255);
	params1.custom3 = static_cast<float>(rand() % 255);

	// Copy params.
	cr::video::VFilterParams params2 = params1;

	// Compare params.
	bool result = true;
	if (params1.mode != params2.mode)
	{
		std::cout << "[" << __LINE__ << "] " << "mode not equal" << std::endl;
		result = false;
	}
	if (params1.level != params2.level)
	{
		std::cout << "[" << __LINE__ << "] " << "level not equal" << std::endl;
		result = false;
	}
	if (params1.processingTimeMcSec != params2.processingTimeMcSec)
	{
		std::cout << "[" << __LINE__ << "] " << "processingTimeMcSec not equal" << std::endl;
		result = false;
	}
	if (params1.type != params2.type)
	{
		std::cout << "[" << __LINE__ << "] " << "type not equal" << std::endl;
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
	cr::video::VFilterParams params1;
	params1.mode = rand() % 255;
	params1.level = rand() % 255;
	params1.processingTimeMcSec = rand() % 255;
	params1.type = rand() % 255;
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
	cr::video::VFilterParams params2;
	if (!params2.decode(buffer, size))
	{
		std::cout << "[" << __LINE__ << "] " << "Can't decode params" << std::endl;
		return false;
	}

	// Compare params.
	bool result = true;
	if (params1.mode != params2.mode)
	{
		std::cout << "[" << __LINE__ << "] " << "mode not equal" << std::endl;
		result = false;
	}
	if (params1.level != params2.level)
	{
		std::cout << "[" << __LINE__ << "] " << "level not equal" << std::endl;
		result = false;
	}
	if (params1.processingTimeMcSec != params2.processingTimeMcSec)
	{
		std::cout << "[" << __LINE__ << "] " << "processingTimeMcSec not equal" << std::endl;
		result = false;
	}
	if (params1.type != params2.type)
	{
		std::cout << "[" << __LINE__ << "] " << "type not equal" << std::endl;
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
	cr::video::VFilterParams params1;
	params1.level = rand() % 255;
	params1.mode = rand() % 255;
	params1.processingTimeMcSec = rand() % 255;
	params1.type = rand() % 255;
	params1.custom1 = static_cast<float>(rand() % 255);
	params1.custom2 = static_cast<float>(rand() % 255);
	params1.custom3 = static_cast<float>(rand() % 255);

	// Prepare mask.
	cr::video::VFilterParamsMask mask;
	mask.mode = true;
	mask.level = false;
	mask.processingTimeMcSec = true;
	mask.type = false;
	mask.custom1 = true;
	mask.custom2 = false;
	mask.custom3 = true;

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
	cr::video::VFilterParams params2;
	if (!params2.decode(buffer, size))
	{
		std::cout << "[" << __LINE__ << "] " << "Can't decode params" << std::endl;
		return false;
	}

	// Compare params.
	bool result = true;
	if (params2.mode != params1.mode)
	{
		std::cout << "[" << __LINE__ << "] " << "mode not equal" << std::endl;
		result = false;
	}
	if (params2.level != 0)
	{
		std::cout << "[" << __LINE__ << "] " << "level not equal" << std::endl;
		result = false;
	}
	if (params2.processingTimeMcSec != params1.processingTimeMcSec)
	{
		std::cout << "[" << __LINE__ << "] " << "processingTimeMcSec not equal" << std::endl;
		result = false;
	}
	if (params2.type != 0)
	{
		std::cout << "[" << __LINE__ << "] " << "type not equal" << std::endl;
		result = false;
	}
	if (params2.custom1 != params1.custom1)
	{
		std::cout << "[" << __LINE__ << "] " << "custom1 not equal" << std::endl;
		result = false;
	}
	if (params2.custom2 != 0.0f)
	{
		std::cout << "[" << __LINE__ << "] " << "custom2 not equal" << std::endl;
		result = false;
	}
	if (params2.custom3 != params1.custom3)
	{
		std::cout << "[" << __LINE__ << "] " << "custom3 not equal" << std::endl;
		result = false;
	}

	return result;
}