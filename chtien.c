/*


    Ryan 2012/08/31

    add Zcl data Response Judge

 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include "../include/debug.h"

#define start 0x02
#define end   0x03
#define atcmd   16
#define uartAck 8192

#if 1
static char *serial_name = "/dev/ttyS0";
#else
//static char *serial_name = "/dev/ttyUSB0";
#endif


/* cmd ok !! */
unsigned char *str="$00,56,000D6F00008E6C34,C001,F007,01,01,FFFF,FF,FF,FF,FFFF,010301230001743C\r\n";
unsigned char *deviceList="$00,23,FFFFFFFFFFFF0000,C001,0C00,FF,FF,FFFF,03,FF,FF,FFFF";

unsigned char *joinNetworkHead = "$00,23,";
unsigned char *joinNetworkTail = ",C001,0C00,FF,FF,FFFF,01,0A,FF,FFFF";
unsigned char *leaveNetworkHead = "$00,23,";
unsigned char *leaveNetworkTail = ",0000,0034,FF,FF,FFFF,FF,FF,FF,FFFF,0000000000000000";

char ttyName[100];
unsigned char sensorMsgBuf[100];
unsigned char sendUartBuf[100];
unsigned char input[100];
unsigned char macAddress[100];
unsigned char output[100];
unsigned char sendUartBuf[100];
unsigned char sum, outlen;
unsigned char rcvUartBuf[uartAck];
int i = 0, j = 0, inlen, len;

/* Table of CRC values for high-order byte */
static const uint8_t table_crc_hi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/* Table of CRC values for low-order byte */
static const uint8_t table_crc_lo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

//baudrateSet(baudrate)
int uartInit(int fd,char *comport, int baudrate)
{
    int speed;
    int baudr;
    static struct termios newtio;    
    //dbg_printf("comport = %s\n",comport); 
    //dbg_printf("baudrate = %d\n",baudrate);   
    //sprintf(speed,"B%d", baudrate);
    //dbg_printf("%s\n",speed);
    switch(baudrate)
    {    
      case 2400 : 
        baudr = B2400;
        break;
      case 4800 :
        baudr = B4800;
        break;
      case 9600 :
        baudr = B9600;
        break;
      case 19200 : 
        baudr = B19200;
        break;
      case 38400 : 
        baudr = B38400;
        break;
      case 57600 : 
        baudr = B57600;
        break;
      case 115200 : 
        baudr = B115200;
        break;
    
      default : error("invalid baudrate");
                   //S printf("invalid baudrate\n");
                   //S return(1);
                   break;
  }

    
    /*open Uart */
    //fd = open( serial_name, O_RDWR | O_NOCTTY | O_NDELAY ); //270
    fd = open( comport, O_RDWR | O_NOCTTY | O_NDELAY );
    if ( fd < 0 )
    {
        perror( "Cannot Open Serial Port !\n" );
        return 0;
    }
    else
    {
        printf( "open Serial Port success !\r\n" );
    }

    /**/
    tcgetattr( fd, &newtio );
   
   	/* Baudrate setup */
    if( cfsetispeed( &newtio, baudr ) < 0)
    {
      perror("cfsetispeed");
    }
    if( cfsetospeed( &newtio, baudr ) < 0)
    {
      perror("cfsetospeed");
    }
    

    // diable RX echo
#if 1 
	newtio.c_iflag &= ~(IUCLC | IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /*Input*/
	newtio.c_oflag &= ~OPOST; /*Output*/
	newtio.c_cflag |= CRTSCTS;

#else
    newtio.c_iflag &= ~( IUCLC | IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON );
    newtio.c_lflag &= ~( ICANON | ECHO | ECHOE | ISIG ); /*Input*/
    newtio.c_lflag &= baudr | CRTSCTS | CS8 | CLOCAL | CREAD ; ; /*Input*/
    newtio.c_oflag &= ~OPOST; /*Output*/
#endif
    //newtio.c_cc[VTIME]=  1;
    //newtio.c_cc[VMIN] = 16;

    /*瞼艂Y簣N簫簿癒A瞻瞿繙猻O禮_?礎糧繡礙*/

    if ( tcsetattr( fd, TCSANOW, &newtio ) < 0 )
    {
        printf( "setup tty Fail!" );
    }

    return fd;
}


unsigned char CheckSum( unsigned char *msgBuf, unsigned char msgLen, unsigned char len )
{
    int i;
    unsigned char checksum = 0;

    for ( i = 0; i < msgLen; i ++ )
    {
        checksum += msgBuf[i];
    }

    printf( "sum = %x ", checksum + len );
    return ( 0xFF - ( checksum + len ) );
}

int dataDump(char* rcvUartBuf,int dataSize)
{  
  printf( "[Request]: " );
  for ( i = 0; i < dataSize; i++ )  
    printf( "%C", rcvUartBuf[i] );
#if 0
  printf( "\n\n[Parse]: " );
  for ( i = 0; i < readLen; i++ )
    printf( "%C", rcvUartBuf[i] );
  printf( "\n**********************************End*****************************\n\n" );
        //return 0;8565C2000001040006010501
#endif
	return 0;
}


static uint16_t crc16(uint8_t *buffer, uint16_t buffer_length)
{
    uint8_t crc_hi = 0xFF; /* high CRC byte initialized */
    uint8_t crc_lo = 0xFF; /* low CRC byte initialized */
    unsigned int i; /* will index into CRC lookup */

    /* pass through message buffer */
    while (buffer_length--) {
        i = crc_hi ^ *buffer++; /* calculate the CRC  */
        crc_hi = crc_lo ^ table_crc_hi[i];
        crc_lo = table_crc_lo[i];
    }

    return (crc_hi << 8 | crc_lo);
}


int _modbus_rtu_send_msg_pre(uint8_t *req, int req_length)
{
    uint16_t crc = crc16(req, req_length);    
	dbg_printf("%X\n",crc);
    req[req_length++] = crc >> 8;
    req[req_length++] = crc & 0x00FF;

    return req_length;
}


int main(int argc,char* argv[])
{
  int opt;
  int fd;
  char *device=NULL;
  char *baudrate=NULL;
    
  while ((opt = getopt(argc, argv, "d:b:-:")) != -1) 
  {
    switch (opt) 
    {
      case 'd':
        //dbg_printf("-d %s\n",optarg);
        device=optarg;
        dbg_printf("-d %s\n",device);
        break;
      case 'b':
        //dbg_printf("-b %s\n",optarg);
        baudrate=optarg;
        dbg_printf("-b %s\n",baudrate);
        break;
      case '-':
        //dbg_printf("- %s\n",optarg);
        if(strcmp(optarg,"help")==0)
        {
          printf( "Usage  : [-d device] [-b] baudrate\n");
          printf( "         %s -d /dev/ttyUSB0 -b 38400 \n",argv[0]);
          return 0;
        }        
        break;  
      case '?':
      default: /* '?' */
        fprintf(stderr, "Usage: %s [-d device] [-b] baudrate\n", argv[0]);
                   exit(EXIT_FAILURE);
    }
  }
    fd = uartInit(fd, device==NULL ? serial_name: device 
                    , baudrate==NULL ? 115200 : atoi(baudrate));
    while ( 1 )
    {
        int i;
        short readLen = 0;
        
        //memset(sendUartBuf, 0, sizeof(sendUartBuf));
        memset( rcvUartBuf,  0, sizeof( rcvUartBuf ) );
        memset( input,  0, sizeof( input ) );       
        memset( output,  0, sizeof( output ) );       
       
        //printf( "Atcmd no include start, length, checkSum, end \n" );
        printf( "j: join device, l: leave device, s : Show ,d: device list, E : Exit program \n" );
        printf( " Enter Atcmd : " );    
        
        scanf( "%s", input ); //if input "s" then command "s",  s : Show 

        if(input[0]=='j'||input[0]=='J'&&input[1]==0)      /* j: join device */
        {
          printf(" Enter Mac Address : ");
          scanf( "%s", macAddress );             
          strcat(output,joinNetworkHead);
          strcat(output,macAddress);               
          strcat(output,joinNetworkTail);   
          //printf("output = %s\n",output);
          //printf("output len = %d\n",strlen(output));
          memcpy(input,output,strlen(output));
        }
        else if(input[0]=='l'||input[0]=='L'&&input[1]==0)  /* l: leave device */
        {
          printf(" Enter Mac Address : ");
          scanf( "%s", macAddress );             
          strcat(output,leaveNetworkHead);
          strcat(output,macAddress);               
          strcat(output,leaveNetworkTail);   
          //printf("output = %s\n",output);
          //printf("output len = %d\n",strlen(output));
          memcpy(input,output,strlen(output));;
        }
        else if(input[0]=='e'||input[0]=='E'&&input[1]==0) 
        {
          printf(" Exit Program\n");
          return 0;
        }
        else if(input[0]=='c'||input[0]=='C'&&input[1]==0)
        {
          printf(" enter continue \n"); 
          continue;
        }
        else if(input[0]=='d'||input[0]=='D'&&input[1]==0)
        {
          printf(" enter continue \n"); 
          memcpy(input,deviceList,strlen(deviceList));
        }
	    int write_count=0;		
	
    	len = strlen(input);
    	dbg_printf("inputlen= %d\n",len);
    	input[len++]='\r';
    	input[len++]='\n';    	
    	dbg_printf("inputlen= %d\n",strlen(input));

#if 0

for(i=0; i<strlen(input); i++)	
	{
  		write_count = write( fd, &input[i], 1);	
		//printf("%d",write_count++);
	}	
#else

        printf( "***********************************Start****************************\n\n" );
    	dataDump(input,strlen(input));
	    write(fd, input,strlen(input));
#endif
        printf( "\n" );        
        sleep( 1 );        
        
        readLen = read ( fd, rcvUartBuf, uartAck );
		printf("readLen = %d\n",readLen);
       
#if 0
        printf( "[Response]: " )

        for ( i = 0; i < uartAck; i++ )
        {
            printf( "%02X", rcvUartBuf[i] );
        }
#endif        
        printf( "\n\n" );
        printf( "[Response]: " );
        for ( i = 0; i < readLen; i++ )
        {
            printf( "%C", rcvUartBuf[i] );
        }
        printf( "\n**********************************End*****************************\n\n" );        
    }
    return 0;
}






