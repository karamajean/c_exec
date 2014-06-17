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
#include "../include/debug.h"

#define start 0x02
#define end   0x03
#define atcmd   16
#define uartAck 500

//static char *serial_name = "/dev/ttyS0";
static char *serial_name = "/dev/ttyUSB0";

char ttyName[100];
unsigned char sensorMsgBuf[100];
unsigned char sendUartBuf[100];
unsigned char input[100];
unsigned char output[100] = {0};
unsigned char sendUartBuf[100];
unsigned char sum, outlen;
unsigned char rcvUartBuf[uartAck];
int i = 0, j = 0, inlen, len;

int uartInit(int fd,char *comport, int baudrate)
{
    
    int baudr=0;
    static struct termios newtio;    
   
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
    
      default : perror("invalid baudrate");
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
    
    tcgetattr( fd, &newtio );   
    
    if( cfsetispeed( &newtio, baudr ) < 0)
    {
      perror("cfsetispeed");
    }
    if( cfsetospeed( &newtio, baudr ) < 0)
    {
      perror("cfsetospeed");
    }
    

    // diable RX echo
    newtio.c_iflag &= ~( IUCLC | IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON );
    newtio.c_lflag &= ~( ICANON | ECHO | ECHOE | ISIG ); /*Input*/
    newtio.c_lflag &= baudr | CS8 | CLOCAL | CREAD ; ; /*Input*/
    newtio.c_oflag &= ~OPOST; /*Output*/

    //newtio.c_cc[VTIME]=  1;
    //newtio.c_cc[VMIN] = 16;   

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

void dataDump(int fd,char* rcvUartBuf)
{ 
  int readLen;  
  
  readLen = read ( fd, rcvUartBuf, uartAck);
  
  printf( "***********************************Start****************************\n\n" );
  printf( "[R]: " );
  for ( i = 0; i < readLen; i++ )  
    printf( "%02X", rcvUartBuf[i] );

  printf( "\n\n[Parse]: " );
  for ( i = 0; i < readLen; i++ )
    printf( "%C", rcvUartBuf[i] );
  printf( "\n**********************************End*****************************\n\n" );        
}

int main(int argc,char* argv[])
{
  int opt;
  int fd=0;
  char *device=NULL;
  char *baudrate=NULL;
    
  while ((opt = getopt(argc, argv, "d:b:-:")) != -1) 
  {
    switch (opt) 
    {
      case 'd':        
        device=optarg;
        dbg_printf("-d %s\n",device);
        break;
      case 'b': 
        baudrate=optarg;
        dbg_printf("-b %s\n",baudrate);
        break;
      case '-':       
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
                    , baudrate==NULL ? 19200 : atoi(baudrate));
    while ( 1 )
    {
        int i;
        short readLen = 0;        
       
        memset( rcvUartBuf,  0, sizeof( rcvUartBuf ) );
        memset( input,  0, sizeof( input ) );       
       
        printf( "Atcmd no include start, length, checkSum, end \n" );
        printf( "Enter Atcmd : " );    
        
        scanf( "%s", input );
    
        if((input[0]=='q' || input[0]=='Q') && input[1]==0 ) 
        {
          printf(" enter quit \n");
          return 0;
        }
        else if(input[0]=='s'|| (input[0]=='S'&&input[1]==0) )
        {
          printf(" enter Show \n");
          dataDump(fd,rcvUartBuf);  
          continue;
        }
        
        /*DEBUG*/
#ifdef RYAN_DEBUG        
        len = ( strlen( input ) / 2 );
        inlen = strlen( input );       
        printf( "inlen = %X\n", inlen ); 
#endif
  

        printf( "[T]: " );

        for ( i = 0; i < inlen; i++ )
        {
            if ( input[i] < 58 )                      /*0~9 字元轉數字 0~9*/
                input[i] -= 48;    
            
            if ( input[i] > 64 && input[i] < 71 )     /*A~F 字元轉數字 A~F*/     
                input[i] -= 55;            
            
            if ( input[i] > 96 && input[i] < 103 )    /*a~f 字元轉數字 a~f*/     
                input[i] -= 87;            
            
            output[i] = input[i]; 
#ifdef RYAN_DEBUG              
            printf( "%X", output[i] ); //debug
#endif           
        }     

        outlen = inlen;
        for ( i = 0; i < outlen/2; i++ )
        {
            output[i] = ( output[i * 2] << 4 ) | ( output[( i * 2 ) + 1] );
            printf( "%02X", output[i] );
        }
        printf( "\n" );
        
        printf( "outlen = %X ", outlen );
        sum = CheckSum ( output, outlen, len );
        printf( "len= %x ", len );
        printf( "checksum= %x \n", sum );      

        sendUartBuf[0] = 0x02;
        sendUartBuf[1] = 0x00;
        sendUartBuf[2] = outlen;

        for ( i = 0, j = 0; i < outlen + 5; i++, j++ )
        {
            sendUartBuf[i + 3] = output[j];
        }

        sendUartBuf[i - 2] = sum;
        sendUartBuf[i - 1] = 0x03;


        printf( "sendUartBuf = : " );

        for ( i = 0; i < len + 5; i++ )
        {
            printf( "%02X", sendUartBuf[i] );
        }
#ifdef RYAN_DEBUG                  
        printf( "\n sendbuf outlen = %d\n", outlen + 5 );
        write( fd, &sendUartBuf, outlen + 5 );
#endif
        printf( "\n" );

        
        sleep( 5 );

        /* read reponse */
        readLen = read ( fd, rcvUartBuf, uartAck );
   
        printf( "***********************************Start****************************\n\n" );

        printf( "[R]: " );

        for ( i = 0; i < uartAck; i++ )
        {
            printf( "%02X", rcvUartBuf[i] );
        }
        
        printf( "\n\n" );
        printf( "[Parse]: " );
        for ( i = 0; i < uartAck; i++ )
        {
            printf( "%C", rcvUartBuf[i] );
        }
        printf( "\n**********************************End*****************************\n\n" );      
    }

    return 0;
}






