#ifndef SRC_GATEWAY_HPP_
#define SRC_GATEWAY_HPP_

#include "CanController.hpp"
#include "GattServer.hpp"
#include "CarParam.hpp"
#include "Timerfd.hpp"
#include "Signalfd.hpp"
#include <pthread.h>


#include <cstdint>
#include "CarParam.hpp"
#include "ObstacleDetector.hpp"

namespace acm
{

class Gateway
{
public:

	enum CanId
		: uint16_t
		{
			CanId_UltrasoundData = 1,
		CanId_DirectionCmd = 2,
		CanId_SpeedCmd = 3,
		CanId_DirectionData = 4,
		CanId_SpeedData = 5,
		CanId_BatteryData = 6
	};

	enum BleUuid
		: uint16_t
		{
			BleUuid_AcmService = 0x7db9, BleUuid_AcmCharState = 0xd288,	// Write without resp
		BleUuid_AcmCharFeedb = 0xc15b,	// Read and Notify
		BleUuid_AcmCharAlert = 0xdcb1 	// Read and Notify
	};

	static constexpr unsigned int CAN_WRITE_PERIOD_MS = 25;

	Gateway() = default;
	~Gateway() = default;

	void Ble_advertise();

	int run();

private:

	///////////////////////////////////////////////////////////////////////////
	//// Callbacks
	///////////////////////////////////////////////////////////////////////////
	static void signalCallback(int signum, void *user_data);
	static void Can_onTimeToSend(void* user_data);
	static void Can_onDataReceived(int fd, uint32_t events, void *user_data);
	static void Ble_onTimeToSend(void* user_data);
	static void Ble_onDataReceived(struct gatt_db_attribute *attrib,
			unsigned int id, uint16_t offset, const uint8_t *value, size_t len,
			uint8_t opcode, struct bt_att *att, void *user_data);
	static void * AutonomousControl(void * arg) ; 

	Timerfd timer;
	Signalfd signal;

	CanController m_canController;
	GattServer m_gattServer;
	CarParamOut m_carParamOut;
	CarParamIn m_carParamIn ; 
	ObstacleDetector m_obstacleDetector;

	pthread_t autonomousThread ; 
};

} // namespace acm

#endif /* SRC_GATEWAY_HPP_ */
