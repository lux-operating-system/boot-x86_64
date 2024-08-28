/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#include <lxboot.h>
#include <lxfs.h>
#include <stdio.h>
#include <string.h>

#define CONFIG_BUFFER           (char *)0x100000

static BootConfig config;

int loadConfig(const char *path) {
    memset(&config, 0, sizeof(BootConfig));
    if(!lxfsRead(bootInfo.bootDevice, partitionIndex, path, CONFIG_BUFFER)) {
        printf("failed to load /lxboot.conf");
        while(1);
    }

    config.size = lxfsSize(bootInfo.bootDevice, partitionIndex, path);
    if(!config.size) {
        printf("boot configuration file is empty, no boot option available");
        while(1);
    }

    for(size_t i = 0; i < (config.size - 7); i++) {
        if(!memcmp(CONFIG_BUFFER+i, "[entry]", 7)) {
            config.count++;
        }
    }

    printf("config: found %d boot option%s\n", config.count, config.count == 1 ? "" : "s");

    if(!config.count) {
        printf("no boot option available");
        while(1);
    }

    return config.count;
}

static char *skipLine(char *line) {
    while(*line != '\n') line++;
    line++;
    return line;
}

static size_t lineLength(char *line) {
    size_t i = 0;
    while(line[i] && line[i] != '\n') i++;
    return i;
}

static char *copyLine(char *dest, char *line) {
    size_t len = lineLength(line);
    memcpy(dest, line, len);
    dest[len] = 0;  // null terminator
    return dest;
}

static char *copyWord(char *dest, char *line) {
    size_t len = 0;
    while(line[len] && line[len] != '\n' && line[len] != ' ') {
        dest[len] = line[len];
        len++;
    }

    dest[len] = 0;  // null terminator
    return dest;
}

static char *appendLine(char *dest, char *line) {
    return copyLine(dest + strlen(dest), line);
}

BootConfig *selectBootOption(int option) {
    if(option >= config.count) return NULL;
    char line[256];

    // find the boot option in question
    int count = 0;
    size_t i;
    for(i = 0; i < (config.size - 7); i++) {
        if(!memcmp(CONFIG_BUFFER+i, "[entry]", 7)) {
            count++;
            if(count > option) break;
        }
    }

    //printf("config: selected boot option %d\n", option);

    memset(config.disk, 0, CONFIG_MAX_NAME);
    memset(config.name, 0, CONFIG_MAX_NAME);
    memset(config.kernel, 0, CONFIG_MAX_KERNEL);
    memset(config.ramdisk, 0, CONFIG_MAX_KERNEL);
    memset(config.arguments, 0, CONFIG_MAX_ARGUMENTS);
    memset(config.modules, 0, CONFIG_MAX_MODULES);
    config.moduleCount = 0;

    // now parse the boot option
    char *entry = CONFIG_BUFFER+i;
    entry = skipLine(entry);

    while(memcmp(entry, "boot", 4)) {
        if(!memcmp(entry, "name ", 5)) {
            copyLine(config.name, entry + 5);
            //printf("config: OS name is '%s'\n", config.name);
        } else if(!memcmp(entry, "disk ", 5)) {
            copyLine(config.disk, entry + 5);
            //printf("config: booting from disk '%s'\n", config.disk);
        } else if(!memcmp(entry, "kernel ", 7)) {
            copyWord(config.kernel, entry + 7);
            //printf("config: booting kernel '%s'\n", config.kernel);

            if(entry[7 + strlen(config.kernel)] == ' ') {
                copyLine(config.arguments, entry + 8 + strlen(config.kernel));
                //printf("config: kernel arguments '%s'\n", config.arguments);
            }
        } else if(!memcmp(entry, "ramdisk ", 8)) {
            copyLine(config.ramdisk, entry + 8);
            //printf("config: using ramdisk '%s'\n", config.ramdisk);
        } else if(!memcmp(entry, "module ", 7)) {
            appendLine(config.modules, entry + 7);
            appendLine(config.modules, " ");
            //printf("config: kernel boot module '%s'\n", copyLine(line, entry + 7));
            config.moduleCount++;
        } else {
            printf("config: undefined command '%s', aborting\n", copyLine(line, entry));
            while(1);
        }

        entry = skipLine(entry);
    }

    // verify boot option
    if(!strlen(config.kernel) || !strlen(config.disk)) {
        printf("boot option does not specify kernel or boot device\n");
        while(1);
    }

    return &config;
}