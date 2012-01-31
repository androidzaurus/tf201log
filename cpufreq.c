#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct cpucore {
	int id;
	int online;
	int curr_freq;
	int prev_freq;
	char avail_freq[256];
};

void crlf2null(char *s)
{
	int i;
	int len = strlen(s);
	for (i=0 ; i<len ; i++, s++) {
		if (*s == '\n') {
			*s = '\0';
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	const char *path_sysfs         = "/sys/devices/system/cpu";
	const char *path_present       = "/present";
	const char *path_online        = "/online";
	const char *path_offline       = "/offline";
	const char *path_cpus          = "/cpu%d/cpufreq";
	const char *path_curr_cpufreq  = "/scaling_cur_freq";
	const char *path_avail_cpufreq = "/scaling_available_frequencies";

	char path[512];
	char buf[512];
	char *token;
	int fd;
	int id_min, id_max;
	struct cpucore *cpucores;
	struct cpucore *core;

	strcpy(path, path_sysfs);
	strcat(path, path_present);
	fd = open(path, O_RDONLY);
	read(fd, buf, sizeof(buf));
	close(fd);
	sscanf(buf, "%d-%d", &id_min, &id_max);

	cpucores = (struct cpucore *)calloc(id_max - id_min + 1, sizeof(struct cpucore));

	while(1) {
		int i;
		core = cpucores;
		printf("-------------------------------\n");
		for (i=id_min ; i<=id_max ; i++) {
			int khz;
			core->id = i;
			core->online = 0;

			strcpy(path, path_sysfs);
			sprintf(buf, path_cpus, i);
			strcat(path, buf);
			strcat(path, path_curr_cpufreq);
			fd = open(path, O_RDONLY);
			if (fd < 0)
				continue;

			core->online = 1;
			core->prev_freq = core->curr_freq;
			read(fd, buf, sizeof(buf));
			close(fd);
			crlf2null(buf);
			core->curr_freq = atoi(buf);

			if (core->avail_freq[0] == '\0') {
				strcpy(path, path_sysfs);
				sprintf(buf, path_cpus, i);
				strcat(path, buf);
				strcat(path, path_avail_cpufreq);
				fd = open(path, O_RDONLY);
				read(fd, buf, sizeof(buf));
				close(fd);
				crlf2null(buf);
				strcpy(core->avail_freq, buf);
			}
			printf("core %d: %dMHz\n", core->id, core->curr_freq/1000);
			core++;
		}
		sleep(3);
	}

	return 0;
}
