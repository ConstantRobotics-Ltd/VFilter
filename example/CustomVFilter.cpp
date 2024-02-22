#include "CustomVFilter.h"
#include "CustomVFilterVersion.h"



cr::video::CustomVFilter::CustomVFilter()
{

}



cr::video::CustomVFilter::~CustomVFilter()
{

}



std::string cr::video::CustomVFilter::getVersion()
{
	return CUSTOM_VFILTER_VERSION;
}



bool cr::video::CustomVFilter::setParam(VFilterParam id, float value)
{
	// Lock mutex for setting parameters.
	std::lock_guard<std::mutex>lock(m_paramsMutex);
	// Check param ID.
	switch (id)
	{
	case VFilterParam::MODE:
	{
		m_params.mode = static_cast<int>(value);
		return true;
	}
	case VFilterParam::LEVEL:
	{
		m_params.level = static_cast<int>(value);
		return true;
	}
	case VFilterParam::PROCESSING_TIME_MCSEC:
	{
		m_params.processingTimeMcSec = static_cast<int>(value);
		return true;
	}
	case VFilterParam::TYPE:
	{
		m_params.type = static_cast<int>(value);
	return true;
	}
	case VFilterParam::CUSTOM_1:
	{
		// Custom parameter.
		m_params.custom1 = value;
		return true;
	}
	case VFilterParam::CUSTOM_2:
	{
		// Custom parameter.
		m_params.custom2 = value;
		return true;
	}
	case VFilterParam::CUSTOM_3:
	{
		// Custom parameter.
		m_params.custom3 = value;
		return true;
	}
	}

	return false;
}



float cr::video::CustomVFilter::getParam(VFilterParam id)
{
	// Lock mutex for reading parameters.
	std::lock_guard<std::mutex>lock(m_paramsMutex);
	// Check param ID.
	switch (id)
	{
	case VFilterParam::MODE:
	{
		return static_cast<float>(m_params.mode);
	}
	case VFilterParam::LEVEL:
	{
		return static_cast<float>(m_params.level);
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

	return 0.0f;
}



void cr::video::CustomVFilter::getParams(VFilterParams& params)
{
	// Lock mutex for assigning parameters.
	std::lock_guard<std::mutex>lock(m_paramsMutex);
	params = m_params;
}



bool cr::video::CustomVFilter::executeCommand(VFilterCommand id)
{
	switch (id)
	{
	case VFilterCommand::RESTART:
	{
		return true;
	}
	case VFilterCommand::ON:
	{
		return true;
	}
	case VFilterCommand::OFF:
	{
		return true;
	}
	}
	return false;
}



bool cr::video::CustomVFilter::processFrame(cr::video::Frame &frame)
{
	// some processing
    return false;
}



bool cr::video::CustomVFilter::setMask(cr::video::Frame mask)
{
	std::lock_guard<std::mutex>lock(m_paramsMutex);
	m_mask = mask;
    return true;
}



bool cr::video::CustomVFilter::decodeAndExecuteCommand(uint8_t *data, int size)
{
	// Decode command.
	VFilterCommand commandId = VFilterCommand::RESTART;
	VFilterParam paramId = VFilterParam::LEVEL;
	float value = 0.0f;
	switch (VFilter::decodeCommand(data, size, paramId, commandId, value))
	{
		// COMMAND.
	case 0:
		// Execute command.
		return executeCommand(commandId);
		// SET_PARAM.
	case 1:
	{
		// Set param.
		return setParam(paramId, value);
	}
	default:
	{
		return false;
	}

	}
	return false;
}