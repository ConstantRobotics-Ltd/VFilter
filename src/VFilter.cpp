#include "VFilter.h"
#include "VFilterVersion.h"
#include <cstring>



cr::video::VFilterParams &cr::video::VFilterParams::operator= (const VFilterParams& src)
{
	// Check yourself.
	if (this == &src)
	{
		return *this;
	}
	
	// Copy params.
	mode = src.mode;
	level = src.level;
	processingTimeMcSec = src.processingTimeMcSec;
	type = src.type;
	custom1 = src.custom1;
	custom2 = src.custom2;
	custom3 = src.custom3;

	return *this;
}



bool cr::video::VFilterParams::encode(uint8_t* data, int bufferSize, int& size,
	VFilterParamsMask* mask)
{
	// Check buffer size.
	if (bufferSize < 32)
		return false;

	// Copy atributes.
	data[0] = 0x02;
	data[1] = VFILTER_MAJOR_VERSION;
	data[2] = VFILTER_MINOR_VERSION;

	// Prepare params mask.
	VFilterParamsMask paramsMask;
	if (mask != nullptr)
		paramsMask = *mask;

	// Fill bit mask for params.
	int pos = 3;
	data[pos] = 0x00;
	data[pos] = data[pos] | (paramsMask.mode ? (uint8_t)128 : (uint8_t)0);
	data[pos] = data[pos] | (paramsMask.level ? (uint8_t)64 : (uint8_t)0);
	data[pos] = data[pos] | (paramsMask.processingTimeMcSec ? (uint8_t)32 : (uint8_t)0);
	data[pos] = data[pos] | (paramsMask.type ? (uint8_t)16 : (uint8_t)0);
    data[pos] = data[pos] | (paramsMask.custom1 ? (uint8_t)8 : (uint8_t)0);
    data[pos] = data[pos] | (paramsMask.custom2 ? (uint8_t)4 : (uint8_t)0);
    data[pos] = data[pos] | (paramsMask.custom3 ? (uint8_t)2 : (uint8_t)0);
    pos += 1;

	// Copy params to buffer.
	if (paramsMask.mode)
	{
		memcpy(&data[pos], &mode, 4);
		pos += 4;
	}
	if (paramsMask.level)
	{
		memcpy(&data[pos], &level, 4);
		pos += 4;
	}
	if (paramsMask.processingTimeMcSec)
	{
		memcpy(&data[pos], &processingTimeMcSec, 4);
		pos += 4;
	}
	if (paramsMask.type)
	{
		memcpy(&data[pos], &type, 4);
		pos += 4;
	}
	if (paramsMask.custom1)
	{
		memcpy(&data[pos], &custom1, 4);
		pos += 4;
	}
	if (paramsMask.custom2)
	{
		memcpy(&data[pos], &custom2, 4);
		pos += 4;
		
	}
	if (paramsMask.custom3)
	{
		memcpy(&data[pos], &custom3, 4);
		pos += 4;
	}
	
	size = pos;

	return true;
}



bool cr::video::VFilterParams::decode(uint8_t* data, int dataSize)
{
	// Check data size.
	if (dataSize < 4)
		return false;

	// Check atributes.
	if (data[0] != 0x02 ||
		data[1] != VFILTER_MAJOR_VERSION ||
		data[2] != VFILTER_MINOR_VERSION)
		return false;

	// Decode params.
	int pos = 4;
   	if ((data[3] & (uint8_t)128) == (uint8_t)128)
	{
		if (dataSize < pos + 4)
			return false;
		memcpy(&mode, &data[pos], 4);
		pos += 4;
	}
	else
	{
		mode = 0;
	}
	if ((data[3] & (uint8_t)64) == (uint8_t)64)
	{
		if (dataSize < pos + 4)
			return false;
		memcpy(&level, &data[pos], 4);
		pos += 4;
	}
	else
	{
		level = 0;
	}
	if ((data[3] & (uint8_t)32) == (uint8_t)32)
	{
		if (dataSize < pos + 4)
			return false;
		memcpy(&processingTimeMcSec, &data[pos], 4);
		pos += 4;
	}
	else
	{
		processingTimeMcSec = 0;
	}
	if ((data[3] & (uint8_t)16) == (uint8_t)16)
	{
		if (dataSize < pos + 4)
			return false;
		memcpy(&type, &data[pos], 4);
		pos += 4;
	}
	else
	{
		type = 0;
	}
	if ((data[3] & (uint8_t)8) == (uint8_t)8)
	{
		if (dataSize < pos + 4)
			return false;
		memcpy(&custom1, &data[pos], 4);
		pos += 4;
	}
	else
	{
		custom1 = 0.0f;
	}
	if ((data[3] & (uint8_t)4) == (uint8_t)4)
	{
		if (dataSize < pos + 4)
			return false;
		memcpy(&custom2, &data[pos], 4);
		pos += 4;
	}
	else
	{
		custom2 = 0.0f;
	}
	if ((data[3] & (uint8_t)2) == (uint8_t)2)
	{
		if (dataSize < pos + 4)
			return false;
		memcpy(&custom3, &data[pos], 4);
		pos += 4;
	}
	else
	{
		custom3 = 0.0f;
	}

	return true;
}



cr::video::VFilter::~VFilter()
{

}



std::string cr::video::VFilter::getVersion()
{
	return VFILTER_VERSION;
}



void cr::video::VFilter::encodeSetParamCommand(
	uint8_t* data, int& size, VFilterParam id, float value)
{
	// Fill header.
	data[0] = 0x01;
	data[1] = VFILTER_MAJOR_VERSION;
	data[2] = VFILTER_MINOR_VERSION;

	// Fill data.
	int paramId = (int)id;
	memcpy(&data[3], &paramId, 4);
	memcpy(&data[7], &value, 4);
	size = 11;
}



void cr::video::VFilter::encodeCommand(uint8_t* data, int& size,
	VFilterCommand id)
{
	// Fill header.
	data[0] = 0x00;
	data[1] = VFILTER_MAJOR_VERSION;
	data[2] = VFILTER_MINOR_VERSION;

	// Fill data.
	int commandId = (int)id;
	memcpy(&data[3], &commandId, 4);
	size = 7;
}



int cr::video::VFilter::decodeCommand(uint8_t* data, int size, VFilterParam& paramId,
	VFilterCommand& commandId, float& value)
{
	// Check size.
	if (size < 7)
	{
		return -1;
	}

	// Check version.
	if (data[1] != VFILTER_MAJOR_VERSION || data[2] != VFILTER_MINOR_VERSION)
	{
		return -1;
	}

	// Extract data.
	int id = 0;
	memcpy(&id, &data[3], 4);
	value = 0.0f;

	// Check command type.
	if (data[0] == 0x00)
	{
		commandId = static_cast<VFilterCommand> (id);
		return 0;
	}
	else if (data[0] == 0x01)
	{
		// Check size.
		if (size != 11)
		{
			return false;
		}

		paramId = static_cast<VFilterParam>(id);
		memcpy(&value, &data[7], 4);
		return 1;
	}

	return -1;
}