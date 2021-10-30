# Time synchronizer version 1

## Description
C-languge program for:  
+ setting the system time and wait for setting it;  
+ updating the system time accordingly to the NTP server data;  
+ calculating the difference between the system time and the NTP time;  
+ waiting for some time.  


## Before running
Stop automatic time update:
```
sudo timedatectl set-ntp false
```
 
##  How to run
```
gcc synchronizer.c -o synchroniser
sudo ./sycroniser --synchronise --hours=[hours] --minutes=[minutes] --seconds=[seconds]
./sycroniser --wait --hours=[hours] --minutes=[minutes] --seconds=[seconds].
./sycroniser --diff
```

##  MAN
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
