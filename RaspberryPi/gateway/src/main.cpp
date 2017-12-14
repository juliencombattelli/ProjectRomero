/*
 * main.cpp
 *
 *  Created on: Dec 6, 2017
 *      Author: jucom
 */

#include "common-defs.h"

#include <map>
#include <utility>
#include <vector>
#include <memory>
#include <sys/timerfd.h>
#include <thread>
#include "CanController.hpp"
#include "CanSocket.hpp"
#include "CarParam.hpp"
#include "GattServer.hpp"
#include "Gateway.hpp"
#include "Timerfd.hpp"

#if 0

static void signal_callback(int signum, void *user_data)
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

constexpr uint16_t uuid_acm_serv 		= 0x7db9;
// Write without resp
constexpr uint16_t uuid_acm_char_state 	= 0xd288;
// Read and Notify
constexpr uint16_t uuid_acm_char_feedb 	= 0xc1cb;
constexpr uint16_t uuid_acm_char_alert 	= 0xdcb1;

CanController can("can0");

static void acm_state_write_cb(struct gatt_db_attribute *attrib, unsigned int id, uint16_t offset, const uint8_t *value, size_t len, uint8_t opcode, struct bt_att *att, void *user_data)
{
	GattServer* server = (GattServer*)user_data;
	UNUSED(server);

	int current_state = value[0] >> 5;
	int current_dir = value[0] & 0x7;


	pthread_mutex_lock(&acm_car_mutex);

	car.dir = current_dir;

	switch (current_state)
	{
	case 0:
		car.idle = false;
		car.mode = false;
		car.moving = false;
		car.turbo = false;
		break;
	case 1:
		car.idle = true;
		car.mode = false;
		car.moving = false;
		car.turbo = false;
		break;
	case 2:
		car.idle = true;
		car.mode = false;
		car.moving = true;
		car.turbo = false;
		break;
	case 3:
		car.idle = true;
		car.mode = false;
		car.moving = false;
		car.turbo = true;
		break;
	case 4:
		car.idle = true;
		car.mode = false;
		car.moving = true;
		car.turbo = true;
		break;
	case 5:
		car.idle = false;
		car.mode = true;
		car.moving = false;
		car.turbo = false;
		break;
	case 6:
		car.idle = true;
		car.mode = true;
		car.moving = false;
		car.turbo = false;
		break;
	}

	int tmp_dir = car.dir;
	int tmp_mov = car.moving;

	pthread_mutex_unlock(&acm_car_mutex);

	printf("dir 	: %d\n", (int)tmp_dir);
	printf("moving 	: %d\n", (int)tmp_mov);
}

int timer_set(int fd, unsigned int msec)
{
	struct itimerspec itimer;
	unsigned int sec = msec / 1000;

	memset(&itimer, 0, sizeof(itimer));
	itimer.it_interval.tv_sec = 0;
	itimer.it_interval.tv_nsec = 0;
	itimer.it_value.tv_sec = sec;
	itimer.it_value.tv_nsec = (msec - (sec * 1000)) * 1000 * 1000;

	return timerfd_settime(fd, 0, &itimer, NULL);
}

void timer_destroy(void *user_data)
{
	int *fdptr = (int*)user_data;

	if(fdptr)
	{
		close(*fdptr);
		*fdptr = -1;
	}
}

void timer_callback(int fd, uint32_t events, void *user_data)
{
	uint64_t expired;
	ssize_t result;

	result = read(fd, &expired, sizeof(expired));
	if (result != sizeof(expired))
	{
		fprintf(stderr,"Timer callback error reading\n");
		return;
	}
	std::thread canSender(SendData, user_data);
	canSender.detach();
	//SendData(0);
	timer_set(fd, 25);
}

void can_read_callback(int fd, uint32_t events, void *user_data)
{
	if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
	{
		mainloop_exit_failure();
		return;
	}

	std::thread canReceiver(ReceiveData, user_data);
	canReceiver.detach();
}

int main(int argc, char *argv[])
{
	int hci0_id = hci_devid("hci0");
	if(hci0_id < 0)
	{
		fprintf(stderr, "hci0 : no such device\n");
		exit(EXIT_FAILURE);
	}

	int hci0_dd = hci_open_dev(hci0_id);
	if(hci0_dd < 0)
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
	unsigned char buf[HCI_MAX_EVENT_SIZE] = {
			0x1E,
			0x02, 0x01, 0x1A, 0x1A,
			0xFF, 0x4C, 0x00, 0x02, 0x15, 0xE2, 0x0A, 0x39, 0xF4, 0x73, 0xF5, 0x4B, 0xC4, 0xA1,
			0x2F, 0x17, 0xD1, 0xAD, 0x07, 0xA9, 0x61, 0x00, 0x00, 0x00, 0x00, 0xC8, 0x00};
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

	int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	timer_set(timerfd, 25);

	sigset_t mask;

	mainloop_init();

	acm_car_init(&car);

	GattServer server("AnAwesomeGattServer");

	struct gatt_db_attribute *acm_service = server.add_service(uuid_acm_serv);

	server.add_characteristic(acm_service, uuid_acm_char_state,
			BT_ATT_PERM_WRITE, BT_GATT_CHRC_PROP_WRITE_WITHOUT_RESP,
			NULL, acm_state_write_cb, &server);

	server.add_characteristic(acm_service, uuid_acm_char_feedb,
			BT_ATT_PERM_READ, BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_NOTIFY,
			NULL, NULL, &server);

	server.add_characteristic(acm_service, uuid_acm_char_alert,
			BT_ATT_PERM_READ, BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_NOTIFY,
			NULL, NULL, &server);

	server.set_service_active(acm_service, true);


	printf("Running GATT server\n");

	can.registerMessageType(CanId_DirectionCmd, 2);
	can.registerMessageType(CanId_SpeedCmd, 2);

	if (mainloop_add_fd(timerfd, EPOLLIN | EPOLLET, timer_callback, &can, timer_destroy) < 0)
	{
		fprintf(stderr, "Failed to initialize timer callback\n");

		close(timerfd);
		return EXIT_FAILURE;
	}

	if (mainloop_add_fd(can.fd(), EPOLLIN, can_read_callback, &can, NULL) < 0)
	{
		fprintf(stderr, "Failed to initialize CAN read callback\n");

		return EXIT_FAILURE;
	}

	printf("Running CAN controller\n");

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	mainloop_set_signal(&mask, signal_callback, NULL, NULL);

	print_prompt();

	mainloop_run();

	printf("\n\nShutting down...\n");

	return EXIT_SUCCESS;
}
#endif

int main()
{
	acm::Gateway app;
	return app.run();
}
