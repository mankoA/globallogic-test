#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <argp.h>
#include <stdbool.h>

#include "ntp_handler.c"
#include "waiter.c"

static char doc[] = "Time synchronizer.";
static char args_doc[] = "[time_synchronizer.c]...";
static struct argp_option options[] = { 
    { "synchronize", 's', 0, 0, "System time synchronization with the NTP server or wait until it."},
    { "wait", 'w', 0, 0, "Wait for some time."},
    { "diff", 'd', 0, 0, "Difference between the NTP server time and the system one."},
    
    //additional
    { "hours", 'h', "Numbah", 0, "Hours to wait."},
    { "minutes", 'm', "Numbah", 0, "Minutes to wait."},
    { "seconds", 'c', "Numbah", 0, "Seconds to wait."},
    { 0 } 
};

struct arguments {
    enum { SYNCHRONIZE_MODE, WAIT_MODE, DIFF_MODE } mode;
    int hours;
    int minutes ;
    int seconds;

};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
      case 's': arguments->mode = SYNCHRONIZE_MODE; break;
      case 'w': arguments->mode = WAIT_MODE; break;
      case 'd': arguments->mode = DIFF_MODE; break;
      case 'h': 
        arguments->hours = (int) strtol(arg, NULL, 10); break;
      case 'm': 
        arguments->minutes= (int) strtol(arg, NULL, 10); break;
      case 'c': 
        arguments->seconds = (int) strtol(arg, NULL, 10); break;
      case ARGP_KEY_ARG: return 0;
      default: return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int set_system_time(time_t current_system_time)
{
    struct timeval now;
    int return_value;
    long int t = (long int) current_system_time;
    now.tv_sec=(uint32_t) t;
    now.tv_usec=0;

    return_value = settimeofday(&now, NULL);
    return return_value;
}


int get_current_time(time_t *current_system_time){
    time ( current_system_time );
    return 0;
}

void print_time(char * description, time_t *current_system_time){
  char string_time[100];
  struct tm * timeinfo;
  timeinfo = localtime ( current_system_time );
  strftime(string_time, 100, "%Y-%m-%d %H:%M:%S.000", timeinfo);
  printf("%s time and date: %s\n", description, string_time);
}

void convert_time_to_seconds(uint16_t hh, uint16_t mm, uint16_t ss, uint32_t* wait_secs){
   *wait_secs = hh*3600 + mm*60 + ss;
   printf("%d seconds = %d hours : %d minutes : %d seconds\n", *wait_secs, hh, mm, ss);
}

void wait_for_seconds(struct arguments arguments){
  uint32_t wait_secs = 0;
  convert_time_to_seconds(arguments.hours, arguments.minutes, arguments.seconds, &wait_secs);

  if (wait_secs > 0)
    wait_for(wait_secs);
}


int time_synchronize(){
  time_t current_system_time, ntp_server_time;
  int return_value;
  return_value = get_current_time(&current_system_time); 
  return_value = receive_ntp_server_time(&ntp_server_time);  

  print_time("System time", &current_system_time);
  print_time("NTP server", &ntp_server_time);

  return_value = set_system_time(ntp_server_time);
  if (return_value != 0)
    error("ERROR: system time is not set");
  else
    printf("Time is set\n");

  return_value = get_current_time(&current_system_time); 
  print_time("New system time", &current_system_time);
  return 0;
};

int time_unsynchronize(){
  return 0;
}

int time_difference(){
  time_t current_system_time, ntp_server_time;
  int return_value;
  return_value = get_current_time(&current_system_time); 
  return_value = receive_ntp_server_time(&ntp_server_time);  

  print_time("System time", &current_system_time);
  print_time("NTP server", &ntp_server_time);

  uint16_t delay = difftime(current_system_time, ntp_server_time);
  printf("System delay: %d\n", delay);
  return 0;
}

int main( int argc, char * argv [] ) 
{

  struct arguments arguments = {DIFF_MODE, 0, 0, 0};
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  printf("Arguments mode: %d, wait until: %d hours %d minutes %d seconds\n\n", arguments.mode,
         arguments.hours, arguments.minutes, arguments.seconds);

  time_t current_system_time, ntp_server_time;
  int return_value;
  return_value = get_current_time(&current_system_time); 
  print_time("System time", &ntp_server_time);

  switch(arguments.mode){
      case 0: // SYNCHRONIZE_MODE
      wait_for_seconds(arguments);
      time_synchronize();
      break;
      case 1: // WAIT_MODE
      wait_for_seconds(arguments);
      break;
      case 2: // DIFF_MODE
      time_difference();
      break;
  }

  return 0;
}
