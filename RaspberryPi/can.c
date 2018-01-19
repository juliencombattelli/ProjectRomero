/*
 * can.c
 *
 *  Created on: 27 nov. 2017
 *      Author: JulienCombattelli
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <pthread.h>
#include <signal.h>
#include "can.h"

int socket_send;

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

void Print_Obstacle_Detection(obstacle ** obst) {
    int i ;
    for(i=0 ; i<6 ; i++) {
        if(obst[i]->detected) {
            printf("||=============================||\n") ;
            switch(i) {
                case 0:
                    printf("OBASTACLE DETECTED ON || SIDE LEFT || (SL)\n") ;
                    break ;
                case 1:
                    printf("OBASTACLE DETECTED ON || FRONT SIDE LEFT || (FSL)\n") ;
                    break ;
                case 2:
                    printf("OBASTACLE DETECTED ON || FRONT LEFT || (FL)\n") ;
                    break ;
                case 3:
                    printf("OBASTACLE DETECTED ON || FRONT RIGHT || (FR)\n") ;
                    break ;
                case 4:
                    printf("OBASTACLE DETECTED ON || FRONT SIDE RIGHT || (FSR)\n") ;
                    break ;
                default:
                    printf("OBASTACLE DETECTED ON || SIDE RIGHT || (SR)\n") ;
                    break ;
            }
            if(obst[i]->mobile) {
                printf("Mobile\n") ;
            } else {
                printf("Static\n") ;
            }
            printf("Dist : %d cm\n",obst[i]->dist) ;
            printf("||=============================||\n") ;
        }
    }
}

//init the send
int initSend()
{
    int s;
    struct sockaddr_can addr;
    
    struct ifreq ifr;
    
    const char *ifname = "can0";
    
    if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        perror("Error while opening socket");
    }
    
    strcpy(ifr.ifr_name, ifname);
    ioctl(s, SIOCGIFINDEX, &ifr);
    
    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    
    printf("%s at index %d\n", ifname, ifr.ifr_ifindex);
    
    if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
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
void SendData_Speed(uint16_t speed)
{
    int nbytes;
    struct can_frame frame;
    frame.can_id  = 0x003; //id commande vitesse
    frame.can_dlc = 2;
    struct data_speed_command data_speed;
    data_speed.speed_command.num_speed_command = speed;
    create_speed_command_frame(data_speed, frame.data);
    
    nbytes = write(socket_send, &frame, sizeof(struct can_frame));
    
    //printf("Wrote %d bytes\n", nbytes);
}

/*
 * TODO: Add value from bluetooth in parameters
 * Call the function create_direction_command_frame (learn more about structures  : canRaspi.h)
 */
//send data direction command
void SendData_Direction(uint16_t direction)
{
    int nbytes;
    struct can_frame frame;
    frame.can_id  = 0x002; //id commande direction
    frame.can_dlc = 2;
    struct data_direction_command data_direction;
    data_direction.direction_command.num_direction_command = direction;
    create_direction_command_frame(data_direction, frame.data);
    
    nbytes = write(socket_send, &frame, sizeof(struct can_frame));
    
    //printf("Wrote %d bytes\n", nbytes);
}

//receive and print data depending on the id of the frame
void * ReceiveData()
{
    int s;
    int nbytes;
    struct sockaddr_can addr;
    struct can_frame frame;
    struct ifreq ifr;
    
    const char *ifname = "can0";
    
    if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        perror("Error while opening socket");
    }
    
    strcpy(ifr.ifr_name, ifname);
    ioctl(s, SIOCGIFINDEX, &ifr);
    
    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    
    printf("%s at index %d\n", ifname, ifr.ifr_ifindex);
    
    if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Error in socket bind");
    }
    
    while(1)
    {
        nbytes = read(s, &frame, sizeof(struct can_frame)) ;
        
        if(nbytes < 0)
        {
            perror("ERROR can raw socket read ") ;
            exit(1) ;
        }
        
        if(nbytes < sizeof(struct can_frame))
        {
            fprintf(stderr, "read : incomplete can frame\n") ;
            exit(1) ;
        }
        
        if (frame.can_id == 0x001) {
             printf("Reception Data ultrasons \n") ;
             printf("%d bytes \n",nbytes) ;
             printf("id : %d\n",frame.can_id) ;
             printf("dlc : %d\n",frame.can_dlc) ;
            
             data_ultrasound data_us ;
             obstacle * obst[6] ;
             int it ;
             for(it=0 ; it<6 ; it++) {
                 obst[it]=malloc(sizeof(obstacle));
             }
             memcpy(data_us.ultrasound.num_ultrasound,frame.data,sizeof(frame.data)) ;
             Obstacle_Detection(data_us,obst) ;
            
             //function to delete after debug
             Print_Obstacle_Detection(obst) ;
            
            //TODO: SEND DATA TO APP
         }
         if (frame.can_id == 0x005) {
             printf("Reception speed data \n") ;
             printf("%d bytes \n",nbytes) ;
             printf("id : %d\n",frame.can_id) ;
             printf("dlc : %d\n",frame.can_dlc) ;
             
             data_odometer data_speed ;
             data_speed.odometer.num_odometer=frame.data[0] ;
             printf("speed_data : %d\n",data_speed.odometer.num_odometer) ;
             
             //TODO: SEND DATA TO APP
             
         }
         if (frame.can_id == 0x004) {
             printf("Reception direction data \n") ;
             printf("%d bytes \n",nbytes) ;
             printf("id : %d\n",frame.can_id) ;
             printf("dlc : %d\n",frame.can_dlc) ;
             
             data_potentiometer data_direction;
             data_direction.potentiometer.num_potentiometer = frame.data[0] ;
             printf("data_direction : %d\n",data_direction.potentiometer.num_potentiometer) ;
             
             //TODO: SEND DATA TO APP
         }
        if (frame.can_id == 0x006) {
            printf("Reception battery data \n") ;
            printf("%d bytes \n",nbytes) ;
            printf("id : %d\n",frame.can_id) ;
            printf("dlc : %d\n",frame.can_dlc) ;
            
            data_battery data_batt ;
            data_batt.battery.num_battery=frame.data[0] ;
            printf("data0 : %d\n",data_batt.battery.num_battery) ;
            
            //TODO: SEND DATA TO APP
        }
    }
}

void Obstacle_Detection(data_ultrasound data, obstacle ** obst) {
    int i ;
    for(i=0 ; i<6 ; i++) {
        obst[i]->detected= (data.ultrasound.num_ultrasound[i])&0x01 ;
        obst[i]->mobile= ((data.ultrasound.num_ultrasound[i])&0x02)>>1 ;
        obst[i]->dist= ((data.ultrasound.num_ultrasound[i])&0xFC)>>1 ;
    }
}


 int main(void)
 {
	 pthread_t threadReceive ;
 	int i=0 ;
 	//create and launch a thread to receive data
 	if(pthread_create(&threadReceive,NULL,ReceiveData,NULL)==1) {
 		fprintf (stderr, "%s", strerror (1));
 	}
	 //init send configuration
	 socket_send = initSend();
	 //init the function called by the signal SIGALRM
	// signal(SIGALRM,SendData_Speed);
	 //generate a signal SIGALRM each 25ms
	// ualarm(25000,25000) ;
	 //wait the end of the thread created (impossible due to the while 1)
	 pthread_join(threadReceive,NULL) ;
	 return 0;
 }
 
