#ifndef SRC_GATEWAY_HPP_
#define SRC_GATEWAY_HPP_

#include <cstdint>

#include <pthread.h>

#include "mainloop/Timerfd.hpp"
#include "mainloop/Signalfd.hpp"
#include "can/CanController.hpp"
#include "gatt/GattServer.hpp"
#include "CarParam.hpp"
#include "ObstacleDetector.hpp"

namespace acm
{

class Application
{
public:

	enum CanId : uint16_t
	{
		CanId_UltrasoundData 	= 1,
		CanId_DirectionCmd 		= 2,
		CanId_SpeedCmd 			= 3,
		CanId_DirectionData 	= 4,
		CanId_SpeedData 		= 5,
		CanId_BatteryData 		= 6
	};

	enum BleUuid : uint16_t
	{
		BleUuid_AcmService 		= 0x7db9,
		BleUuid_AcmCharState 	= 0xd288,	// Write without resp
		BleUuid_AcmCharFeedb 	= 0xc15b,	// Read and Notify
		BleUuid_AcmCharAlert 	= 0xdcb1 	// Read and Notify
	};

	static constexpr unsigned int CAN_WRITE_PERIOD_MS 		= 25;
	static constexpr unsigned int AUTO_PROCESS_PERIOD_MS 	= 200; // TODO: choose wisely =)

	Application() = default;
	~Application() = default;

	void bleAdvertise();

	int run();

private:

	///////////////////////////////////////////////////////////////////////////
	//// Callbacks
	///////////////////////////////////////////////////////////////////////////
	void signalCallback(int signum);
	void AutonomousControl();
	void canOnTimeToSend();
	void canOnDataReceived(int fd, uint32_t events);
	void bleOnTimeToSend(void* user_data);
	void bleOnDataReceived(struct gatt_db_attribute *attrib,
			unsigned int id, uint16_t offset, const uint8_t *value, size_t len,
			uint8_t opcode, struct bt_att *att);

	Timerfd timerCanSend;
	Timerfd timerAutonomousProcess;
	Signalfd signal;

	CanController m_canController;
	GattServer m_gattServer;
	CarParamOut m_carParamOut;
	CarParamIn m_carParamIn ;
	ObstacleDetector m_obstacleDetector;

	pthread_t m_autonomousThread ;
};

} // namespace acm

#endif /* SRC_GATEWAY_HPP_ */
