#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lv2/core/lv2.h"

static const char* kUri = "urn:asier:lv2:ttape202";

enum PortIndex : uint32_t {
  IN_L = 0,
  IN_R,
  OUT_L,
  OUT_R,
  MODE_SELECTOR,
  REPEAT_RATE,
  INTENSITY,
  ECHO_VOL,
  REVERB_VOL,
  SATURATION,
  WOW_FLUTTER,
  BASS,
  TREBLE,
  TAPE_AGE,
  TIME_MODE,
  REVERB_TYPE,
  DIRECT_MODE,
  CARRYOVER,
  KILL_DRY,
  INPUT_MODE,
  WARP,
  TWIST,
  TWIST_TYPE,
  OUTPUT_DB,
  STEREO_WIDTH,
  DRY_WET,
  TAP_SETTING,
  REPEAT_SYNC,
  SYNC_BPM,
  WOW_FLUTTER_GLOBAL
};

enum ParamIndex : uint32_t {
  P_MODE_SELECTOR = 0,
  P_REPEAT_RATE,
  P_INTENSITY,
  P_ECHO_VOL,
  P_REVERB_VOL,
  P_SATURATION,
  P_WOW_FLUTTER,
  P_BASS,
  P_TREBLE,
  P_TAPE_AGE,
  P_TIME_MODE,
  P_REVERB_TYPE,
  P_DIRECT_MODE,
  P_CARRYOVER,
  P_KILL_DRY,
  P_INPUT_MODE,
  P_WARP,
  P_TWIST,
  P_TWIST_TYPE,
  P_OUTPUT_DB,
  P_STEREO_WIDTH,
  P_DRY_WET,
  P_TAP_SETTING,
  P_REPEAT_SYNC,
  P_SYNC_BPM,
  P_WOW_FLUTTER_GLOBAL,
  kParamCount
};

struct ParamDef {
  float def;
  float min;
  float max;
};

static const ParamDef kParamDefs[kParamCount] = {
    {7.0f, 1.0f, 12.0f},     // mode_selector
    {0.45f, 0.0f, 1.0f},     // repeat_rate
    {0.55f, 0.0f, 1.0f},     // intensity
    {0.62f, 0.0f, 1.0f},     // echo_vol
    {0.20f, 0.0f, 1.0f},     // reverb_vol
    {0.20f, 0.0f, 1.0f},     // saturation
    {0.16f, 0.0f, 1.0f},     // wow_flutter
    {0.0f, -1.0f, 1.0f},     // bass
    {0.0f, -1.0f, 1.0f},     // treble
    {0.0f, 0.0f, 1.0f},      // tape_age
    {0.0f, 0.0f, 1.0f},      // time_mode
    {0.0f, 0.0f, 4.0f},      // reverb_type
    {0.0f, 0.0f, 1.0f},      // direct_mode
    {1.0f, 0.0f, 1.0f},      // carryover
    {0.0f, 0.0f, 1.0f},      // kill_dry
    {0.0f, 0.0f, 1.0f},      // input_mode
    {0.0f, 0.0f, 1.0f},      // warp
    {0.0f, 0.0f, 1.0f},      // twist
    {0.0f, 0.0f, 2.0f},      // twist_type
    {0.0f, -24.0f, 12.0f},   // output_db
    {0.55f, 0.0f, 1.0f},     // stereo_width
    {0.50f, 0.0f, 1.0f},     // dry_wet
    {0.0f, 0.0f, 5.0f},      // tap_setting
    {0.0f, 0.0f, 1.0f},      // repeat_sync
    {120.0f, 40.0f, 300.0f}, // sync_bpm
    {0.0f, 0.0f, 1.0f}       // wow_flutter_global
};

static const uint8_t kDiscreteParam[kParamCount] = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1};

// Head combinations per mode (1..12), bits 0..3 for heads 1..4.
static const uint8_t kModeMask[12] = {
    0x1u,  // 1: head1
    0x2u,  // 2: head2
    0x4u,  // 3: head3
    0x3u,  // 4: head1+head2
    0x6u,  // 5: head2+head3
    0x5u,  // 6: head1+head3
    0x7u,  // 7: head1+head2+head3
    0x9u,  // 8: head1+head4
    0xCu,  // 9: head3+head4
    0xDu,  // 10: head1+head3+head4
    0xBu,  // 11: head1+head2+head4
    0xFu   // 12: head1+head2+head3+head4
};

static const float kSyncDivisionsBeats[] = {
    4.0f, 3.0f, 2.0f, 1.5f, 1.0f, 0.75f, 0.6666667f, 0.5f, 0.375f, 0.3333333f, 0.25f, 0.1875f, 0.1666667f, 0.125f};
static constexpr int kSyncDivisionCount = static_cast<int>(sizeof(kSyncDivisionsBeats) / sizeof(kSyncDivisionsBeats[0]));
static constexpr uint32_t kGlobalWowSize = 2048u;
static constexpr uint32_t kRevComb1Size = 1559u;
static constexpr uint32_t kRevComb2Size = 2089u;
static constexpr uint32_t kRevComb3Size = 2713u;
static constexpr uint32_t kRevComb4Size = 3209u;
static constexpr uint32_t kRevAP1Size = 227u;
static constexpr uint32_t kRevAP2Size = 571u;
static const float kRevRoomBase[5] = {0.69f, 0.82f, 0.77f, 0.73f, 0.65f};
static const float kRevDampFc[5] = {8800.0f, 5200.0f, 6800.0f, 7600.0f, 9800.0f};
static const float kRevAP1Gain[5] = {0.58f, 0.70f, 0.66f, 0.63f, 0.55f};
static const float kRevAP2Gain[5] = {0.46f, 0.57f, 0.53f, 0.49f, 0.44f};
static const float kRevInDrive[5] = {0.86f, 0.98f, 0.92f, 0.88f, 0.80f};

struct Plugin {
  float sr;
  const float* in_l;
  const float* in_r;
  float* out_l;
  float* out_r;
  const float* controls[kParamCount];
  float smooth[kParamCount];

  float* delay_l;
  float* delay_r;
  uint32_t delay_size;
  uint32_t write_pos;

  float echo_lp_l;
  float echo_lp_r;
  float dry_lp_l;
  float dry_lp_r;
  float wow_phase;
  float flutter_phase;
  uint32_t rng;
  float wow_global_l[kGlobalWowSize];
  float wow_global_r[kGlobalWowSize];
  uint32_t wow_global_pos;

  float rev_c1_l[kRevComb1Size];
  float rev_c1_r[kRevComb1Size];
  float rev_c2_l[kRevComb2Size];
  float rev_c2_r[kRevComb2Size];
  float rev_c3_l[kRevComb3Size];
  float rev_c3_r[kRevComb3Size];
  float rev_c4_l[kRevComb4Size];
  float rev_c4_r[kRevComb4Size];
  float rev_ap1_l[kRevAP1Size];
  float rev_ap1_r[kRevAP1Size];
  float rev_ap2_l[kRevAP2Size];
  float rev_ap2_r[kRevAP2Size];
  uint32_t rev_c1_pos_l;
  uint32_t rev_c1_pos_r;
  uint32_t rev_c2_pos_l;
  uint32_t rev_c2_pos_r;
  uint32_t rev_c3_pos_l;
  uint32_t rev_c3_pos_r;
  uint32_t rev_c4_pos_l;
  uint32_t rev_c4_pos_r;
  uint32_t rev_ap1_pos_l;
  uint32_t rev_ap1_pos_r;
  uint32_t rev_ap2_pos_l;
  uint32_t rev_ap2_pos_r;
  float rev_c1_lp_l;
  float rev_c1_lp_r;
  float rev_c2_lp_l;
  float rev_c2_lp_r;
  float rev_c3_lp_l;
  float rev_c3_lp_r;
  float rev_c4_lp_l;
  float rev_c4_lp_r;
  float rev_tone_l;
  float rev_tone_r;
};

static constexpr float kPi = 3.14159265358979323846f;
static constexpr float kTwoPi = 6.28318530717958647692f;
static constexpr float kRefSampleRate = 48000.0f;
static constexpr float kMaxDelaySeconds = 8.2f;

static bool finite_float(float x) {
  return __builtin_isfinite(x);
}

static bool finite_double(double x) {
  return __builtin_isfinite(x);
}

static float finite_or(float x, float fallback) {
  return finite_float(x) ? x : fallback;
}

static float clamp(float x, float lo, float hi) {
  x = finite_or(x, lo);
  return x < lo ? lo : (x > hi ? hi : x);
}

static float zap(float x) {
  x = finite_or(x, 0.0f);
  return fabsf(x) < 1.0e-20f ? 0.0f : x;
}

static float sample_rate(const Plugin* p) {
  return (p && finite_float(p->sr) && p->sr >= 1000.0f && p->sr <= 384000.0f) ? p->sr : kRefSampleRate;
}

static int nearest_int(float x) {
  x = finite_or(x, 0.0f);
  return static_cast<int>(x + (x >= 0.0f ? 0.5f : -0.5f));
}

static float quantize_2(float x) {
  x = finite_or(x, 0.0f);
  const float scaled = x * 100.0f;
  const int32_t rounded = static_cast<int32_t>(scaled + (scaled >= 0.0f ? 0.5f : -0.5f));
  return static_cast<float>(rounded) * 0.01f;
}

static float time_coef_ms(const Plugin* p, float ms) {
  ms = clamp(ms, 0.1f, 20000.0f);
  const float sr = sample_rate(p);
  return finite_or(expf(-1.0f / (0.001f * ms * sr)), 0.0f);
}

static float wrap_phase01(float x) {
  if (!finite_float(x)) return 0.0f;
  while (x >= 1.0f) x -= 1.0f;
  while (x < 0.0f) x += 1.0f;
  return x;
}

static float db_to_gain(float db) {
  return finite_or(powf(10.0f, db / 20.0f), 1.0f);
}

static float soft_clip(float x) {
  x = finite_or(x, 0.0f);
  return finite_or(tanhf(x), 0.0f);
}

static int control_switch(float x) {
  return x >= 0.5f ? 1 : 0;
}

static uint32_t next_u32(Plugin* p) {
  p->rng = p->rng * 1664525u + 1013904223u;
  return p->rng;
}

static float frand(Plugin* p) {
  return ((next_u32(p) >> 8) & 0xFFFFu) / 32768.0f - 1.0f;
}

static float read_delay(const float* buf, uint32_t size, uint32_t wpos, float delay_samples) {
  if (!buf || size < 4u) return 0.0f;
  delay_samples = clamp(delay_samples, 1.0f, static_cast<float>(size - 2u));

  float r = static_cast<float>(wpos) - delay_samples;
  while (r < 0.0f) r += static_cast<float>(size);
  while (r >= static_cast<float>(size)) r -= static_cast<float>(size);

  const uint32_t i0 = static_cast<uint32_t>(r);
  const uint32_t i1 = (i0 + 1u < size) ? i0 + 1u : 0u;
  const float frac = r - static_cast<float>(i0);
  const float s0 = buf[i0];
  const float s1 = buf[i1];
  return finite_or(s0 + (s1 - s0) * frac, 0.0f);
}

static float eq_repeats(Plugin* p, float x, float bass, float treble, bool left) {
  x = finite_or(x, 0.0f);
  bass = clamp(bass, -1.0f, 1.0f);
  treble = clamp(treble, -1.0f, 1.0f);

  float* lp = left ? &p->echo_lp_l : &p->echo_lp_r;
  const float sr = sample_rate(p);
  const float alpha = clamp(1.0f - finite_or(expf(-2.0f * kPi * 1200.0f / sr), 0.0f), 0.00001f, 0.99f);
  *lp += (x - *lp) * alpha;
  *lp = zap(clamp(*lp, -8.0f, 8.0f));

  const float low = *lp;
  const float high = x - *lp;
  const float low_gain = db_to_gain(bass * 15.0f);
  const float high_gain = db_to_gain(treble * 15.0f);

  return finite_or(low * low_gain + high * high_gain, 0.0f);
}

static float preamp_color(Plugin* p, float x, float sat, bool aged, bool left) {
  x = finite_or(x, 0.0f);
  sat = clamp(sat, 0.0f, 1.0f);
  const float age_amt = aged ? 1.0f : 0.0f;

  float* lp = left ? &p->dry_lp_l : &p->dry_lp_r;
  const float sr = sample_rate(p);
  const float fc = 6500.0f - age_amt * 3200.0f;
  const float alpha = clamp(1.0f - finite_or(expf(-2.0f * kPi * fc / sr), 0.0f), 0.00001f, 0.99f);
  *lp += (x - *lp) * alpha;
  *lp = zap(clamp(*lp, -8.0f, 8.0f));

  const float pregain = 1.0f + sat * (aged ? 6.0f : 4.0f);
  return finite_or(soft_clip(*lp * pregain) / (1.0f + sat * 0.55f), 0.0f);
}

static float one_pole_alpha_hz(float fc, float sr) {
  fc = clamp(fc, 20.0f, sr * 0.45f);
  return clamp(1.0f - finite_or(expf(-2.0f * kPi * fc / sr), 0.0f), 0.00001f, 0.99f);
}

static float comb_tick(float* buf, uint32_t size, uint32_t* pos, float* lp_state, float input, float fb, float damp_a) {
  const uint32_t idx = *pos;
  const float y = buf[idx];
  *lp_state += (y - *lp_state) * damp_a;
  const float w = clamp(input + (*lp_state) * fb, -4.0f, 4.0f);
  buf[idx] = zap(w);
  *pos = (idx + 1u < size) ? idx + 1u : 0u;
  return y;
}

static float allpass_tick(float* buf, uint32_t size, uint32_t* pos, float input, float g) {
  const uint32_t idx = *pos;
  const float z = buf[idx];
  const float y = z - g * input;
  buf[idx] = zap(clamp(input + g * y, -4.0f, 4.0f));
  *pos = (idx + 1u < size) ? idx + 1u : 0u;
  return y;
}

static float process_reverb(Plugin* p, float x, int type, float amount, bool left) {
  amount = clamp(amount, 0.0f, 1.0f);
  if (amount <= 0.00001f) return 0.0f;

  type = type < 0 ? 0 : (type > 4 ? 4 : type);
  x = finite_or(x, 0.0f);

  const float room = clamp(kRevRoomBase[type] + amount * 0.10f, 0.55f, 0.93f);
  const float damp_a = one_pole_alpha_hz(kRevDampFc[type] * (1.0f - amount * 0.28f), sample_rate(p));
  const float input = x * kRevInDrive[type] * (0.55f + amount * 0.65f);

  float* c1_buf = left ? p->rev_c1_l : p->rev_c1_r;
  float* c2_buf = left ? p->rev_c2_l : p->rev_c2_r;
  float* c3_buf = left ? p->rev_c3_l : p->rev_c3_r;
  float* c4_buf = left ? p->rev_c4_l : p->rev_c4_r;
  float* ap1_buf = left ? p->rev_ap1_l : p->rev_ap1_r;
  float* ap2_buf = left ? p->rev_ap2_l : p->rev_ap2_r;
  uint32_t* c1_pos = left ? &p->rev_c1_pos_l : &p->rev_c1_pos_r;
  uint32_t* c2_pos = left ? &p->rev_c2_pos_l : &p->rev_c2_pos_r;
  uint32_t* c3_pos = left ? &p->rev_c3_pos_l : &p->rev_c3_pos_r;
  uint32_t* c4_pos = left ? &p->rev_c4_pos_l : &p->rev_c4_pos_r;
  uint32_t* ap1_pos = left ? &p->rev_ap1_pos_l : &p->rev_ap1_pos_r;
  uint32_t* ap2_pos = left ? &p->rev_ap2_pos_l : &p->rev_ap2_pos_r;

  float* c1_lp = left ? &p->rev_c1_lp_l : &p->rev_c1_lp_r;
  float* c2_lp = left ? &p->rev_c2_lp_l : &p->rev_c2_lp_r;
  float* c3_lp = left ? &p->rev_c3_lp_l : &p->rev_c3_lp_r;
  float* c4_lp = left ? &p->rev_c4_lp_l : &p->rev_c4_lp_r;
  float* tone = left ? &p->rev_tone_l : &p->rev_tone_r;

  float c1 = comb_tick(c1_buf, kRevComb1Size, c1_pos, c1_lp, input, room * 0.996f, damp_a);
  float c2 = comb_tick(c2_buf, kRevComb2Size, c2_pos, c2_lp, input, room * 0.973f, damp_a);
  float c3 = comb_tick(c3_buf, kRevComb3Size, c3_pos, c3_lp, input, room * 0.951f, damp_a);
  float c4 = comb_tick(c4_buf, kRevComb4Size, c4_pos, c4_lp, input, room * 0.929f, damp_a);

  float y = (c1 + c2 + c3 + c4) * 0.25f;
  y = allpass_tick(ap1_buf, kRevAP1Size, ap1_pos, y, kRevAP1Gain[type]);
  y = allpass_tick(ap2_buf, kRevAP2Size, ap2_pos, y, kRevAP2Gain[type]);

  const float tone_a = one_pole_alpha_hz(kRevDampFc[type] * 0.75f, sample_rate(p));
  *tone += (y - *tone) * tone_a;
  y = *tone;

  return finite_or(y * amount, 0.0f);
}

static void init_plugin(Plugin* p, float sr) {
  memset(p, 0, sizeof(Plugin));
  p->sr = sr;
  p->rng = 0x54545032u;
  for (uint32_t i = 0; i < kParamCount; ++i) {
    p->controls[i] = nullptr;
    p->smooth[i] = kParamDefs[i].def;
  }
}

static void reset_dsp_state(Plugin* p) {
  if (!p) return;
  p->write_pos = 0u;
  p->echo_lp_l = 0.0f;
  p->echo_lp_r = 0.0f;
  p->dry_lp_l = 0.0f;
  p->dry_lp_r = 0.0f;
  p->wow_phase = 0.0f;
  p->flutter_phase = 0.0f;
  p->wow_global_pos = 0u;
  p->rng = 0x54545032u;
  p->rev_c1_pos_l = 0u;
  p->rev_c1_pos_r = 0u;
  p->rev_c2_pos_l = 0u;
  p->rev_c2_pos_r = 0u;
  p->rev_c3_pos_l = 0u;
  p->rev_c3_pos_r = 0u;
  p->rev_c4_pos_l = 0u;
  p->rev_c4_pos_r = 0u;
  p->rev_ap1_pos_l = 0u;
  p->rev_ap1_pos_r = 0u;
  p->rev_ap2_pos_l = 0u;
  p->rev_ap2_pos_r = 0u;
  p->rev_c1_lp_l = 0.0f;
  p->rev_c1_lp_r = 0.0f;
  p->rev_c2_lp_l = 0.0f;
  p->rev_c2_lp_r = 0.0f;
  p->rev_c3_lp_l = 0.0f;
  p->rev_c3_lp_r = 0.0f;
  p->rev_c4_lp_l = 0.0f;
  p->rev_c4_lp_r = 0.0f;
  p->rev_tone_l = 0.0f;
  p->rev_tone_r = 0.0f;

  if (p->delay_l && p->delay_size > 0u) memset(p->delay_l, 0, sizeof(float) * p->delay_size);
  if (p->delay_r && p->delay_size > 0u) memset(p->delay_r, 0, sizeof(float) * p->delay_size);
  memset(p->wow_global_l, 0, sizeof(p->wow_global_l));
  memset(p->wow_global_r, 0, sizeof(p->wow_global_r));
  memset(p->rev_c1_l, 0, sizeof(p->rev_c1_l));
  memset(p->rev_c1_r, 0, sizeof(p->rev_c1_r));
  memset(p->rev_c2_l, 0, sizeof(p->rev_c2_l));
  memset(p->rev_c2_r, 0, sizeof(p->rev_c2_r));
  memset(p->rev_c3_l, 0, sizeof(p->rev_c3_l));
  memset(p->rev_c3_r, 0, sizeof(p->rev_c3_r));
  memset(p->rev_c4_l, 0, sizeof(p->rev_c4_l));
  memset(p->rev_c4_r, 0, sizeof(p->rev_c4_r));
  memset(p->rev_ap1_l, 0, sizeof(p->rev_ap1_l));
  memset(p->rev_ap1_r, 0, sizeof(p->rev_ap1_r));
  memset(p->rev_ap2_l, 0, sizeof(p->rev_ap2_l));
  memset(p->rev_ap2_r, 0, sizeof(p->rev_ap2_r));
}

static LV2_Handle instantiate(const LV2_Descriptor*, double rate, const char*, const LV2_Feature* const*) {
  Plugin* p = static_cast<Plugin*>(malloc(sizeof(Plugin)));
  if (!p) return nullptr;

  const float sr = (finite_double(rate) && rate >= 1000.0 && rate <= 384000.0) ? static_cast<float>(rate) : kRefSampleRate;
  init_plugin(p, sr);

  const uint32_t size = static_cast<uint32_t>(sr * kMaxDelaySeconds) + 8u;
  p->delay_size = (size < 1024u) ? 1024u : size;
  p->delay_l = static_cast<float*>(calloc(p->delay_size, sizeof(float)));
  p->delay_r = static_cast<float*>(calloc(p->delay_size, sizeof(float)));
  if (!p->delay_l || !p->delay_r) {
    free(p->delay_l);
    free(p->delay_r);
    free(p);
    return nullptr;
  }

  reset_dsp_state(p);
  return p;
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data) {
  Plugin* p = static_cast<Plugin*>(instance);
  if (!p) return;

  switch (port) {
    case IN_L:
      p->in_l = static_cast<const float*>(data);
      break;
    case IN_R:
      p->in_r = static_cast<const float*>(data);
      break;
    case OUT_L:
      p->out_l = static_cast<float*>(data);
      break;
    case OUT_R:
      p->out_r = static_cast<float*>(data);
      break;
    case MODE_SELECTOR:
      p->controls[P_MODE_SELECTOR] = static_cast<const float*>(data);
      break;
    case REPEAT_RATE:
      p->controls[P_REPEAT_RATE] = static_cast<const float*>(data);
      break;
    case INTENSITY:
      p->controls[P_INTENSITY] = static_cast<const float*>(data);
      break;
    case ECHO_VOL:
      p->controls[P_ECHO_VOL] = static_cast<const float*>(data);
      break;
    case REVERB_VOL:
      p->controls[P_REVERB_VOL] = static_cast<const float*>(data);
      break;
    case SATURATION:
      p->controls[P_SATURATION] = static_cast<const float*>(data);
      break;
    case WOW_FLUTTER:
      p->controls[P_WOW_FLUTTER] = static_cast<const float*>(data);
      break;
    case BASS:
      p->controls[P_BASS] = static_cast<const float*>(data);
      break;
    case TREBLE:
      p->controls[P_TREBLE] = static_cast<const float*>(data);
      break;
    case TAPE_AGE:
      p->controls[P_TAPE_AGE] = static_cast<const float*>(data);
      break;
    case TIME_MODE:
      p->controls[P_TIME_MODE] = static_cast<const float*>(data);
      break;
    case REVERB_TYPE:
      p->controls[P_REVERB_TYPE] = static_cast<const float*>(data);
      break;
    case DIRECT_MODE:
      p->controls[P_DIRECT_MODE] = static_cast<const float*>(data);
      break;
    case CARRYOVER:
      p->controls[P_CARRYOVER] = static_cast<const float*>(data);
      break;
    case KILL_DRY:
      p->controls[P_KILL_DRY] = static_cast<const float*>(data);
      break;
    case INPUT_MODE:
      p->controls[P_INPUT_MODE] = static_cast<const float*>(data);
      break;
    case WARP:
      p->controls[P_WARP] = static_cast<const float*>(data);
      break;
    case TWIST:
      p->controls[P_TWIST] = static_cast<const float*>(data);
      break;
    case TWIST_TYPE:
      p->controls[P_TWIST_TYPE] = static_cast<const float*>(data);
      break;
    case OUTPUT_DB:
      p->controls[P_OUTPUT_DB] = static_cast<const float*>(data);
      break;
    case STEREO_WIDTH:
      p->controls[P_STEREO_WIDTH] = static_cast<const float*>(data);
      break;
    case DRY_WET:
      p->controls[P_DRY_WET] = static_cast<const float*>(data);
      break;
    case TAP_SETTING:
      p->controls[P_TAP_SETTING] = static_cast<const float*>(data);
      break;
    case REPEAT_SYNC:
      p->controls[P_REPEAT_SYNC] = static_cast<const float*>(data);
      break;
    case SYNC_BPM:
      p->controls[P_SYNC_BPM] = static_cast<const float*>(data);
      break;
    case WOW_FLUTTER_GLOBAL:
      p->controls[P_WOW_FLUTTER_GLOBAL] = static_cast<const float*>(data);
      break;
  }
}

static void activate(LV2_Handle instance) {
  reset_dsp_state(static_cast<Plugin*>(instance));
}

static void run(LV2_Handle instance, uint32_t nframes) {
  Plugin* p = static_cast<Plugin*>(instance);
  if (!p || !p->in_l || !p->in_r || !p->out_l || !p->out_r || !p->delay_l || !p->delay_r || p->delay_size < 4u) return;

  const float sr = sample_rate(p);
  const float smooth_step = 1.0f - time_coef_ms(p, 9.0f);

  for (uint32_t i = 0; i < nframes; ++i) {
    for (uint32_t k = 0; k < kParamCount; ++k) {
      const ParamDef* def = &kParamDefs[k];
      float target = p->controls[k] ? *p->controls[k] : def->def;
      target = clamp(target, def->min, def->max);

      if (kDiscreteParam[k]) {
        p->smooth[k] = clamp(static_cast<float>(nearest_int(target)), def->min, def->max);
      } else {
        target = quantize_2(target);
        p->smooth[k] += (target - p->smooth[k]) * smooth_step;
        p->smooth[k] = zap(clamp(p->smooth[k], def->min, def->max));
      }
    }

    const int mode = nearest_int(p->smooth[P_MODE_SELECTOR]);
    float repeat_rate = clamp(p->smooth[P_REPEAT_RATE], 0.0f, 1.0f);
    float feedback = clamp(p->smooth[P_INTENSITY], 0.0f, 1.0f);
    float echo_vol = clamp(p->smooth[P_ECHO_VOL], 0.0f, 1.0f);
    float reverb_vol = clamp(p->smooth[P_REVERB_VOL], 0.0f, 1.0f);
    float sat = clamp(p->smooth[P_SATURATION], 0.0f, 1.0f);
    float wow_amt = clamp(p->smooth[P_WOW_FLUTTER], 0.0f, 1.0f);
    const float bass = clamp(p->smooth[P_BASS], -1.0f, 1.0f);
    const float treble = clamp(p->smooth[P_TREBLE], -1.0f, 1.0f);

    const bool tape_aged = control_switch(p->smooth[P_TAPE_AGE]) != 0;
    const bool time_long = control_switch(p->smooth[P_TIME_MODE]) != 0;
    const int reverb_type = nearest_int(p->smooth[P_REVERB_TYPE]);
    const int direct_mode = nearest_int(p->smooth[P_DIRECT_MODE]);
    const bool direct_preamp = direct_mode > 0;
    const bool carryover = control_switch(p->smooth[P_CARRYOVER]) != 0;
    const bool kill_dry = control_switch(p->smooth[P_KILL_DRY]) != 0;
    const bool line_in = control_switch(p->smooth[P_INPUT_MODE]) != 0;
    const bool warp = control_switch(p->smooth[P_WARP]) != 0;
    const bool twist = control_switch(p->smooth[P_TWIST]) != 0;
    const int twist_type = nearest_int(p->smooth[P_TWIST_TYPE]);
    const float dry_wet = clamp(p->smooth[P_DRY_WET], 0.0f, 1.0f);
    const int tap_setting = nearest_int(p->smooth[P_TAP_SETTING]);
    const bool repeat_sync = control_switch(p->smooth[P_REPEAT_SYNC]) != 0;
    const float sync_bpm = clamp(p->smooth[P_SYNC_BPM], 40.0f, 300.0f);
    const bool wow_flutter_global = control_switch(p->smooth[P_WOW_FLUTTER_GLOBAL]) != 0;

    // At max INTENSITY, the unit should enter unstable/self-oscillation territory.
    feedback = clamp(feedback * feedback * 1.08f + feedback * 0.14f, 0.0f, 1.24f);

    if (warp) {
      feedback = clamp(feedback + 0.38f, 0.0f, 1.24f);
      repeat_rate = clamp(repeat_rate * 1.50f + 0.12f, 0.0f, 1.0f);
      sat = clamp(sat + 0.15f, 0.0f, 1.0f);
    }
    if (twist) {
      const int tt = twist_type < 0 ? 0 : (twist_type > 2 ? 2 : twist_type);
      if (tt == 1) {  // Hard
        wow_amt = clamp(wow_amt + 0.45f, 0.0f, 1.0f);
        sat = clamp(sat + 0.25f, 0.0f, 1.0f);
        feedback = clamp(feedback + 0.20f, 0.0f, 1.28f);
        repeat_rate = clamp(repeat_rate * 0.42f, 0.0f, 1.0f);
      } else if (tt == 2) {  // Natural
        wow_amt = clamp(wow_amt + 0.18f, 0.0f, 1.0f);
        sat = clamp(sat + 0.10f, 0.0f, 1.0f);
        repeat_rate = clamp(repeat_rate * 0.70f, 0.0f, 1.0f);
      } else {  // Normal
        wow_amt = clamp(wow_amt + 0.32f, 0.0f, 1.0f);
        sat = clamp(sat + 0.20f, 0.0f, 1.0f);
        repeat_rate = clamp(repeat_rate * 0.55f, 0.0f, 1.0f);
      }
    }

    const float min_base = 0.035f;
    const float max_base = time_long ? 2.0f : 1.0f;

    const uint8_t mask = kModeMask[(mode < 1 ? 1 : (mode > 12 ? 12 : mode)) - 1];
    const float factors[4] = {
        (mode == 12) ? 1.00f : 1.00f,
        (mode == 12) ? 1.68f : 2.00f,
        (mode == 12) ? 2.52f : 3.00f,
        (mode == 12) ? 3.32f : 4.00f};

    float shortest = 1000.0f;
    float longest = 0.0f;
    for (int h = 0; h < 4; ++h) {
      if (!(mask & (1u << h))) continue;
      if (factors[h] < shortest) shortest = factors[h];
      if (factors[h] > longest) longest = factors[h];
    }
    if (shortest > 999.0f) shortest = 1.0f;
    if (longest < 0.0001f) longest = 1.0f;

    float tap_ref = 1.0f;   // head1
    float note_mul = 1.0f;  // quarter
    const int ts = tap_setting < 0 ? 0 : (tap_setting > 5 ? 5 : tap_setting);
    if (ts == 1) {
      note_mul = 0.75f;  // dotted eighth
    } else if (ts == 2) {
      tap_ref = shortest;  // short
    } else if (ts == 3) {
      tap_ref = shortest;
      note_mul = 0.75f;
    } else if (ts == 4) {
      tap_ref = longest;  // long
    } else if (ts == 5) {
      tap_ref = longest;
      note_mul = 0.75f;
    }

    float base_time = 0.0f;
    if (repeat_sync) {
      const float quarter = 60.0f / sync_bpm;
      const float raw_index = clamp(repeat_rate, 0.0f, 1.0f) * static_cast<float>(kSyncDivisionCount - 1);
      int div_index = nearest_int(raw_index);
      if (div_index < 0) div_index = 0;
      if (div_index >= kSyncDivisionCount) div_index = kSyncDivisionCount - 1;
      const float beat_mul = kSyncDivisionsBeats[div_index];
      base_time = quarter * beat_mul * tap_ref * note_mul;
    } else {
      // Clockwise REPEAT RATE => faster tape => shorter echo spacing.
      const float head1_time = max_base * finite_or(powf(min_base / max_base, repeat_rate), 1.0f);
      base_time = head1_time * tap_ref * note_mul;
    }
    base_time = clamp(base_time, min_base, max_base);

    p->wow_phase = wrap_phase01(p->wow_phase + (0.08f + wow_amt * 0.60f) / sr);
    p->flutter_phase = wrap_phase01(p->flutter_phase + (2.8f + wow_amt * 8.5f) / sr);
    float wow = sinf(kTwoPi * p->wow_phase) * (0.0014f + wow_amt * 0.0090f);
    wow += sinf(kTwoPi * p->flutter_phase) * (0.0004f + wow_amt * 0.0020f);
    if (tape_aged) wow *= 1.75f;
    const float wow_for_echo = wow_flutter_global ? 0.0f : wow;

    float in_l = finite_or(p->in_l[i], 0.0f);
    float in_r = finite_or(p->in_r[i], 0.0f);
    const float in_trim = line_in ? 1.0f : 0.78f;
    in_l *= in_trim;
    in_r *= in_trim;

    // Optional global wow/flutter mode: modulate full signal path instead of echoes only.
    const uint32_t wow_pos = p->wow_global_pos;
    p->wow_global_l[wow_pos] = in_l;
    p->wow_global_r[wow_pos] = in_r;
    if (wow_flutter_global) {
      const float center = 96.0f;
      const float mod_samp = wow * sr * 0.85f;
      const float d_l = clamp(center + mod_samp, 1.0f, static_cast<float>(kGlobalWowSize - 2u));
      const float d_r = clamp(center + mod_samp * 1.07f, 1.0f, static_cast<float>(kGlobalWowSize - 2u));
      in_l = read_delay(p->wow_global_l, kGlobalWowSize, wow_pos, d_l);
      in_r = read_delay(p->wow_global_r, kGlobalWowSize, wow_pos, d_r);
    }
    p->wow_global_pos = (wow_pos + 1u < kGlobalWowSize) ? wow_pos + 1u : 0u;

    float dry_l = in_l;
    float dry_r = in_r;
    if (direct_preamp) {
      const float dm_scale = (direct_mode >= 2) ? 1.0f : 0.72f;
      dry_l = preamp_color(p, dry_l, sat * 0.55f * dm_scale, tape_aged, true);
      dry_r = preamp_color(p, dry_r, sat * 0.55f * dm_scale, tape_aged, false);
    }

    float taps_l = 0.0f;
    float taps_r = 0.0f;
    float tap_count = 0.0f;
    for (int h = 0; h < 4; ++h) {
      if (!(mask & (1u << h))) continue;
      float d = base_time * factors[h] * sr;
      d += wow_for_echo * sr * (0.5f + 0.25f * static_cast<float>(h));
      d = clamp(d, 1.0f, static_cast<float>(p->delay_size - 2u));
      taps_l += read_delay(p->delay_l, p->delay_size, p->write_pos, d);
      taps_r += read_delay(p->delay_r, p->delay_size, p->write_pos, d);
      tap_count += 1.0f;
    }
    if (tap_count > 0.0f) {
      const float n = 1.0f / tap_count;
      taps_l *= n;
      taps_r *= n;
    }

    taps_l = eq_repeats(p, taps_l, bass, treble, true);
    taps_r = eq_repeats(p, taps_r, bass, treble, false);
    taps_l = preamp_color(p, taps_l, sat, tape_aged, true);
    taps_r = preamp_color(p, taps_r, sat, tape_aged, false);

    const float width = clamp(p->smooth[P_STEREO_WIDTH], 0.0f, 1.0f);
    const float mid = (taps_l + taps_r) * 0.5f;
    const float side = (taps_l - taps_r) * (0.25f + width * 0.75f);
    const float wet_l = mid + side;
    const float wet_r = mid - side;

    const float rv_l = process_reverb(p, wet_l, reverb_type, reverb_vol, true);
    const float rv_r = process_reverb(p, wet_r, reverb_type, reverb_vol, false);

    float write_l = in_l + wet_l * feedback + frand(p) * (tape_aged ? 0.0007f : 0.0002f);
    float write_r = in_r + wet_r * feedback + frand(p) * (tape_aged ? 0.0007f : 0.0002f);
    write_l = clamp(write_l, -2.5f, 2.5f);
    write_r = clamp(write_r, -2.5f, 2.5f);

    const float input_energy = fabsf(in_l) + fabsf(in_r);
    const float decay = carryover ? 1.0f : (input_energy > 0.00004f ? 1.0f : 0.92f);
    p->delay_l[p->write_pos] = zap(write_l * decay);
    p->delay_r[p->write_pos] = zap(write_r * decay);

    p->write_pos = (p->write_pos + 1u < p->delay_size) ? p->write_pos + 1u : 0u;

    const float wet_bus_l = wet_l * echo_vol + rv_l;
    const float wet_bus_r = wet_r * echo_vol + rv_r;
    const float dry_amt = kill_dry ? 0.0f : (1.0f - dry_wet);
    const float wet_amt = dry_wet;

    float out_l = dry_l * dry_amt + wet_bus_l * wet_amt;
    float out_r = dry_r * dry_amt + wet_bus_r * wet_amt;

    out_l = soft_clip(out_l);
    out_r = soft_clip(out_r);

    const float out_gain = db_to_gain(p->smooth[P_OUTPUT_DB]);
    out_l = clamp(finite_or(out_l * out_gain, 0.0f), -1.0f, 1.0f);
    out_r = clamp(finite_or(out_r * out_gain, 0.0f), -1.0f, 1.0f);

    p->out_l[i] = out_l;
    p->out_r[i] = out_r;
  }
}

static void cleanup(LV2_Handle instance) {
  Plugin* p = static_cast<Plugin*>(instance);
  if (!p) return;
  free(p->delay_l);
  free(p->delay_r);
  free(p);
}

static const LV2_Descriptor descriptor = {
    kUri, instantiate, connect_port, activate, run, nullptr, cleanup, nullptr};

extern "C" LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index) {
  return index == 0 ? &descriptor : nullptr;
}
