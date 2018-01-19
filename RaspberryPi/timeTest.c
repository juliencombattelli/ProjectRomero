#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <pthread.h>
#include <signal.h>
#include "testData.h"

int socket_send;
int i ;
long timeS1, timeS2 ; 
long timeRU1, timeRU2 ;
long timePO1, timePO2 ; 
long timeHA1, timeHA2 ; 
 
long getMicrotime(){
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}


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

//        printf("%s at index %d\n", ifname, ifr.ifr_ifindex);

        if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("Error in socket bind");
        }

	VAL_SPEED.speed_command.bytes_speed_command[0] = '1';
	VAL_SPEED.speed_command.bytes_speed_command[1] = '2';
	
	VAL_DIRECTION.direction_command.bytes_direction_command[0] = '3';
        VAL_DIRECTION.direction_command.bytes_direction_command[1] = '4';
	return s;
}


//send data speed command
void SendData_Speed() 
{
	int nbytes;
	struct can_frame frame;
        frame.can_id  = 0x002; //id commande vitesse
        frame.can_dlc = 2;
        create_speed_command_frame(VAL_SPEED, frame.data);

        nbytes = write(socket_send, &frame, sizeof(struct can_frame));

  //      printf("Wrote %d bytes\n", nbytes);
	
	i = 0;
}

//send data direction command
void SendData_Direction() 
{
	int nbytes;
	struct can_frame frame;
        frame.can_id  = 0x003; //id commande direction
        frame.can_dlc = 2;
	create_direction_command_frame(VAL_DIRECTION, frame.data);

        nbytes = write(socket_send, &frame, sizeof(struct can_frame));

    //    printf("Wrote %d bytes\n", nbytes);
	
	i = 1;
}

void SendData(int sig)
{
        if (i == 0)
        {
		timeS1=getMicrotime() ; 
		printf("Time between two sends : %ld\n",(timeS1-timeS2)) ; 
                SendData_Direction();
		timeS2=getMicrotime() ; 
        }
        else if (i == 1)
        {
                SendData_Speed();
        }
}

//receive data
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

      //  printf("%s at index %d\n", ifname, ifr.ifr_ifindex);

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
						timeRU1=getMicrotime() ; 
						printf("Time between two ultrasounds %ld\n",(timeRU1-timeRU2)) ; 
						timeRU2=getMicrotime() ; 
	/*					printf("Reception Data ultrasons \n") ;
						printf("%d bytes \n",nbytes) ;
						printf("id : %d\n",frame.can_id) ;
						printf("dlc : %d\n",frame.can_dlc) ; 
						printf("data0 : %x\n",frame.data[0]) ;
						printf("data1 : %x\n",frame.data[1]) ;
						printf("data2 : %x\n",frame.data[2]) ;
						printf("data3 : %x\n",frame.data[3]) ;
						printf("data4 : %x\n",frame.data[4]) ;
						printf("data5 : %x\n",frame.data[5]) ; */
						
				}
				if (frame.can_id == 0x007)
				{
						timePO1=getMicrotime() ; 
						printf("Time between two dir %ld\n",(timePO1-timePO2)) ; 
						timePO2=getMicrotime() ; 
/*						printf("Reception Data potentiometre \n") ;
						printf("%d bytes \n",nbytes) ;
						printf("id : %d\n",frame.can_id) ;
						printf("dlc : %d\n",frame.can_dlc) ; 
						printf("data0 : %x\n",frame.data[0]) ;
						printf("data1 : %x\n",frame.data[1]) ;*/
						
				}
				if (frame.can_id == 0x006)
				{
						timeHA1=getMicrotime() ; 
						printf("Time between two speeds %ld\n",(timeHA1-timeHA2)) ; 
						timeHA2=getMicrotime() ; 
/*						printf("Reception Data capteurs effet hall \n") ;
						printf("%d bytes \n",nbytes) ;
						printf("id : %d\n",frame.can_id) ;
						printf("dlc : %d\n",frame.can_dlc) ; 
						printf("data0 : %x\n",frame.data[0]) ; //left odometer
						printf("data1 : %x\n",frame.data[1]) ; //left odometer
						printf("data2 : %x\n",frame.data[2]) ; //right odometer
						printf("data3 : %x\n",frame.data[3]) ; //right odometer */
				}
				
   
        }
        

}

//FUNCTION TO DELETE , ONLY FOR TEST
void handle(int sig) {
	printf("hello\n");
	
} 

int main(void)
{
        pthread_t threadReceive ; 
      	i=0 ;   
	timeS1=0 ; 
	timeS2=0 ; 
	timeRU1=0 ; 
	timeRU2=0 ; 
	timePO1=0 ; 
	timePO2=0 ; 
	timeHA1=0 ; 
	timeHA2=0 ; 
        if(pthread_create(&threadReceive,NULL,ReceiveData,NULL)==1) {
            fprintf (stderr, "%s", strerror (1));
        }
		socket_send = initSend();
		signal(SIGALRM,SendData);
		ualarm(25000,25000) ;  
	
        pthread_join(threadReceive,NULL) ; 
        return 0;
}
