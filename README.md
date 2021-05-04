## Summary

A portable, free and open source command-line network bandwidth tester tool.


## Description

The uCodev Bandwidth Tester (ubwt for short) is a command-line tool designed to test network bandwidth with unidirectional and bidirectional testing, providing basic or, optionally, extended reports.

It consists of a single tool, that can be executed as a listener on one end, and as connector on the another end - it is not designed to be used as a running service. It is a testing tool, designed to be used with both endpoints under full control of the tester.


## Licensing

GNU GENERAL PUBLIC LICENSE

Version 3, 29 June 2007

See [LICENSE](https://github.com/ucodev/ubwt/blob/main/doc/text/LICENSE)


## Portability

ubwt is designed to be compliant with any POSIX-based operating system. It was successfully tested under **GNU Linux**, **GNU Hurd**, **FreeBSD**, **OpenBSD**, **SunOS**, **Minix** and **Darwin**. Microsoft Windows operating systems are not officially supported by ubwt - but they will, in a near future.

Also, ubwt is architecture independent - it was also successfully tested under **x86/64**, **ARM**, **MIPS (mips/mipsel)** and **SPARC**.


## Stability

DISCLAIMER: This is free software; see the source for copying conditions. There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

ubwt is currently in its early stages of development - there isn't a version that can be considered "stable".


## Installation

Install ubwt with the following commands:

      ~# cd /usr/src
      ~# git clone https://github.com/ucodev/ubwt
      ~# cd ubwt
      ~# ./deploy

or see [INSTALL](https://github.com/ucodev/ubwt/blob/main/doc/text/INSTALL)


## Command-Line Usage Examples

IMPORTANT NOTE: When using ubwt, both listener and connector mode MUST have the same command-line options - by failing to do so, the behaviour of the application is undefined.

### Unidirectional Test - Short Summary

      |     im@listening ~ $ ubwt listener 10.10.10.1
      |      [*] Initializing local regular workers...
      |      [*] Waiting for remote regular workers...
      |      [*] Running bandwidth test...
      |
      |          Latency : 1.128 ms
      |         Download : 943.485 Mbps
      |     im@listening ~ $ 

      |     im@connector ~ $ ubwt connector 10.10.10.1
      |      [*] Initializing local regular workers...
      |      [*] Waiting for remote regular workers...
      |      [*] Running bandwidth test...
      |
      |          Latency : 1.128 ms
      |           Upload : 943.485 Mbps
      |     im@connector ~ $ 

### Bidirectional Test - Short Summary

      |     im@listening ~ $ ubwt -b listener 10.10.10.1
      |      [*] Initializing local regular workers...
      |      [*] Waiting for remote regular workers...
      |      [*] Running bandwidth test...
      |
      |          Latency : 1.351 ms
      |         Download : 968.273 Mbps
      |
      |          Latency : 1.287 ms
      |           Upload : 940.265 Mbps
      |     im@listening ~ $ 

      |     im@connector ~ $ ubwt -b connector 10.10.10.1
      |      [*] Initializing local regular workers...
      |      [*] Waiting for remote regular workers...
      |      [*] Running bandwidth test...
      |
      |          Latency : 1.351 ms
      |           Upload : 968.273 Mbps
      |
      |          Latency : 1.287 ms
      |         Download : 940.265 Mbps
      |     im@connector ~ $ 
 
### Bidirectional Test - Full Report

      |     im@listening ~ $ ubwt -F -b listener 10.10.10.1
      |      [*] Initializing local regular workers...
      |      [*] Waiting for remote regular workers...
      |      [*] Running bandwidth test...
      |
      |     Direction                           : Download
      |     MTU                                 : 1500 octets
      |     L3 Protocol                         : ipv4
      |     L4 Protocol                         : tcp
      |     Requested L4 payload size           : 1432 octets
      |     Estimated headers size              : 66 octets
      |     Transmission time                   : 12.3916 s
      |     Total L4 packets expected           : 1000000
      |     Total L4 packets transfered         : 1000000
      |     Total L4 octets transfered          : 1432000000
      |     Estimated L3 packets transfered     : 1000000
      |     Estimated L3 fragmentation ratio    : 1:1
      |     Estimated L2 octets transfered      : 1498000000
      |     Latency (L4 RTA)                    : 0.950 ms
      |     Effective packet loss               : 0.0000 (%)
      |     Theoretical L1 Bandwidth            : 1000 Mbps
      |     Estimated L2 Bandwidth              : 967.106 Mbps
      |     Effective L4 Bandwidth              : 924.496 Mbps
      |
      |     Direction                           : Upload
      |     MTU                                 : 1500 octets
      |     L3 Protocol                         : ipv4
      |     L4 Protocol                         : tcp
      |     Requested L4 payload size           : 1432 octets
      |     Estimated headers size              : 66 octets
      |     Transmission time                   : 12.7196 s
      |     Total L4 packets expected           : 1000000
      |     Total L4 packets transfered         : 1000000
      |     Total L4 octets transfered          : 1432000000
      |     Estimated L3 packets transfered     : 1000000
      |     Estimated L3 fragmentation ratio    : 1:1
      |     Estimated L2 octets transfered      : 1498000000
      |     Latency (L4 RTA)                    : 1.220 ms
      |     Effective packet loss               : 0.0000 (%)
      |     Theoretical L1 Bandwidth            : 1000 Mbps
      |     Estimated L2 Bandwidth              : 942.164 Mbps
      |     Effective L4 Bandwidth              : 900.654 Mbps
      |     im@listening ~ $ 

