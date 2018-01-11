/*
 * Gateway.cpp
 *
 *  Created on: Dec 10, 2017
 *      Author: jucom
 */

#include "AcmApplication.hpp"
#include "common-defs.h"

namespace acm
{

void Application::bleAdvertise()
{
	int hci0_id = hci_devid("hci0");
	if (hci0_id < 0)
	{
		fprintf(stderr, "hci0 : no such device\n");
		exit(EXIT_FAILURE);
	}

	int hci0_dd = hci_open_dev(hci0_id);
	if (hci0_dd < 0)
	{
		fprintf(stderr, "Unable to open hci0 device");
		exit(EXIT_FAILURE);
	}

	struct hci_filter flt;
	hci_filter_clear(&flt);
	hci_filter_set_ptype(HCI_EVENT_PKT, &flt);
	hci_filter_all_events(&flt);
	if (setsockopt(hci0_dd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0)
	{
		perror("HCI filter setup failed");
		exit(EXIT_FAILURE);
	}

	int len = 32;
	uint16_t ocf = 0x0008;
	uint8_t ogf = 0x08;
	unsigned char buf[HCI_MAX_EVENT_SIZE] =
	{ 0x1E, 0x02, 0x01, 0x1A, 0x1A, 0xFF, 0x4C, 0x00, 0x02, 0x15, 0xE2, 0x0A,
			0x39, 0xF4, 0x73, 0xF5, 0x4B, 0xC4, 0xA1, 0x2F, 0x17, 0xD1, 0xAD,
			0x07, 0xA9, 0x61, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x00 };
	if (hci_send_cmd(hci0_dd, ogf, ocf, len, buf) < 0)
	{
		perror("Send failed");
		exit(EXIT_FAILURE);
	}

	len = read(hci0_dd, buf, sizeof(buf));
	if (len < 0)
	{
		perror("Read failed");
		exit(EXIT_FAILURE);
	}

	if (hci_le_set_advertise_enable(hci0_dd, 1, 0) != 0)
	{
		fprintf(stderr, "Failed to start le advertising\n");
		exit(EXIT_FAILURE);
	}

	hci_close_dev(hci0_dd);
}

static struct gatt_db_attribute * feedb = nullptr;
static uint16_t handle;

int Application::run()
{
	/*
	 * Initialize main event loop
	 */
	mainloop_init();

	/*
	 * Enable le advertising
	 */
	bleAdvertise();

	/*
	 * Open CAN controller on "can0" interface
	 */
	m_canController.open("can0");

	/*
	 * Open Gatt server with name "Acm-gateway"
	 */
	m_gattServer.open("Acm-gateway");

	/*
	 * Initialize GATT server's service and characteristics
	 */
	struct gatt_db_attribute *acm_service = m_gattServer.add_service(BleUuid_AcmService);

	m_gattServer.add_characteristic(acm_service, BleUuid_AcmCharState,
	BT_ATT_PERM_WRITE, BT_GATT_CHRC_PROP_WRITE_WITHOUT_RESP, NULL,
			[](struct gatt_db_attribute *attrib, unsigned int id, uint16_t offset, const uint8_t *value,
			   size_t len, uint8_t opcode, struct bt_att *att, void *user_data)
			{
				acm::Application* self = (decltype(self))(user_data);
				self->bleOnDataReceived(attrib,id,offset,value,len,opcode,att);
			},
			this);

	feedb = m_gattServer.add_characteristic(acm_service, BleUuid_AcmCharFeedb,
	BT_ATT_PERM_READ, BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_NOTIFY,
	NULL, NULL, &m_gattServer);

	handle = gatt_db_attribute_get_handle(feedb);

	bt_uuid_t uuid;
	bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);
	//gatt_db_service_add_descriptor(acm_service, &uuid, BT_ATT_PERM_READ, gatt_svc_chngd_ccc_read_cb, nullptr, this);
	gatt_db_service_add_descriptor(acm_service, &uuid, BT_ATT_PERM_READ, nullptr, nullptr, this); // TODO : test !

	m_gattServer.set_service_active(acm_service, true);

	/*
	 * Initialize CAN controller
	 */
	m_canController.registerMessageType(CanId_DirectionCmd, 2);
	m_canController.registerMessageType(CanId_SpeedCmd, 2);
	// TODO: add other type of messages
	m_canController.mainloopAttachRead(
			[](int fd, uint32_t events, void *user_data)
			{
				acm::Application* self = (decltype(self))(user_data);
				self->canOnDataReceived(fd, events);
			},
			this);

	timer.setDuration(1);
	timer.mainloopAttach(
			[](void *user_data)
			{
				static unsigned int canTimer = 0;
				static unsigned int autoTimer = 0;

				acm::Application* self = (decltype(self))(user_data);

				canTimer++;
				autoTimer++;

				if(canTimer >= CAN_WRITE_PERIOD_MS)
				{
					canTimer = 0;
					self->canOnTimeToSend();
				}
				if(autoTimer >= AUTO_PROCESS_PERIOD_MS)
				{
					autoTimer = 0;
					self->autonomousControl();
				}
			},
			this);

	/*
	 * Initialize timer to periodically write data on CAN
	 */
	/*timerCanSend.setDuration(CAN_WRITE_PERIOD_MS);
	timerCanSend.mainloopAttach(
			[](void *user_data)
			{
				printf("can send callback\n");
				acm::Application* self = (decltype(self))(user_data);
				self->canOnTimeToSend();
			},
			this);*/

	/*
	 * Initialize timer to periodically process autonomous state machine
	 */
	/*timerAutonomousProcess.setDuration(AUTO_PROCESS_PERIOD_MS);
	timerAutonomousProcess.mainloopAttach(
			[](void *user_data)
			{
				printf("auto process callback\n");
				acm::Application* self = (decltype(self))(user_data);
				self->autonomousControl();
			},
			this);*/

	/*
	 * Initialize signal to handle SIGINT and SIGTERM
	 */
	signal.add(SIGINT);
	signal.add(SIGTERM);
	signal.mainloopAttach(
			[](int signum, void *user_data)
			{
				acm::Application* self = (decltype(self))(user_data);
				self->signalCallback(signum);
			},
			this);

	/*
	 * Run main event loop
	 */
	return mainloop_run();
}

void Application::signalCallback(int signum)
{
	switch (signum)
	{
	case SIGINT:
	case SIGTERM:
		mainloop_quit();
		break;
	default:
		break;
	}
}

void Application::autonomousControl()
{
	static int stop_prev = 0; // avoid changing value every iteration

	obstacle obstacles[6];

	//get speed of the car
	m_carParamIn.mutex.lock() ;
		int car_speed = m_carParamIn.speed ;
	m_carParamIn.mutex.unlock() ;

	//get obstacles values
	m_carParamIn.mutex.lock();
		memcpy(obstacles, m_carParamIn.obstacles, sizeof(obstacles));
	m_carParamIn.mutex.unlock();

	int stop = 0;
	// check for each ultrasound if there is an obstacle
	for (int it = 0; it < 6; it++)
	{
		// distance TBD //static and speed= 1.5 //mobile and speed= 3
		if(obstacles[it].detected == 1)
		{
			if((obstacles[it].dist <= 40 && car_speed <= 2) ||
			   (obstacles[it].dist <= 60 && car_speed > 2))
			{
				stop = 1;
				break;
			}
		}
	}

	// update the parameter which will block the car
	if(stop != stop_prev)
	{
		m_carParamOut.mutex.lock();
			m_carParamOut.autonomous_locked = stop;
		m_carParamOut.mutex.unlock();
	}

	stop_prev = stop;
}

void Application::canOnTimeToSend()
{
	static int i = 0;

	//Multiply the period by 2
	if (i == 0)
	{
		m_carParamOut.mutex.lock();
			int dir = m_carParamOut.dir;
			int mode = m_carParamOut.mode;
		m_carParamOut.mutex.unlock();

		if(mode == ACM_MODE_AUTONOMOUS)
			dir=2 ;

		m_canController.sendMessage(CanId_DirectionCmd, (uint8_t*)&dir);

		i = 1;
	}
	else if (i == 1)
	{
		m_carParamOut.mutex.lock();
			int is_moving = m_carParamOut.moving;
			int is_turbo = m_carParamOut.turbo;
			int auto_locked = m_carParamOut.autonomous_locked ;
			int mode = m_carParamOut.mode;
		m_carParamOut.mutex.unlock();

		uint16_t speed = 0;
		if (is_moving && !auto_locked)
		{
			if (mode == ACM_MODE_MANUAL)
				speed = is_turbo ? 2 : 1;
			else
				speed=1;
		}

		m_canController.sendMessage(CanId_SpeedCmd, (uint8_t*)&speed);

		i = 0;
	}
}

void Application::canOnDataReceived(int fd, uint32_t events)
{
	int nbytes;
	struct can_frame frame;

	// TODO: use CanController::recv()
	nbytes = read(m_canController.fd(), &frame, sizeof(struct can_frame));

	if (nbytes < 0)
	{
		perror("ERROR can raw socket read ");
		exit(1);
	}

	if (nbytes < (decltype(nbytes)) sizeof(struct can_frame))
	{
		fprintf(stderr, "read : incomplete can frame\n");
		exit(1);
	}

	uint8_t obst_detection ;
	uint8_t speed ;
	uint8_t dir ;
	uint8_t bat ;
	uint8_t mode ;
	uint8_t road_detect ;
	obstacle data_obst[6];

	if (frame.can_id == CanId_UltrasoundData)
	{
		uint8_t us[6];
		obstacle obst[6];
		memcpy(us, frame.data, sizeof(us));

		m_obstacleDetector.detect(us, obst);

		obst_detection = (((obst[0].detected or obst[1].detected) ? 0x01 : 0x00) << 2) |
						 (((obst[2].detected or obst[3].detected) ? 0x01 : 0x00) << 1) |
						 (((obst[4].detected or obst[5].detected) ? 0x01 : 0x00) << 0);

		m_carParamIn.mutex.lock();
			m_carParamIn.obst = obst_detection ;
			memcpy(m_carParamIn.obstacles, obst, sizeof(m_carParamIn.obstacles));
		m_carParamIn.mutex.unlock();

		//printf("RECEIVED: %d\n", m_carParamIn.obstacles[1].dist);
	}
	if (frame.can_id == CanId_SpeedData)
	{
		speed = frame.data[0] / 10; // speed conversion from cm/s to dm/s

		m_carParamIn.mutex.lock();
			m_carParamIn.speed = speed;
		m_carParamIn.mutex.unlock();
	}
	if (frame.can_id == CanId_DirectionData)
	{
		m_carParamIn.mutex.lock();
			m_carParamIn.dir = m_carParamIn.speed == 0 ? 3 : frame.data[0];
		m_carParamIn.mutex.unlock();
	}
	if (frame.can_id == CanId_BatteryData)
	{
		bat = frame.data[0] ;

		m_carParamIn.mutex.lock();
			m_carParamIn.bat = bat;
		m_carParamIn.mutex.unlock();
	}

	uint8_t buf[2] = {0x00, 0x00};

	m_carParamIn.mutex.lock();
		obst_detection = m_carParamIn.obst;
		speed = m_carParamIn.speed;
		dir = m_carParamIn.dir;
		bat = m_carParamIn.bat;
		road_detect = m_carParamIn.road_detection;
		memcpy(data_obst, self->m_carParamIn.obstacles,sizeof(self->m_carParamIn.obstacles));
		
	m_carParamIn.mutex.unlock();

	buf[0] = ((speed & 0x07) << 3) | ((dir & 0x03) << 1) | ((0x00 & 0x00) << 0);
	buf[1] = ((0x00) << 4) | ((obst_detection) << 2) | ((bat & 0x03) << 0);

	// TODO: GattServer::sendNotification
	bt_gatt_server_send_notification(m_gattServer.m_gatt_server, handle, buf, sizeof(buf));
	
	m_carParamOut.mutex.lock();
		mode = m_carParamOut.mode;
	m_carParamOut.mutex.unlock();
	
	csv.generate_csv(m_carParamOut, m_carParamIn);
	
	/*
	speed = speed * 0.36; // Speed conversion from dm/s to km/h
	  
	//////////////         CSV GENRATION         /////////////
	fprintf(self->f, "%d;%d;%d;%d;", mode, speed, dir, road_detect);
	for (int i = 0 ; i < 6 ; i++)
	{
		fprintf(self->f, "%d;%d;%d;",data_obst[i].detected,data_obst[i].mobile,data_obst[i].dist);
	}	
	fprintf(self->f,"\n");
	*/
}

void Application::bleOnTimeToSend(void* user_data)
{
	UNUSED(user_data);
}

void Application::bleOnDataReceived(struct gatt_db_attribute *attrib,
		unsigned int id, uint16_t offset, const uint8_t *value, size_t len,
		uint8_t opcode, struct bt_att *att)
{
	int current_state = value[0] >> 5;
	int current_dir = value[0] & 0x7;

	m_carParamOut.mutex.lock();
		m_carParamOut.dir = current_dir;
		switch (current_state)
		{
		case 0:
			m_carParamOut.idle = false;
			m_carParamOut.mode = false;
			m_carParamOut.moving = false;
			m_carParamOut.turbo = false;
			break;
		case 1:
			m_carParamOut.idle = true;
			m_carParamOut.mode = false;
			m_carParamOut.moving = false;
			m_carParamOut.turbo = false;
			break;
		case 2:
			m_carParamOut.idle = true;
			m_carParamOut.mode = false;
			m_carParamOut.moving = true;
			m_carParamOut.turbo = false;
			break;
		case 3:
			m_carParamOut.idle = true;
			m_carParamOut.mode = false;
			m_carParamOut.moving = false;
			m_carParamOut.turbo = true;
			break;
		case 4:
			m_carParamOut.idle = true;
			m_carParamOut.mode = false;
			m_carParamOut.moving = true;
			m_carParamOut.turbo = true;
			break;
		case 5:
			m_carParamOut.idle = false;
			m_carParamOut.mode = true;
			m_carParamOut.moving = false;
			m_carParamOut.turbo = false;
			break;
		case 6:
			m_carParamOut.idle = true;
			m_carParamOut.mode = true;
			m_carParamOut.moving = false;
			m_carParamOut.turbo = false;
			break;
		}
	m_carParamOut.mutex.unlock();
}

}  // namespace acm

