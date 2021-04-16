## Summary

A portable, free and open source command-line network bandwidth tester tool.


## Description

The uCodev Bandwidth Tester (ubwt for short) is a command-line tool designed to test network bandwidth with unidirectional and bidirectional testing, providing basic or, optionally, extended reports.


## Licensing

GNU GENERAL PUBLIC LICENSE

Version 3, 29 June 2007

See [LICENSE](https://github.com/ucodev/ubwt/blob/main/doc/text/LICENSE)


## Portability

ubwt is designed to be compliant with any POSIX-based operating system. It was successfully tested under **Linux**, **FreeBSD**, **OpenBSD**, **SunOS**, **Minix** and **Darwin**. Microsoft Windows operating systems are not officially supported by ubwt - but they will, in a near future.

Also, ubwt is architecture independent - it was also successfully tested under **x86/64**, **ARM**, **MIPS** and **SPARC**.


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

      Unidirectional test :      im@listening ~ $ ubwt listener myaddress.local
                          :  im_the@connector ~ $ ubwt connector listening.local

      Bidirectional test  :  term_1@localhost ~ $ ubwt -b listener 127.0.0.1
                          :  term_2@localhost ~ $ ubwt -b connector 127.0.0.1

