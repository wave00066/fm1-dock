/* SPDX-License-Identifier: GPL-3.0-only */
/* HYP-1: RustyPawz's hypnotic techno and acid machine (RedtailWorks; github.com/RustyPawz/fm1-HYP1, 1.5.0, 824316e;
 * its files in hyp/ unchanged but for RAM, see take.py), a fork of AMB-1, as a fm1-dock module: a breathing pad, a
 * deep drone, a generative 303 line and a bed of grain over drums you sequence, in ten pieces. Its app on fm1-dock's
 * host (hyp_platform.inc: its platform calls); its engine renders in the host's audio interrupt, as on its own
 * firmware (one core), with the FPU as HYP-1 builds it (module.cfg). HOME is HYP-1's TEMPO page: HOME held 1.5 s
 * alone opens fm1-dock's module list. As modules/dub/dub_module.c. */
#include "hyp_platform.inc"
#include "project.c"
#include "ui.c"

#define HYP_BLOCK 256u
static int32_t hyp_buf[2u * HYP_BLOCK];
static void hyp_render(int32_t *out, uint32_t n)
{
    uint32_t t0 = H->ticks(), i;
    if (n != HYP_BLOCK)
        return;
    en_render(hyp_buf, n, 4096u);           /* (the host applies MASTER) */
    for (i = 0; i < 2u * n; i++) {          /* HYP-1's 24 bits into the host's Q15, added */
        int32_t v = out[i] + (hyp_buf[i] >> 8);
        out[i] = v > 32767 ? 32767 : v < -32768 ? -32768 : v;
    }
    hyp_load(H->ticks() - t0);
}

static void hyp_ui(uint32_t ev, int32_t arg) { hbf_event(ev, arg); }
static void hyp_midi(uint32_t st, uint32_t d1, uint32_t d2) { hbf_midi(st, d1, d2); }
static void hyp_note(uint32_t ch, uint32_t key, uint32_t vel) { hbf_midi(0x90u | (ch & 15u), key, vel); }
static void hyp_cc(uint32_t ch, uint32_t num, uint32_t val) { hbf_midi(0xB0u | (ch & 15u), num, val); }

/* its boot cards first (ui_init starts them, ui_frame plays them and then starts the music), then the UI */
static uint32_t hyp_last_frame, hyp_started;
static void hyp_draw(uint32_t full)
{
    uint32_t now = H->ms();
    if (!hyp_started) {
        hyp_started = 1;
        ui_init();
        hyp_lights(1);
        hyp_last_frame = now;
        return;
    }
    if (full && !ui.splash) {               /* (back from the module list: the whole screen again, as it was, and
                                             * the lights; each band's picture differs from what it was drawn for) */
        lcd_fill(0, 0, 240, 240, K_BG);
        ui.sig[0] = ~ui.sig[0];
        ui.sig[1] = ~ui.sig[1];
        ui.sig[2] = ~ui.sig[2];
        hyp_lights(1);
    }
    if (now - hyp_last_frame >= 16u) {      /* ~60 frames a second at most, input in between (its main loop) */
        hyp_last_frame = now;
        ui_frame();
    } else {
        ui_input_only();
    }
}

static int hyp_init(const hb_host_t *h)
{
    H = h;
    en_init();
    project_start(plat_seed());             /* a piece at random, a little changed, or what was saved */
    project_apply();
    H->log("hyp: HYP-1 1.5.0 (RustyPawz/fm1-HYP1 824316e)");
    return 0;
}

static void hyp_release(void)
{
    en_play(0);
    en_panic();
}

HB_DESC = {HB_ABI, "HYP-1", hyp_init, hyp_release, hyp_note, hyp_cc, hyp_render, hyp_ui, hyp_draw, 0, 0,
           HB_F_SCREEN | HB_F_PANEL, hyp_midi};
