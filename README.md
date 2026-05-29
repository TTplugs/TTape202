> **WIP:** TTape202 is an early S2400-focused LV2 stereo tape-echo effect prototype.

# TTape202 for S2400

TTape202 is an original delay/reverb effect inspired by Space Echo workflows (multi-head tape delay, wow/flutter motion, feedback character, and reverb blend).  
It is not a firmware clone and does not use proprietary presets, branding, or panel graphics from commercial products.

## Scope

- LV2 effect plugin (`lv2:DelayPlugin`)
- Stereo audio input and stereo audio output
- No GUI
- ARM64-first target for S2400 workflows
- 22 control ports (`index 4..25`)
- Internal parameter quantization to `0.01` for smoother editing behavior

## Build on Ubuntu ARM64

```bash
cd ~/TTape202
make clean
make
make check
```

Bundle:

```text
TTape202.lv2
```

Expected checks:

```bash
file TTape202.lv2/TTape202.so
readelf -d TTape202.lv2/TTape202.so | grep NEEDED
strings -a TTape202.lv2/TTape202.so | grep -E 'GLIBC_|GLIBCXX_|GCC_' | sort -V | uniq
nm -D TTape202.lv2/TTape202.so | grep lv2_descriptor
```

Optional validators (if installed):

```bash
lv2_validate TTape202.lv2/manifest.ttl TTape202.lv2/TTape202.ttl
LV2_PATH=. lv2lint -Mpack urn:asier:lv2:ttape202
```

## Copy to Plugin Folder

```bash
ssh root@[IP] 'rm -rf "/mnt/user/Musica/Desarrollo LV2/TTape202.lv2"'
scp -r TTape202.lv2 root@[IP]:'/mnt/user/Musica/Desarrollo LV2/'
```

## Port Order

- `0` `in_l`
- `1` `in_r`
- `2` `out_l`
- `3` `out_r`
- `4` `mode_selector`
- `5` `repeat_rate`
- `6` `intensity`
- `7` `echo_vol`
- `8` `reverb_vol`
- `9` `saturation`
- `10` `wow_flutter`
- `11` `bass`
- `12` `treble`
- `13` `tape_age`
- `14` `time_mode`
- `15` `reverb_type`
- `16` `direct_mode`
- `17` `carryover`
- `18` `kill_dry`
- `19` `input_mode`
- `20` `warp`
- `21` `twist`
- `22` `twist_type`
- `23` `output_db`
- `24` `stereo_width`
- `25` `dry_wet`

## Multi-Head Modes

Mode mapping follows the RE-202 reference table (heads 1..4):

- `1`: `H1`
- `2`: `H2`
- `3`: `H3`
- `4`: `H1+H2`
- `5`: `H2+H3`
- `6`: `H1+H3`
- `7`: `H1+H2+H3`
- `8`: `H1+H4`
- `9`: `H3+H4`
- `10`: `H1+H3+H4`
- `11`: `H1+H2+H4`
- `12`: `H1+H2+H3+H4` (dense spacing variant)

## Hidden-Setting Mapping

- `time_mode`: Normal (tap head-1 max 1 s) / Long (tap head-1 max 2 s)
- `reverb_type`: Spring / Hall / Plate / Room / Ambience
- `direct_mode`: Analog Bypass / RE-201 Simulation / RE-201 Simulation (SAT always style)
- `carryover`: preserves or mutes tails when effect state changes
- `kill_dry`: direct output on/off equivalent
- `warp`, `twist`: performance-style stress controls

## Quick Presets

### Tape Slap

- `mode_selector`: `1`
- `repeat_rate`: `0.22`
- `intensity`: `0.28`
- `echo_vol`: `0.55`
- `reverb_vol`: `0.08`
- `saturation`: `0.18`
- `wow_flutter`: `0.09`
- `bass`: `-0.10`
- `treble`: `0.18`
- `tape_age`: `0`
- `time_mode`: `0`
- `reverb_type`: `0`
- `direct_mode`: `1`
- `carryover`: `1`
- `kill_dry`: `0`
- `input_mode`: `1`
- `warp`: `0`
- `twist`: `0`
- `twist_type`: `0`
- `output_db`: `0.00`
- `stereo_width`: `0.35`
- `dry_wet`: `0.42`

### Dub Spiral

- `mode_selector`: `7`
- `repeat_rate`: `0.58`
- `intensity`: `0.78`
- `echo_vol`: `0.72`
- `reverb_vol`: `0.24`
- `saturation`: `0.36`
- `wow_flutter`: `0.28`
- `bass`: `0.20`
- `treble`: `-0.12`
- `tape_age`: `1`
- `time_mode`: `1`
- `reverb_type`: `1`
- `direct_mode`: `1`
- `carryover`: `1`
- `kill_dry`: `0`
- `input_mode`: `1`
- `warp`: `0`
- `twist`: `0`
- `twist_type`: `1`
- `output_db`: `-1.50`
- `stereo_width`: `0.70`
- `dry_wet`: `0.58`

### Ambient Wash

- `mode_selector`: `12`
- `repeat_rate`: `0.64`
- `intensity`: `0.66`
- `echo_vol`: `0.63`
- `reverb_vol`: `0.42`
- `saturation`: `0.24`
- `wow_flutter`: `0.33`
- `bass`: `-0.08`
- `treble`: `-0.04`
- `tape_age`: `1`
- `time_mode`: `1`
- `reverb_type`: `4`
- `direct_mode`: `0`
- `carryover`: `1`
- `kill_dry`: `0`
- `input_mode`: `1`
- `warp`: `0`
- `twist`: `0`
- `twist_type`: `2`
- `output_db`: `-2.00`
- `stereo_width`: `0.88`
- `dry_wet`: `0.66`
