# ethernet_watchdog
Simple Ethernet watchdog test the network carrier if there is not response it pull down and up the network.

This simple ethernet watchdog send the ping at certain interval that is configured via Macro's to a host and wait for the reply.
Sleep time also configured via Macro's in metric, seconds.

How to compile this program?
----------------------------

Use GCC and its POSIX Compatible not portable in Windows.

Usage of application:
---------------------

./a.out: option requires an argument -- 'h'
Example: ./a.out -n /sys/class/net/eth0/carrierUsage: ./a.out progname [options] [arguments...]
Commands: -n |	--ethernet                      - Ethernet network interface name
		--version                       - Display version
          -h |	--help                          - Display this help text
