#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

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

static const struct command printerInitializeCommand = {2,(char[2]){0x1b,0x40}};
static const struct command cashDrawerEject[2] = {
    {5,(char[5]){0x1b,0x70,0,0x40,0x50}},
    {5,(char[5]){0x1b,0x70,1,0x40,0x50}}
};
static const struct command rasterModeStartCommand = {4,(char[4]){0x1d,0x76,0x30,0}};
static const struct command pageCutCommand = {4, (char[4]){29,'V','A',20}};

FILE* lfd = 0;

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

int getOptionChoiceIndex(const char * choiceName, ppd_file_t * ppd) {
    ppd_choice_t * choice;
    ppd_option_t * option;

    choice = ppdFindMarkedChoice(ppd, choiceName);
    if (choice == NULL) {
        if ((option = ppdFindOption(ppd, choiceName)) == NULL) return -1;
        if ((choice = ppdFindChoice(option, option->defchoice)) == NULL) return -1;
    }
    return atoi(choice->choice);
}

void initializeSettings(char * commandLineOptionSettings) {
    ppd_file_t * ppd = NULL;
    cups_option_t * options = NULL;
    int numOptions = 0;

    ppd = ppdOpenFile(getenv("PPD"));
    ppdMarkDefaults(ppd);

    numOptions = cupsParseOptions(commandLineOptionSettings, 0, &options);
    if (numOptions != 0 && options != NULL) {
        cupsMarkOptions(ppd, numOptions, options);
        cupsFreeOptions(numOptions, options);
    }

    memset(&settings, 0x00, sizeof(struct settings_));
    settings.cashDrawer1 = getOptionChoiceIndex("CashDrawer1Setting", ppd);
    settings.cashDrawer2 = getOptionChoiceIndex("CashDrawer2Setting", ppd);
    settings.blankSpace  = getOptionChoiceIndex("BlankSpace", ppd);
    settings.feedDist    = getOptionChoiceIndex("FeedDist", ppd);

    ppdClose(ppd);
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

__sighandler_t old_signal;
void EndPage() {
    for (int i = 0; i < settings.feedDist; ++i)
        skiplines(0x18);
    signal(15, old_signal);
}

void cancelJob(int foo) {
    for (int i = 0; i < 0x258; ++i)
        mputchar(0);
    EndPage();
    ShutDown();
}

void pageSetup() {
    old_signal = signal(15, cancelJob);
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
            perror("ERROR: Unable to open raster file - ");
            sleep(1);
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

        int foo = (header.cupsWidth > 0x240) ? 0x240 : header.cupsWidth;
        foo = (foo + 7) & 0xFFFFFFF8;
        int width_size = foo >> 3;

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
