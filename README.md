HyperTransfer
=============

A high performance FTP software & a hobby project of mine.

Features & Known Bugs
=============
1) Half Synchronize Half Asynchronize architecture, fully utilizations of your CPU, networking bandwidth and disk
bandwidth  

2) PASV mode implemented only, sorry for that. Besides, active mode has bad impacts on firewall configuration

3) Crush sometimes due to file description serial number reuse of Linux kernel

How to Build This
=============
Following steps are tested in Ubuntu 12.04 LTS, modify them according to your development environments

Dependency:
1.libevent 1.4.14b stable
./configure --prefix=/opt/libevent-static --enable-static --disable-shared

2.log4cplus 1.1.1
./configure --prefix=/opt/log4cplus --enable-static --disable-shared

3.libssl-dev
./sudo aptitude install libssl-dev

4.cmake
./sudo aptitude install cmake


Build:
1. Change to HyperTransfer project directory
2. Run cmake -i
3. Run make

Configure It & Run
=============
Read the ftp.conf in conf dir, and logs are stored in log dir.

1. Modify the ftp.conf according to your environment
2. Run ./hyper_transfer_server
