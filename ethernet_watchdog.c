#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <getopt.h>


/* Configuration Parameters */
#define MaxInterrupts 10000

/* Time metrics in second */
#define Normal_Sleep 10 
#define First_Time_Sleep 60
#define Second_Time_Sleep 60 * 2
#define High_Time_Sleep 60 * 5

char *eth_wdog = NULL;

int open_watchdog(char *eth_interface) {
	int fd;
	fd = open(eth_interface, O_RDWR | O_NOCTTY);
	if(fd < 0) {
		perror("open(\"/sys/class/net/<interface>/carrier\")");
		exit(1);
	}
	return fd;
}

int read_eth_interupt_counter(int fd) {
	char val[256];
	int rv;
	
	rv = read(fd, val, sizeof(val));
	if( rv <= 0) {
		fprintf(stderr, "Ethernet watchdog failed to get value\n");
		return 0;
	}
	printf("Packets counter: %d\n", atoi(val));
	return atoi(val);
}

void print_usage(char *progname)
{
	printf("Example: ./a.out -n /sys/class/net/<interface>/carrier");
	printf("Usage: %s progname [options] [arguments...]\n"
		   "Commands: -n |	--ethernet                      - Ethernet network interface name\n"
		   "          		--version                       - Display version\n"
		   "          -h |	--help                          - Display this help text\n"
		   "\n", progname);
}

static int help_flag		= 0;
static int version_flag		= 0;
static int eth_address_flag	= 0;
char *version="1.0";

static struct option long_options[] = {
    {"add",    required_argument, 0,  0 },
    {"delete", required_argument, 0,  0 },
    {"help",  no_argument, 0,  0 },
    {"version", no_argument, &version_flag,  1 },
    {"network",  required_argument, 0, 'c'},
    {0, 0, 0, 0 }
};


void ethernet_watchdog(char *eth_interface) {
	int fd = -1;
	int val;
	int sleep_interval = Normal_Sleep;
	int level;

	level = 0;
	
	while (1) {
		switch (level) {
		case 0 : // Normal
			sleep_interval = Normal_Sleep;
			break;
		case 1 : // first time
			sleep_interval = First_Time_Sleep;
			break;
		case 2 : // second time
			sleep_interval = Second_Time_Sleep;
			break;
		case 3 : // high time
			sleep_interval = High_Time_Sleep;
			break;
		default: ;
		}
		
		fd = open_watchdog(eth_interface);
		printf("sleeping: %d\n", sleep_interval);
		sleep(sleep_interval);
		if(fd > 0) {
			val = read_eth_interupt_counter(fd);
		} else {
			continue;
		}

		if(fd > 0) {
			char response[1] = {'0'};
			write(fd, response, sizeof(response));
			close(fd);
			fd = -1;
		}

		if (val >= MaxInterrupts) {
			if (level < 3)
				level++;
		} else {
			/* When the packet storm is over, it brings interface down
			to clear any packets in buffers */
			if(level > 0) {
				printf("Restaring network\n");
				system("ifconfig eth0 down");
				sleep(5);
				system("ifconfig eth0 up");
			}
			level = 0;
		}
	} 
}

int main(int argc, char *argv[]) {
	const char *version="1.0";
	int  c, option_index = 0;

	if (argc < 2) {
		print_usage(argv[0]);
		return 0;
	}

	while (1) {
		c = getopt_long(argc, argv, "h:n:v",long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'n':
			printf("option with Ethernet interface:'%s'\n", optarg);
			eth_wdog = strdup(optarg);
			eth_address_flag = 1;
			break;
		case 'h':
			printf("option a\n");
			break;
		case 'v':
			version_flag = 1;
			break;
		case '?':
			print_usage(argv[0]);
			break;
		}
	}

	if (help_flag) {
		print_usage(argv[0]);
		return 0;
	}

	if (version_flag)
		printf("Ethernet Watchdog version is %s\n",version);

	if (eth_address_flag)
		ethernet_watchdog(eth_wdog);

	return 0;
}
