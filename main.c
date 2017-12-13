#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

#define TOMATO_TIME_MINUTES 25
#define VACATION_TIME_MINUTES 5
#define TOMATO_TIME_SECONDS 60 * TOMATO_TIME_MINUTES
#define VACATION_TIME_SECONDS  60 * VACATION_TIME_MINUTES
#define FILENAME "tomato_stats.txt"

struct tomato_stats {
	uint64_t tomato_total;
	uint64_t vacation_total;
};
enum mode {tomato, vacation, reset, quit};
enum mode read_mode();
void print_time(int);
void read_file(char*, struct tomato_stats*);
void update_file(char*, struct tomato_stats*);
void libnotify_notify(enum mode);

int main(){
	struct tomato_stats stats;
	enum mode mode_next;
	time_t next_alarm;
	time_t now;

	read_file(FILENAME, &stats);
	
	do {
		if(stats.tomato_total > 0){
			printf("The total time spent studying is: %" PRIu64 " minutes\n", stats.tomato_total);
		}
		if(stats.vacation_total > 0){
			printf("The total time spent on break is: %" PRIu64 " minutes\n", stats.vacation_total);
		}
		mode_next = read_mode();
		switch(mode_next){
			case tomato:
				now = time(0);
				next_alarm = now + TOMATO_TIME_SECONDS;
				double timediff;
				do{
					timediff = difftime(next_alarm, now);
					print_time((int)timediff);
					sleep(5);
					now = time(0);
				} while(timediff > 0);
				stats.tomato_total += TOMATO_TIME_MINUTES;
				libnotify_notify(mode_next);
				update_file(FILENAME, &stats);
				break;
			case vacation:
				now = time(0);
				next_alarm = now + VACATION_TIME_SECONDS;
				do{
					timediff = difftime(next_alarm, now);
					print_time((int)timediff);
					sleep(5);
					now = time(0);
				} while(timediff > 0);
				stats.vacation_total += VACATION_TIME_MINUTES;
				libnotify_notify(mode_next);
				update_file(FILENAME, &stats);
				break;
			case reset:
				stats.tomato_total = 0;
				stats.vacation_total = 0;
				update_file(FILENAME, &stats);
				break;
			default:
				break;
		}
	} while (mode_next != quit);
}

enum mode read_mode(){
	printf("p = pomodoro, b = break, r = reset statistics, q = quit\n");
	printf("Choose what to do next: ");
	int c = getchar();
	while (getchar() != '\n'){
	}
	switch(c){
		case 'p':
		case 'P':
			return tomato;
			break;
		case 'b':
		case 'B':
			return vacation;
			break;
		case 'r':
		case 'R':
			return reset;
			break;
		case 'q':
		case 'Q':
		default:
			return quit;
			break;
	}
}

void libnotify_notify(enum mode mode){
	if(system("which notify-send >/dev/null")){
		return;
	}
	switch(mode){
		case tomato:
			system("notify-send 'tomato' 'pomodoro time has elapsed'");
			break;
		case vacation:
			system("notify-send 'tomato' 'break time has elapsed'");
			break;
		default:
			break;
	}
}

void print_time(int timediff){
	if(timediff >= 60){
		int min = timediff / 60;
		int sec = timediff % 60;
		if(sec == 0){
			printf("\33[A\33[2K\%d minute(s) remaining...\n", min);
		} else {
			printf("\33[A\33[2K\%d minute(s), %d second(s) remaining...\n", min, sec);
		}
	} else {
		printf("\33[A\33[2K\%d second(s) remaining...\n", timediff);
	}
}

void update_file(char *name, struct tomato_stats *stats){
	FILE *f = fopen(name, "w");
	if(!f){
		return;
	}
	fprintf(f, "%" PRIu64 "\n", stats->tomato_total);
	fprintf(f, "%" PRIu64 "\n", stats->vacation_total);
	fclose(f);
}

void read_file(char *name, struct tomato_stats *stats){
	FILE *f = fopen(name, "r");
	if(!f){
		stats->tomato_total = 0;
		stats->vacation_total = 0;
		return;
	}
	fscanf(f, "%" SCNu64 "\n", &stats->tomato_total);
	fscanf(f, "%" SCNu64 "\n", &stats->vacation_total);
	fclose(f);
}
