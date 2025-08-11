#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>

void freq() {
    int core = 0;
    char path[128], buf[128];
    FILE *fp;
    while (1) {
        snprintf(path, sizeof(path),
                 "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", core);
        fp = fopen(path, "r");
        if (!fp) break;
        if (fgets(buf, sizeof(buf), fp)) {
            int mhz = atoi(buf) / 1000;
            printf("cpu%d: %d MHz\n", core, mhz);
        }
        fclose(fp);
        core++;
    }

    if (core == 0)
        fprintf(stderr, "error reading CPU frequency.\n");
}

void model() {
    FILE *fp;
    char line[256];
    size_t len;
    fp = popen("getprop ro.soc.model", "r");
    if (fp) {
        if (fgets(line, sizeof(line), fp)) {
            len = strlen(line);
            while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
                line[--len] = '\0';
            if (line[0] != '\0') {
                printf("model: %s\n", line);
                pclose(fp);
                return;
            }
        }
        pclose(fp);
    }
    fp = fopen("/proc/cpuinfo", "r");
    if (!fp) {
        perror("fopen");
        return;
    }
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "model name", 10) == 0 ||
            strncmp(line, "Hardware", 8) == 0 ||
            strncmp(line, "Processor", 9) == 0) {
            char *colon = strchr(line, ':');
            if (colon) {
                char *name = colon + 1;
                while (*name == ' ' || *name == '\t') name++;
                len = strlen(name);
                while (len > 0 && (name[len - 1] == '\n' || name[len - 1] == '\r'))
                    name[--len] = '\0';
                printf("model: %s\n", name);
                break;
            }
        }
    }
    fclose(fp);
}



void core() {
    int count = 0;
    DIR *cpu_dir = opendir("/sys/devices/system/cpu");
    if (!cpu_dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(cpu_dir)) != NULL) {
        if (strncmp(entry->d_name, "cpu", 3) == 0 && isdigit(entry->d_name[3])) {
            count++;
        }
    }

    closedir(cpu_dir);
    printf("cores: %d\n", count);
}

void usage(const char *prog) {
    fprintf(stderr,
        "usage: xcpu [option]\n"
        "  -f   show CPU frequency\n"
        "  -m   show CPU model\n"
        "  -c   show available CPU cores\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-f") == 0) {
        freq();
    } else if (strcmp(argv[1], "-m") == 0) {
        model();
    } else if (strcmp(argv[1], "-c") == 0) {
        core();
    } else {
        usage(argv[0]);
        return 1;
    }

    return 0;
}