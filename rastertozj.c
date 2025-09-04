#include <cups/cups.h>
#include <cups/raster.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

#define DEBUGFILE "/tmp/debugraster.txt"

struct settings_ {
    int cashDrawer1;
    int cashDrawer2;
    int blankSpace;
    int feedDist;
};
struct settings_ settings;

struct command {
    int length;
    char* command;
};

static const struct command printerInitializeCommand = {2, (char[2]){0x1b, 0x40}};
static const struct command cashDrawerEject[2] = {
    {5, (char[5]){0x1b, 0x70, 0, 0x40, 0x50}},
    {5, (char[5]){0x1b, 0x70, 1, 0x40, 0x50}}
};
static const struct command rasterModeStartCommand = {4, (char[4]){0x1d, 0x76, 0x30, 0}};
static const struct command pageCutCommand = {4, (char[4]){0x1d, 'V', 'A', 20}};

void mputchar(char c) {
    putchar((unsigned char)c);
}

void outputarray(const char * array, int length) {
    for (int i = 0; i < length; ++i)
        mputchar(array[i]);
}

void outputCommand(struct command output) {
    outputarray(output.command, output.length);
}

int lo(int val) { return val & 0xFF; }
int hi(int val) { return lo(val >> 8); }

void rasterheader(int xsize, int ysize) {
    outputCommand(rasterModeStartCommand);
    mputchar(lo(xsize));
    mputchar(hi(xsize));
    mputchar(lo(ysize));
    mputchar(hi(ysize));
}

void skiplines(int size) {
    mputchar(0x1b);
    mputchar(0x4a);
    mputchar(size);
}

int getOptionInt(const char *name, cups_dest_t *dest) {
    const char *value = cupsGetOption(name, dest->num_options, dest->options);
    return value ? atoi(value) : 0;
}

void initializeSettings(char *job_options) {
    cups_dest_t *dests, *dest = NULL;
    cups_dinfo_t *info = NULL;
    int num_dests = cupsGetDests(&dests);

    if (num_dests == 0 || dests == NULL) {
        fputs("ERROR: No printer destinations found\n", stderr);
        return;
    }

    dest = cupsGetDest(NULL, NULL, num_dests, dests);
    if (!dest) {
        fputs("ERROR: No default printer found\n", stderr);
        cupsFreeDests(num_dests, dests);
        return;
    }

    info = cupsCopyDestInfo(CUPS_HTTP_DEFAULT, dest);
    if (!info) {
        fputs("ERROR: Unable to get printer info\n", stderr);
        cupsFreeDests(num_dests, dests);
        return;
    }

    cups_option_t *parsed_options = NULL;
    int parsed_count = cupsParseOptions(job_options, 0, &parsed_options);

    if (parsed_count > 0) {
        dest->options = parsed_options;
        dest->num_options = parsed_count;
    }

    memset(&settings, 0x00, sizeof(struct settings_));
    settings.cashDrawer1 = getOptionInt("CashDrawer1Setting", dest);
    settings.cashDrawer2 = getOptionInt("CashDrawer2Setting", dest);
    settings.blankSpace  = getOptionInt("BlankSpace", dest);
    settings.feedDist    = getOptionInt("FeedDist", dest);

    cupsFreeDestInfo(info);
    cupsFreeDests(num_dests, dests);
    if (parsed_options) cupsFreeOptions(parsed_count, parsed_options);
}

void jobSetup() {
    if (settings.cashDrawer1 == 1)
        outputCommand(cashDrawerEject[0]);
    if (settings.cashDrawer2 == 1)
        outputCommand(cashDrawerEject[1]);
    outputCommand(printerInitializeCommand);
}

void ShutDown() {
    if (settings.cashDrawer1 == 2)
        outputCommand(cashDrawerEject[0]);
    if (settings.cashDrawer2 == 2)
        outputCommand(cashDrawerEject[1]);
    outputCommand(pageCutCommand);
    outputCommand(printerInitializeCommand);
}

void (*old_signal)(int);

void EndPage() {
    for (int i = 0; i < settings.feedDist; ++i)
        skiplines(0x18);
    signal(SIGTERM, old_signal);
}

void cancelJob(int foo) {
    for (int i = 0; i < 0x258; ++i)
        mputchar(0);
    EndPage();
    ShutDown();
}

void pageSetup() {
    old_signal = signal(SIGTERM, cancelJob);
}

int main(int argc, char *argv[]) {
    int fd = 0;
    cups_raster_t * ras = NULL;
    cups_page_header2_t header;
    int page = 0;
    int y = 0;

    unsigned char * rasterData = NULL;
    unsigned char * originalRasterDataPtr = NULL;

    if (argc < 6 || argc > 7) {
        fputs("ERROR: rastertozj job-id user title copies options [file]\n", stderr);
        return EXIT_FAILURE;
    }

    if (argc == 7) {
        if ((fd = open(argv[6], O_RDONLY)) == -1) {
            perror("ERROR: Unable to open raster file");
            return EXIT_FAILURE;
        }
    } else {
        fd = 0;
    }

    initializeSettings(argv[5]);
    jobSetup();
    ras = cupsRasterOpen(fd, CUPS_RASTER_READ);
    page = 0;

    while (cupsRasterReadHeader2(ras, &header)) {
        if (header.cupsHeight == 0 || header.cupsBytesPerLine == 0)
            break;

        if (rasterData == NULL) {
            rasterData = malloc(header.cupsBytesPerLine * 24);
            if (rasterData == NULL) {
                if (originalRasterDataPtr != NULL) free(originalRasterDataPtr);
                cupsRasterClose(ras);
                if (fd != 0) close(fd);
                return EXIT_FAILURE;
            }
            originalRasterDataPtr = rasterData;
        }

        fprintf(stderr, "PAGE: %d %d\n", ++page, header.NumCopies);
        pageSetup();

        int adjustedWidth = (header.cupsWidth > 0x240) ? 0x240 : header.cupsWidth;
        adjustedWidth = (adjustedWidth + 7) & 0xFFFFFFF8;
        int width_size = adjustedWidth >> 3;

        y = 0;
        int zeroy = 0;

        while (y < header.cupsHeight) {
            if ((y & 127) != 0)
                fprintf(stderr, "INFO: Printing page %d, %d%% complete...\n", page, (100 * y / header.cupsHeight));

            int rest = header.cupsHeight - y;
            if (rest > 24) rest = 24;
            y += rest;

            if (y) {
                unsigned char * buf = rasterData;
                int j;
                for (j = 0; j < rest; ++j) {
                    if (!cupsRasterReadPixels(ras, buf, header.cupsBytesPerLine))
                        break;
                    buf += width_size;
                }
                if (j < rest) continue;
            }

            int rest_bytes = rest * width_size;
            int j;
            for (j = 0; j < rest_bytes && !rasterData[j]; j++);

            if (j >= rest_bytes) {
                ++zeroy;
                continue;
            }

            for (; zeroy > 0; --zeroy)
                skiplines(24);

            rasterheader(width_size, rest);
            outputarray((char*)rasterData, rest_bytes);
            skiplines(0);
        }

        if (!settings.blankSpace)
            for (; zeroy > 0; --zeroy)
                skiplines(24);

        EndPage();
    }

    ShutDown();

    if (originalRasterDataPtr != NULL) free(originalRasterDataPtr);
    cupsRasterClose(ras);
    if (fd != 0) close(fd);

    if (page == 0)
        fputs("ERROR: No pages found!\n", stderr);
    else
        fputs("INFO: Ready to print.\n", stderr);

    return (page == 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
