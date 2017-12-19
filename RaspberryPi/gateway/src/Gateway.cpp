/*
 * Gateway.cpp
 *
 *  Created on: Dec 10, 2017
 *      Author: jucom
 */

#include "Gateway.hpp"
#include "common-defs.h"

namespace acm
{

void Gateway::Ble_advertise()
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

struct gatt_db_attribute * feedb = nullptr;
uint16_t handle;

void gatt_svc_chngd_ccc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{

}


void gap_device_name_write_cb(gatt_db_attribute *attrib, unsigned int id, uint16_t offset, const uint8_t *value, size_t len, uint8_t opcode, bt_att *att, void *user_data)
{
	GattServer* self = static_cast<decltype(self)>(user_data);
	uint8_t error = 0;

	PRLOG("Direction Write called\n");

	if (!(offset + len))
	{
		self->m_device_name.clear();
		goto done;
	}

	if (offset > self->m_device_name.size())
	{
		error = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	if (value)
		self->m_device_name += std::string(reinterpret_cast<const char*>(value), len);

done:
	gatt_db_attribute_write_result(attrib, id, error);
}

int Gateway::run()
{
	/*
	 * Initialize main event loop
	 */
	mainloop_init();

	/*
	 * Enable le advertising
	 */
	Ble_advertise();

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
	struct gatt_db_attribute *acm_service = m_gattServer.add_service(
			BleUuid_AcmService);

	m_gattServer.add_characteristic(acm_service, BleUuid_AcmCharState,
	BT_ATT_PERM_WRITE, BT_GATT_CHRC_PROP_WRITE_WITHOUT_RESP,
	NULL, Ble_onDataReceived, this);

	feedb = m_gattServer.add_characteristic(acm_service, BleUuid_AcmCharFeedb,
	BT_ATT_PERM_READ, BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_NOTIFY,
	NULL, NULL, &m_gattServer);

	handle = gatt_db_attribute_get_handle(feedb);

	bt_uuid_t uuid;
	bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);
	gatt_db_service_add_descriptor(acm_service, &uuid, BT_ATT_PERM_READ, gatt_svc_chngd_ccc_read_cb, nullptr, this);

	m_gattServer.set_service_active(acm_service, true);

	/*
	 * Initialize CAN controller
	 */
	m_canController.registerMessageType(CanId_DirectionCmd, 2);
	m_canController.registerMessageType(CanId_SpeedCmd, 2);
	// TODO: add other type of messages
	m_canController.mainloopAttachRead(Can_onDataReceived, this);

	/*
	 * Initialize timer to periodically write data on CAN
	 */
	timer.setDuration(CAN_WRITE_PERIOD_MS);
	timer.mainloopAttach(Can_onTimeToSend, this);

	/*
	* Create thread autonomous
	*/
	pthread_create(thread, NULL,AutonomousControl, NULL); 


	/*
	 * Initialize signal to handle SIGINT and SIGTERM
	 */
	signal.add(SIGINT);
	signal.add(SIGTERM);
	signal.mainloopAttach(signalCallback, this);

	/*
	 * Run main event loop
	 */
	return mainloop_run();
}

//TODO: replace this function by a periodic signal (period must be defined)
void Gateway::AutonomousControl(void *arg) {
	int it ; 
	int stop ; 
	while(1) {
		obstacle obstacles[6]; 

		self->m_carParamIn.mutex.lock();
		memcpy(obstacles,self->m_carParamIn.obstacles,sizeof(obstacles));		
		self->m_carParamIn.mutex.unlock();	

		it=0 ; 
		stop=0 ; 
		while(it<6) {
			if(obstacles[it].detected==1 && obstacles[it].mobile==0  obstacles[it].dist<=30) { //static and speed= 1.5
				printf("====STOP====\n") ; //send STOP frame 
				break ;
			} else if(obstacles[it].detected==1 && obstacles[it].mobile==1  obstacles[it].dist<=100) { //mobile and speed= 3
				printf("====STOP====\n") ; //send STOP frame 
				break ; 
			}
			it++ ; 
		}
		usleep(200) ; 		
	}
}

void Gateway::signalCallback(int signum, void *user_data)
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

void Gateway::Can_onTimeToSend(void* user_data)
{
	Gateway* self = (decltype(self)) user_data;

	static int i = 0;

	//Multiply the period by 2
	if (i == 0)
	{
		self->m_carParamOut.mutex.lock();
		int dir = self->m_carParamOut.dir;
		self->m_carParamOut.mutex.unlock();

		/*uint16_t direction = '0';
		switch (dir)
		{
		case 0:
		case 1:
			direction = '2';
			break;
		case 3:
		case 4:
			direction = '1';
			break;
		default:
			direction = '0';
		}*/

		self->m_canController.sendMessage(CanId_DirectionCmd,
				(uint8_t*) &dir);
		i = 1;
	}
	else if (i == 1)
	{
		self->m_carParamOut.mutex.lock();
		int is_moving = self->m_carParamOut.moving;
		int is_turbo = self->m_carParamOut.turbo;
		self->m_carParamOut.mutex.unlock();

		uint16_t speed = 0;

		if (is_moving)
		{
			if (is_turbo)
				speed = 2;
			else
				speed = 1;
		}

		self->m_canController.sendMessage(CanId_SpeedCmd, (uint8_t*) &speed);

		i = 0;
	}
}

void confirm_write(gatt_db_attribute *attr, int err, void *user_data)
{
	if (!err)
		return;

	fprintf(stderr, "Error caching attribute %p - err: %d\n", attr, err);
	exit(1);
}

void Gateway::Can_onDataReceived(int fd, uint32_t events, void *user_data)
{
	Gateway* self = (decltype(self)) user_data;

	int nbytes;
	struct can_frame frame;

	// TODO: use CanController::recv()
	nbytes = read(self->m_canController.fd(), &frame, sizeof(struct can_frame));

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

	if (frame.can_id == CanId_UltrasoundData)
	{
		//printf("Reception Data ultrasons \n");
		//printf("%d bytes : ", nbytes);
		/*printf("id : %d\n", frame.can_id);
		 printf("dlc : %d\n", frame.can_dlc);*/

		uint8_t us[6];
		obstacle obst[6];
		memcpy(us, frame.data, sizeof(us));
		/*printf("data_us : ");
		 for (int i = 0; i < 6; i++)
		 printf("%0d ", data_us[i]);
		 printf("\n");*/
		self->m_obstacleDetector.detect(us, obst);


		obst_detection = (((obst[0].detected or obst[1].detected) ? 0x01 : 0x00) << 2) |
						 (((obst[2].detected or obst[3].detected) ? 0x01 : 0x00) << 1) |
						 (((obst[4].detected or obst[5].detected) ? 0x01 : 0x00) << 0);

		self->m_carParamIn.mutex.lock();
		self->m_carParamIn.obst=obst_detection ; 
		memcpy(self->m_carParamIn.obstacles,us,sizeof(self->m_carParamIn.obstacles));		
		self->m_carParamIn.mutex.unlock();

		//function to delete after debug
		self->m_obstacleDetector.print(obst);
	}
	if (frame.can_id == CanId_SpeedData)
	{
		/*printf("Reception speed data \n");
		printf("%d bytes \n", nbytes);
		printf("id : %d\n", frame.can_id);
		printf("dlc : %d\n", frame.can_dlc);*/

		speed = frame.data[0] / 10;

		self->m_carParamIn.mutex.lock();
		self->m_carParamIn.speed=speed ; 
		self->m_carParamIn.mutex.unlock();

		//printf("speed_data : %d\n", speed);
	}
	if (frame.can_id == CanId_DirectionData)
	{
		/*printf("Reception direction data \n");
		printf("%d bytes \n", nbytes);
		printf("id : %d\n", frame.can_id);
		printf("dlc : %d\n", frame.can_dlc);*/

		self->m_carParamIn.mutex.lock();
		if(self->m_carParamIn.speed==0) 
			self->m_carParamIn.dir = 3 ; 
		else 
			self->m_carParamIn.dir = frame.data[0] ; 
		self->m_carParamIn.mutex.unlock();
		

		//printf("data dir : %d\n", frame.data[0]);
	}
	if (frame.can_id == CanId_BatteryData)
	{
		/*printf("Reception battery data \n");
		printf("%d bytes \n", nbytes);
		printf("id : %d\n", frame.can_id);
		printf("dlc : %d\n", frame.can_dlc);*/

		bat = frame.data[0] ; 

		self->m_carParamIn.mutex.lock();
		self->m_carParamIn.bat=bat ; 
		self->m_carParamIn.mutex.unlock();
		//printf("data batt : %d\n", bat);
	}

	uint8_t buf[2] = {0x00, 0x00};
	self->m_carParamIn.mutex.lock();
	obst_detection=m_carParamIn.obst ; 
	speed=m_carParamIn.speed ; 
	dir=m_carParamIn.dir ; 
	bat=m_carParamIn.bat ; 
	self->m_carParamIn.mutex.unlock();

	buf[0] = ((speed & 0x07) << 3) | ((dir & 0x03) << 1) | ((0x00 & 0x00) << 0);
	buf[1] = ((0x00) << 4) | ((obst_detection) << 2) | ((bat & 0x03) << 0);

	// TODO: GattServer::sendNotification
	bt_gatt_server_send_notification(self->m_gattServer.m_gatt_server, handle, buf, sizeof(buf));
}

void Gateway::Ble_onTimeToSend(void* user_data)
{
	UNUSED(user_data);
}

void Gateway::Ble_onDataReceived(struct gatt_db_attribute *attrib,
		unsigned int id, uint16_t offset, const uint8_t *value, size_t len,
		uint8_t opcode, struct bt_att *att, void *user_data)
{
	Gateway* self = (decltype(self)) user_data;
	UNUSED(self);

	int current_state = value[0] >> 5;
	int current_dir = value[0] & 0x7;

	self->m_carParamOut.mutex.lock();

	self->m_carParamOut.dir = current_dir;

	switch (current_state)
	{
	case 0:
		self->m_carParamOut.idle = false;
		self->m_carParamOut.mode = false;
		self->m_carParamOut.moving = false;
		self->m_carParamOut.turbo = false;
		break;
	case 1:
		self->m_carParamOut.idle = true;
		self->m_carParamOut.mode = false;
		self->m_carParamOut.moving = false;
		self->m_carParamOut.turbo = false;
		break;
	case 2:
		self->m_carParamOut.idle = true;
		self->m_carParamOut.mode = false;
		self->m_carParamOut.moving = true;
		self->m_carParamOut.turbo = false;
		break;
	case 3:
		self->m_carParamOut.idle = true;
		self->m_carParamOut.mode = false;
		self->m_carParamOut.moving = false;
		self->m_carParamOut.turbo = true;
		break;
	case 4:
		self->m_carParamOut.idle = true;
		self->m_carParamOut.mode = false;
		self->m_carParamOut.moving = true;
		self->m_carParamOut.turbo = true;
		break;
	case 5:
		self->m_carParamOut.idle = false;
		self->m_carParamOut.mode = true;
		self->m_carParamOut.moving = false;
		self->m_carParamOut.turbo = false;
		break;
	case 6:
		self->m_carParamOut.idle = true;
		self->m_carParamOut.mode = true;
		self->m_carParamOut.moving = false;
		self->m_carParamOut.turbo = false;
		break;
	}

	int tmp_dir = self->m_carParamOut.dir;
	int tmp_mov = self->m_carParamOut.moving;

	self->m_carParamOut.mutex.unlock();

	//printf("dir 	: %d\n", (int) tmp_dir);
	//printf("moving 	: %d\n", (int) tmp_mov);
}

}  // namespace acm

