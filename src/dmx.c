/*****************************************************************************
* Include Files                                                              *
*****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <dirent.h>

// Driver header file
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>


/*****************************************************************************
* Explicit External Declarations                                             *
*****************************************************************************/

/*****************************************************************************
* Local Macro Declarations                                                   *
*****************************************************************************/

#define PRU_NUM 	0

// def dmx_pin 6 is physical pin 3 on the P8 header or gpio 38
// def dmx_pin 12 is physical pin 12 on the P8 header or gpio 44
#define DMX_PIN (12)
#define DMX_CHANNELS (2)

#define DMX_HALT_ADDR (0x100)
#define DMX_CHANNELS_ADDR (0x101)
#define DMX_PIN_ADDR (0x102)

#define UDP_PORT (9930)
#define UDP_BUFLEN (1024*64)

#define AM33XX

/*****************************************************************************
* Local Typedef Declarations                                                 *
*****************************************************************************/


/*****************************************************************************
* Local Function Declarations                                                *
*****************************************************************************/

static int LOCAL_exampleInit ();
static void LOCAL_export_pin (int);
static void LOCAL_unexport_pin (int);
static void LOCAL_udp_listen ();
static void diep (char*);

/*****************************************************************************
* Local Variable Definitions                                                 *
*****************************************************************************/


/*****************************************************************************
* Intertupt Service Routines                                                 *
*****************************************************************************/


/*****************************************************************************
* Global Variable Definitions                                                *
*****************************************************************************/

static void *pruDataMem;
static unsigned char *pruDataMem_byte;

/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/

int main (void)
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
    
    printf("\nINFO: Starting %s example.\r\n", "dmx");
    /* Initialize the PRU */
    prussdrv_init ();		

    /* physical pin 3 on the P8 header is gpio 38 */
    /* physical pin 12 on the P8 header is gpio 44 */
    LOCAL_export_pin(44);
    
    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }
    
    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);

    /* Initialize example */
    printf("\tINFO: Initializing example.\r\n");
    LOCAL_exampleInit();
    
    /* Execute example on PRU */
    printf("\tINFO: Executing example.\r\n");
    prussdrv_exec_program (PRU_NUM, "./dmx.bin");
    LOCAL_udp_listen();
 
    pruDataMem_byte[DMX_HALT_ADDR] = 1;
    
    /* Wait until PRU0 has finished execution */
    printf("\tINFO: Waiting for HALT command.\r\n");
    prussdrv_pru_wait_event (PRU_EVTOUT_0);
    printf("\tINFO: PRU completed transfer.\r\n");
    prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);

    /* Disable PRU and close memory mapping*/
    prussdrv_pru_disable (PRU_NUM);
    prussdrv_exit ();

    /* see comments above */
    LOCAL_unexport_pin(44);

    return(0);

}

/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

static void diep(char *s)
{
  perror(s);
  exit(1);
}

static int LOCAL_exampleInit ()
{  
    int i;

    prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pruDataMem);
    pruDataMem_byte = (unsigned char*) pruDataMem;

    pruDataMem_byte[DMX_HALT_ADDR] = 0;
    pruDataMem_byte[DMX_CHANNELS_ADDR] = DMX_CHANNELS;
    pruDataMem_byte[DMX_PIN_ADDR] = DMX_PIN;

    for (i = 0; i < DMX_CHANNELS; i++) {
        pruDataMem_byte[i] = 0;
    }

    return(0);
}

static void LOCAL_export_pin (int pin) {
	FILE *file;
	char dir_file_name[50];

	// Export the GPIO pin
	file = fopen("/sys/class/gpio/export", "w");
	fprintf(file, "%d", pin);
	fclose(file);

	// Let GPIO know what direction we are writing
	sprintf(dir_file_name, "/sys/class/gpio/gpio%d/direction", pin);
	file = fopen(dir_file_name, "w");
	fprintf(file, "out");
	fclose(file);
}

static void LOCAL_unexport_pin (int pin) {
	FILE *file;
	file = fopen("/sys/class/gpio/unexport", "w");
	fwrite(&pin, 4, 1, file);
	fclose(file);
}

// From http://www.abc.se/~m6695/udp.html
static void LOCAL_udp_listen () {
	struct sockaddr_in si_me, si_other;
	int s, i, idx, slen=sizeof(si_other);
	char buf[UDP_BUFLEN];
	int channel, channels, value;
	int packet_length;

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("socket");

	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(UDP_PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s, &si_me, sizeof(si_me))==-1)
		diep("bind");

	for (i=0; i<UDP_BUFLEN; i++) {
		buf[i] = 0;
	}

	while (1) {
		packet_length = recvfrom(s, buf, UDP_BUFLEN, 0, &si_other, &slen);
		if (packet_length == -1) {
			diep("recvfrom()");
		}
    sscanf(buf, "%d", &channels);

    // server exit condition
    if (channels < 0) {
      break;
    }

    pruDataMem_byte[DMX_CHANNELS_ADDR] = channels+1;
    channel = 0;
		buf[packet_length] = 0;
    idx = 0;
    // skip past the channel definition
    while (++idx < packet_length && buf[idx] != ' ');

    while (idx < packet_length) {
			sscanf(buf+idx, " %d", &value);
      // printf("%3d ", value);
      if (value > 0) printf("%d ", channel);
			pruDataMem_byte[channel++] = value;
      // skip past this integer in the packet
      while (++idx < packet_length && buf[idx] != ' ');
		}
		printf("\n");
 	}

	close(s);
}

