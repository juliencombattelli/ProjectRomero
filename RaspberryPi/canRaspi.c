/*
 *  TODO: Add values from bluetooth
 *        Use the structures and functions already defined to send data
 *
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <pthread.h>
#include <signal.h>
#include "canRaspi.h"

int socket_send;
int i ;

data_speed_command VAL_SPEED;
data_direction_command VAL_DIRECTION;

//create the speed command data frame
void create_speed_command_frame(data_speed_command data, char * frame) 
{
        frame[0] = data.speed_command.bytes_speed_command[0];
        frame[1] = data.speed_command.bytes_speed_command[1];
}

//create the direction command data frame
void create_direction_command_frame(data_direction_command data, char * frame) 
{
        frame[0] = data.direction_command.bytes_direction_command[0];
        frame[1] = data.direction_command.bytes_direction_command[1];
}


//init the send
int initSend() 
{
	int s;
        struct sockaddr_can addr;

        struct ifreq ifr;

        const char *ifname = "can0";

        if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
                perror("Error while opening socket");
        }

        strcpy(ifr.ifr_name, ifname);
        ioctl(s, SIOCGIFINDEX, &ifr);

        addr.can_family  = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        printf("%s at index %d\n", ifname, ifr.ifr_ifindex);

        if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("Error in socket bind");
        }

    
        /*
         * DEFAULT VALUES FOR TESTING
         * CHANGE THE VALUES BY THE BLUETOOTH VALUES
        */
        VAL_SPEED.speed_command.bytes_speed_command[0] = '1';
        VAL_SPEED.speed_command.bytes_speed_command[1] = '2';
	
        VAL_DIRECTION.direction_command.bytes_direction_command[0] = '3';
        VAL_DIRECTION.direction_command.bytes_direction_command[1] = '4';
	return s;
}


/*
 * TODO: Add value from bluetooth in parameters
 * Call the function create_speed_command_frame (learn more about structures  : canRaspi.h)
*/
//send data speed command
void SendData_Speed() 
{
	int nbytes;
	struct can_frame frame;
        frame.can_id  = 0x002; //id commande vitesse
        frame.can_dlc = 2;
        create_speed_command_frame(VAL_SPEED, frame.data);

        nbytes = write(socket_send, &frame, sizeof(struct can_frame));

        printf("Wrote %d bytes\n", nbytes);
	
	i = 0;
}

/*
 * TODO: Add value from bluetooth in parameters
 * Call the function create_direction_command_frame (learn more about structures  : canRaspi.h)
 */
//send data direction command
void SendData_Direction() 
{
	int nbytes;
	struct can_frame frame;
        frame.can_id  = 0x003; //id commande direction
        frame.can_dlc = 2;
	create_direction_command_frame(VAL_DIRECTION, frame.data);

        nbytes = write(socket_send, &frame, sizeof(struct can_frame));

        printf("Wrote %d bytes\n", nbytes);
	
	i = 1;
}


//function called by the signal SIGALRM
void SendData(int sig)
{
        //Multiply the period by 2
        if (i == 0)
        {
                SendData_Direction();
        }
        else if (i == 1)
        {
                SendData_Speed();
        }
}

//receive and print data depending on the id of the frame
static void * ReceiveData()
{
        int s;
        int nbytes;
        struct sockaddr_can addr;
        struct can_frame frame;
        struct ifreq ifr;
		
		const char *ifname = "can0";

        if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
                perror("Error while opening socket");
        }

        strcpy(ifr.ifr_name, ifname);
        ioctl(s, SIOCGIFINDEX, &ifr);

        addr.can_family  = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        printf("%s at index %d\n", ifname, ifr.ifr_ifindex);

        if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("Error in socket bind");
        }
    
        while(1) {

                nbytes = read(s, &frame, sizeof(struct can_frame)) ;

                if(nbytes < 0) {
                        perror("ERROR can raw socket read ") ;
                        exit(1) ;
                }

                if(nbytes < sizeof(struct can_frame)) {
                        fprintf(stderr, "read : incomplete can frame\n") ;
                        exit(1) ;
                }

				if (frame.can_id == 0x001)
				{
						printf("Reception Data ultrasons \n") ;
						printf("%d bytes \n",nbytes) ;
						printf("id : %d\n",frame.can_id) ;
						printf("dlc : %d\n",frame.can_dlc) ; 
						printf("data0 : %x\n",frame.data[0]) ;
						printf("data1 : %x\n",frame.data[1]) ;
						printf("data2 : %x\n",frame.data[2]) ;
						printf("data3 : %x\n",frame.data[3]) ;
						printf("data4 : %x\n",frame.data[4]) ;
						printf("data5 : %x\n",frame.data[5]) ;
						
				}
				if (frame.can_id == 0x007)
				{
						printf("Reception Data potentiometre \n") ;
						printf("%d bytes \n",nbytes) ;
						printf("id : %d\n",frame.can_id) ;
						printf("dlc : %d\n",frame.can_dlc) ; 
						printf("data0 : %x\n",frame.data[0]) ;
						printf("data1 : %x\n",frame.data[1]) ;
						
				}
				if (frame.can_id == 0x006)
				{
						printf("Reception Data capteurs effet hall \n") ;
						printf("%d bytes \n",nbytes) ;
						printf("id : %d\n",frame.can_id) ;
						printf("dlc : %d\n",frame.can_dlc) ; 
						printf("data0 : %x\n",frame.data[0]) ; //left odometer
						printf("data1 : %x\n",frame.data[1]) ; //left odometer
						printf("data2 : %x\n",frame.data[2]) ; //right odometer
						printf("data3 : %x\n",frame.data[3]) ; //right odometer
				}
				
   
        }
        

}


int main(void)
{
        pthread_t threadReceive ; 
      	i=0 ;
        //create and launch a thread to receive data
        if(pthread_create(&threadReceive,NULL,ReceiveData,NULL)==1) {
            fprintf (stderr, "%s", strerror (1));
        }
        //init send configuration
		socket_send = initSend();
    
        //init the function called by the signal SIGALRM
		signal(SIGALRM,SendData);
        //generate a signal SIGALRM each 25ms
		ualarm(25000,25000) ;  
	
        //wait the end of the thread created (impossible due to the while 1)
        pthread_join(threadReceive,NULL) ; 
        return 0;
}
