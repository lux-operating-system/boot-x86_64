/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

/* Read-only Minimalist LXFS Implementation */

#include <lxboot.h>
#include <lxfs.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/* helper functions */
int countPath(const char *p) {
    int c = 0;
    for(int i = 0; i < strlen(p); i++) {
        if(p[i] == '/') {
            c++;
        }
    }

    if(strlen(p) > 1 && p[strlen(p)-1] != '/') {
        return c+1;
    } else {
        return c;
    }
}

char *splitPath(char *dst, const char *p, int index) {
    if(!index) {
        return strcpy(dst, p);
    }

    int current = 0, start = 0, len = 0;

    for(int i = 0; i < strlen(p); i++) {
        if(p[i] == '/') {
            current++;

            if(current == index) start = i+1;
            else if(current == index+1) len = i-start;
        }
    }

    //printf("splitpath: current %d, start %d, len %d\n", current, start, len);

    if(!start) {
        return strcpy(dst, p);
    }

    if(!len) {
        len = strlen(p) - start;
    }

    memcpy(dst, p+start, len);
    dst[len] = 0;   // null terminator

    return dst;
}

bool lxfsFindPath(uint8_t disk, int partition, const char *path, LXFSDirectoryEntry *dst) {
    int pathEntries = countPath(path);
    printf("lxfs: attempt to find entry for path %s with %d entries\n", path, pathEntries);

    if(pathEntries <= 1) {
        // root directory
        uint64_t rootBlock = getRootDirectory(disk, partition);
        printf("lxfs: root directory is at block %d\n", rootBlock);
        LXFSDirectoryHeader *rootHeader = (LXFSDirectoryHeader *)LXFS_DIRECTORY_BUFFER;
        readBlock(disk, partition, rootBlock, 1, rootHeader);

        dst->createTime = rootHeader->createTime;
        dst->accessTime = rootHeader->accessTime;
        dst->modTime = rootHeader->modTime;

        dst->flags = LXFS_DIR_VALID;
        dst->flags |= (LXFS_DIR_TYPE_DIR << LXFS_DIR_TYPE_SHIFT);
        dst->owner = LXFS_USER_ROOT;
        dst->group = LXFS_USER_ROOT;
        dst->permissions = LXFS_DEFAULT_PERMS;

        dst->size = rootHeader->sizeEntries;
        dst->entrySize = sizeof(LXFSDirectoryEntry);
        dst->block = rootBlock;

        return true;
    }

    // this is for other non-root directory
    // we alwaysb have to start by finding the root anyway
    LXFSDirectoryEntry *entry = (LXFSDirectoryEntry *)LXFS_DIRECTORY_BUFFER;
    lxfsFindPath(disk, partition, "/", entry);

    bool found = false;
    int pathIndex = 1;
    int blockSizeBytes = getBlockSize(disk, partition) * getSectorSize(disk, partition);

    while(!found) {
        if(((entry->flags >> LXFS_DIR_TYPE_SHIFT) & LXFS_DIR_TYPE_MASK) != LXFS_DIR_TYPE_DIR) {
            printf("lxfs: non-directory cannot have children\n");
            return false;
        }

        uint64_t block = entry->block;
        int directoryIndex;

        splitPath((char *)LXFS_TEXT_BUFFER, path, pathIndex);

        while(block != LXFS_BLOCK_EOF) {
            block = readNextBlock(disk, partition, block, entry);   // one block at a time
            printf("%s\n", (char *)LXFS_TEXT_BUFFER);

            entry = (LXFSDirectoryEntry *)((uint8_t *)LXFS_DIRECTORY_BUFFER + sizeof(LXFSDirectoryHeader));
            directoryIndex = sizeof(LXFSDirectoryHeader);

            while(entry->flags & LXFS_DIR_VALID) {
                //printf("lxfs: searching for %s, found %s\n", (char *)LXFS_TEXT_BUFFER, entry->name);

                if(!strcmp((const char *)LXFS_TEXT_BUFFER, entry->name)) {
                    found = true;
                    break;
                }

                entry = (LXFSDirectoryEntry *)((uint8_t *)entry + entry->entrySize);
                directoryIndex += entry->entrySize;
            }

            // if we get here then we didn't find anything
            if(!found) return false;

            // here we found something, determine if thats the final finding or not
            if(pathIndex == pathEntries-1) {
                // we're done
                printf("lxfs: found entry for %s\n", path);
                memcpy(dst, entry, entry->entrySize);
                return true;
            }

            // we found something, but the hierarchy continues
            found = false;
            pathIndex++;
            if(pathIndex >= pathEntries) {
                return false;
            }
        }
    }

    return false;
}
