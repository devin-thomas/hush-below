/* Native mGBA framebuffer smoke test for sessions where the Qt window is unavailable. */
#include <mgba/core/core.h>
#include <mgba-util/vfs.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    HB_A = 1,
    HB_SELECT = 4,
    HB_START = 8,
    HB_TITLE = 0,
    HB_BRIEF = 1,
    HB_PLAY = 2,
    HB_PAUSE = 3,
    HB_LOG = 4,
    HB_REPORT = 5,
    HB_ENDING = 6,
};

static struct mCore *s_core;
static uint32_t s_state;
static uint32_t s_offsets[11];
static const char *s_output;

static void fail(const char *message) {
    fprintf(stderr, "FAIL: %s\n", message);
    if (s_core) {
        s_core->setKeys(s_core, 0);
        s_core->deinit(s_core);
    }
    exit(1);
}

static void frames(unsigned count) {
    while (count--) {
        s_core->runFrame(s_core);
    }
}

static void tap(uint32_t keys) {
    s_core->setKeys(s_core, keys);
    frames(4);
    s_core->setKeys(s_core, 0);
    frames(3);
}

static uint8_t read8(unsigned field) {
    return (uint8_t) s_core->busRead8(s_core, s_state + s_offsets[field]);
}

static void expect_screen(uint8_t screen, const char *label) {
    uint8_t actual = read8(0);
    if (actual != screen) {
        fprintf(stderr, "FAIL: %s expected screen %u, got %u\n", label, screen, actual);
        fail("screen assertion");
    }
}

static void shot(const char *name) {
    char path[4096];
    int written = snprintf(path, sizeof(path), "%s/%s.png", s_output, name);
    if (written < 0 || (size_t) written >= sizeof(path)) {
        fail("screenshot path is too long");
    }
    struct VFile *file = VFileOpen(path, O_CREAT | O_TRUNC | O_WRONLY);
    if (!file) {
        fprintf(stderr, "FAIL: cannot create %s: %s\n", path, strerror(errno));
        fail("screenshot open");
    }
    bool ok = mCoreTakeScreenshotVF(s_core, file);
    file->close(file);
    if (!ok) {
        fail("mGBA screenshot encoder");
    }
}

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "usage: %s ROM OUTPUT_DIR HB_STATE_HEX HB_OFFSETS_HEX\n", argv[0]);
        return 2;
    }
    char *end = NULL;
    s_state = (uint32_t) strtoul(argv[3], &end, 16);
    bool ewram = s_state >= 0x02000000 && s_state < 0x02040000;
    bool iwram = s_state >= 0x03000000 && s_state < 0x03008000;
    if (!end || *end || (!ewram && !iwram)) {
        fail("invalid hb_state address");
    }
    uint32_t offsets_address = (uint32_t) strtoul(argv[4], &end, 16);
    if (!end || *end || offsets_address < 0x08000000 || offsets_address >= 0x0A000000) {
        fail("invalid hb_debug_offsets address");
    }
    s_output = argv[2];
    s_core = mCoreFind(argv[1]);
    if (!s_core || !s_core->init(s_core)) {
        fail("mGBA core initialization");
    }
    color_t *pixels = calloc(256 * 160, sizeof(*pixels));
    if (!pixels) {
        fail("framebuffer allocation");
    }
    s_core->setVideoBuffer(s_core, pixels, 256);
    mCoreInitConfig(s_core, "hushBelowHeadless");
    if (!mCoreLoadFile(s_core, argv[1])) {
        fail("ROM load");
    }
    s_core->reset(s_core);
    frames(120);
    for (unsigned i = 0; i < sizeof(s_offsets) / sizeof(s_offsets[0]); ++i) {
        s_offsets[i] = s_core->busRead32(s_core, offsets_address + i * 4);
        if (s_offsets[i] >= 256) {
            fail("invalid debug offset table");
        }
    }

    char title[13] = {0};
    char code[9] = {0};
    s_core->getGameTitle(s_core, title);
    s_core->getGameCode(s_core, code);
    if (strcmp(title, "HUSH BELOW!") || strcmp(code, "AGB-HSHB")) {
        fail("ROM identity");
    }

    expect_screen(HB_TITLE, "reset");
    shot("01-title");
    tap(HB_A);
    expect_screen(HB_BRIEF, "dive brief");
    shot("02-dive-brief");
    tap(HB_A);
    expect_screen(HB_PLAY, "gameplay");
    frames(20);
    shot("03-silent-water");
    s_core->busWrite16(s_core, s_state + s_offsets[7], 14 * 8);
    s_core->busWrite16(s_core, s_state + s_offsets[8], 12 * 8);
    frames(2);
    tap(HB_A);
    expect_screen(HB_PLAY, "sonar ping");
    frames(10);
    if (!read8(5)) {
        fail("sonar radius did not expand");
    }
    shot("04-sonar-ping");
    tap(HB_START);
    expect_screen(HB_PAUSE, "pause");
    shot("05-pause");
    tap(HB_START);
    expect_screen(HB_PLAY, "resume");
    tap(HB_SELECT);
    expect_screen(HB_LOG, "dive log");
    shot("06-dive-log");

    s_core->busWrite8(s_core, s_state + s_offsets[1], 2);
    s_core->busWrite8(s_core, s_state + s_offsets[2], 7);
    s_core->busWrite8(s_core, s_state + s_offsets[3], 9);
    s_core->busWrite8(s_core, s_state + s_offsets[4], 0);
    s_core->busWrite16(s_core, s_state + s_offsets[6], 2800);
    s_core->busWrite8(s_core, s_state + s_offsets[9], 2);
    s_core->busWrite8(s_core, s_state + s_offsets[10], 12);
    s_core->busWrite8(s_core, s_state + s_offsets[0], HB_REPORT);
    frames(8);
    expect_screen(HB_REPORT, "dive report");
    shot("07-dive-report");
    s_core->busWrite8(s_core, s_state + s_offsets[0], HB_ENDING);
    frames(8);
    expect_screen(HB_ENDING, "ending");
    shot("08-ending");

    s_core->setKeys(s_core, 0);
    s_core->unloadROM(s_core);
    mCoreConfigDeinit(&s_core->config);
    s_core->deinit(s_core);
    free(pixels);
    puts("PASS: native mGBA state, input, sonar, and 8 framebuffer captures");
    return 0;
}
