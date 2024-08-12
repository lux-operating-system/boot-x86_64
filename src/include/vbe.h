/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#pragma once

#include <stdint.h>

/* this structure is used by the controller to identify itself */
typedef struct {
    char signature[4];          // "VBE2" on call, "VESA" on return
    uint16_t version;
    uint16_t oemOffset;
    uint16_t oemSegment;
    uint32_t capabilities;
    uint16_t modeOffset;
    uint16_t modeSegment;
    uint16_t memory;            // in 64 KiB blocks
    uint8_t reserved[492];
} __attribute__((packed)) VBEController;

/* this structure identifies video modes supported by the controller */
typedef struct {
    uint16_t attributes;        // we don't care abt anything here other than linearity
    uint8_t window[2];          // all these are deprecated
    uint16_t granularity;
    uint16_t windowSize;
    uint16_t segment[2];
    uint32_t bankSwitch;

    uint16_t pitch;             // size of one horizontal line in bytes
    uint16_t width;
    uint16_t height;
    uint8_t xChar;              // deprecated
    uint8_t yChar;              // --
    uint8_t planes;             // --
    uint8_t bpp;
    uint8_t bankCount;
    uint8_t memoryModel;
    uint8_t bankSize;
    uint8_t imagePages;
    uint8_t reserved0;

    uint8_t redMask;
    uint8_t redPosition;
    uint8_t greenMask;
    uint8_t greenPosition;
    uint8_t blueMask;
    uint8_t bluePosition;
    uint8_t reservedMask;
    uint8_t reservedPosition;
    uint8_t directColorAttributes;

    uint32_t framebuffer;       // physical address
    uint32_t offScreenBuffer;
    uint16_t offScreenBufferSize;

    uint8_t reserved1[206];
} __attribute__((packed)) VBEMode;

/* these structures identify the display itself, not the display controller */
typedef struct {
    uint8_t hFrequency;
    uint8_t vFrequency;
    uint8_t hActiveLow;
    uint8_t hBlankLow;
    uint8_t hActiveBlankHigh;
    uint8_t vActiveLow;
    uint8_t vBlankLow;
    uint8_t vActiveBlankHigh;

    uint8_t hSync;
    uint8_t hSyncPulse;
    uint8_t vSync;
    uint8_t vhSyncPulse;

    uint8_t hSizeMm;
    uint8_t vSizeMm;
    uint8_t aspectRatio;
    uint8_t hBorder;
    uint8_t vBorder;

    uint8_t displayType;
} __attribute__((packed)) VBEEDIDTiming;

typedef struct {
    uint8_t padding[8];
    uint16_t manufacturer;
    uint16_t id;
    uint32_t serial;
    uint8_t manufactureWeek;
    uint8_t manufactureYear;

    uint8_t version;
    uint8_t revision;

    uint8_t inputType;
    uint8_t horizontalSizeCm;
    uint8_t verticalSizeCm;
    uint8_t gammaFactor;
    uint8_t DPMSflags;
    uint8_t chroma[10];

    uint8_t estTiming1;
    uint8_t estTiming2;
    uint8_t resTiming;
    uint16_t stdTiming[8];

    VBEEDIDTiming timing[4];

    uint8_t reserved;
    uint8_t checksum;
} __attribute__((packed)) VBEMonitor;