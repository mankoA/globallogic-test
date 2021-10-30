# Time synchronizer version 3

## Description
Systemctl sevice for:  
+ setting the system time and wait for setting it;  
+ updating the system time accordingly to the NTP server data;  
+ calculating the difference between the system time and the NTP time;  
+ waiting for some time.  


## Before running
Stop automatic time update:
```
sudo timedatectl set-ntp false
```
 
##  How to install the service
Run the following bash scrypt:
```
sudo bash create_service.sh
```
In order to change the service program parameters, go to `/etc/.synchronizer_conf`.
In order to change the service running parameters, go to `/etc/systemd/system/synchronizer.service`.

## How to use
Status:
```
systemctl status synchronizer.service
```
```
sudo journalctl -f -u synchronizer
```

##  MAN for program parameters
```
                                                                                                                                                                                  
Usage: synchroniser [OPTION...] [synchroniser.c]...
Time synchronizer.

  -s, --synchronize          System time synchronization with the NTP server or
  			      wait until it.
  -d, --diff                 Difference between the NTP server time and the
                             system one.
  -w, --wait                 Wait for some time.
  
  -c, --seconds	      Seconds to wait.

  -h, --hours	              Hours to wait.
  -m, --minutes 	      Minutes to wait.

                             
  -?, --help                 Give this help list
      --usage                Give a short usage message

```
