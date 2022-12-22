#include <stdio.h>
#include <string.h>
#include <signal.h>

void put_arg(int arg)
{
    FILE *file = fopen("/sys/kernel/debug/lkm_lab/arg", "w");
    fprintf(file, "%d", arg);
    fclose(file);
}

int str_to_uint(char *line)
{
	int res = 0;
	for (size_t i = 0; i < strlen(line); i++, res *= 10)
		if (line[i] < '0' || line[i] > '9') return 0;
		else res += (line[i] - '0');
	return res / 10;
}

void read_info()
{
    int pid = 0, priority = 0, state = 0;
    char processed = 'N', buffer[32];

    do {
        FILE *file = fopen("/sys/kernel/debug/lkm_lab/processed", "r");
        fscanf(file, "%c", &processed);
        fclose(file);
    } while (processed == 'N');

    FILE *file = fopen("/sys/kernel/debug/lkm_lab/ts_pid_node", "r");
    fscanf(file, "%d", &pid);
    fclose(file);

    file = fopen("/sys/kernel/debug/lkm_lab/ts_prio_node", "r");
    fscanf(file, "%d", &priority);
    fclose(file);

    file = fopen("/sys/kernel/debug/lkm_lab/ts_state_node", "r");
    fscanf(file, "%d", &state);
    fclose(file);

    printf("task_struct {\n\tpid = %d,\n\tpriority = %d,\n\tstate = %d\n}\n", pid, priority, state);
    printf("\n\nx86 32-bit registers:\n");

    file = fopen("/sys/kernel/debug/lkm_lab/regset", "r");

    if (file == NULL) return;
    else {
        memset(buffer, 0, 32);
        while (fgets(buffer, 20,file) != NULL)
            printf("%s", buffer);
        fclose(file);
    }
}

int main(int argc, char **argv) 
{
    int pid;

	if (argc == 1) {
		printf("No argument found\n");
		return 0;
	}

    pid = str_to_uint(argv[1]);

    if (kill(pid, 0) != 0 || pid == 0) {
        printf("Invalid pid\n");
        return -1;
    }

	put_arg(pid);
    read_info();

    FILE *file = fopen("/sys/kernel/debug/lkm_lab/processed", "w");
    fprintf(file, "0");
    fclose(file);

	return 0;
}
