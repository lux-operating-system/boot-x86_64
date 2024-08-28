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
