# DC2UDDF Development Guide

dc2uddf downloads dive logs from dive computers via
[libdivecomputer](https://www.libdivecomputer.org/) (0.9.0) and converts them
to UDDF 3.2.3 XML for interchange with logbook apps (MacDive, Subsurface,
Diving Log, etc.).

## Architecture

- `src/dc2uddf.c` — CLI entry point; drives libdivecomputer (device open,
  dive enumeration, `sample_cb` sample callback) and fills the `dif_*` data
  model. Note: libdivecomputer ≥ 0.8 reports `DC_SAMPLE_TIME` in
  **milliseconds**; `dif_sample_t.timestamp` is **seconds** (converted in
  `sample_cb`).
- `src/dif/dif.c` / `dif.h` — device-independent data model (dive
  collections, dives, samples, subsamples, gas mixes), sorting, surface
  interval calculation.
- `src/dif/uddf.c` — all UDDF XML serialization (libxml2). Waypoint children
  are ordered by an explicit rank table matching the schema's `waypointType`
  sequence (NOT alphabetical). `tankdata` elements are direct children of
  `<dive>` between `informationbeforedive` and `samples`.
- `src/dif/algos.c` — post-processing fixups (initial pressure fix, truncate
  after surfacing).
- `src/dumpfile.c` / `dumpfile.h` — splits on-disk dive-data dumps into
  individual records (Uwatec Smart framing). glib-only, no libdivecomputer
  dependency, so it's unit-testable in `check_dif`.
- `src/uwatec_smart_alarms.c` / `.h` — decodes the warning/alarm buzzer bits
  from raw Uwatec dive records (Galileo-bitstream models only); the sample
  walk is a port of upstream v0.9.0 `uwatec_smart_parser.c`, which decodes
  but *drops* EV_WARNING/EV_ALARM before the sample callback (remove this
  module if upstream ever emits them as SAMPLE_EVENTs). glib-only,
  unit-testable in `check_dif`. Emitted as UDDF `<alarm>error</alarm>` with
  `level="1"` (warning, yellow buzzer) / `level="2"` (alarm, red buzzer).
  Other libdivecomputer events map to `<alarm>` in `dif_sample_event_to_alarm`
  (Subsurface-compatible mapping); bookmarks become `<setmarker>`.
- `src/tests/check_dif.c` — Check-framework tests; write `test_simple.uddf`,
  `test.uddf` and `test_alarms.uddf`, which `make check` validates against
  the schema with xmllint.

## UDDF references

- Spec (v3.2.3): https://www.streit.cc/resources/UDDF/v3.2.3/en/index.html
- Overview / ecosystem: https://wrobell.dcmod.org/uddf/
- Vendored schemas: `xsd/uddf_3.2.3.xsd`, `xsd/uddf_3.1.0.xsd`

### Unit conventions (UDDF is strict SI)

| Element | Unit |
|---|---|
| `divetime`, `diveduration`, `passedtime` | seconds |
| `depth`, `greatestdepth` | meters |
| `temperature`, `lowesttemperature` | Kelvin |
| `tankpressure`, `tankpressurebegin` | Pascal (bar × 100000) |
| `heading` | degrees 0–359 (65535 is a "no data" sentinel — never emit) |

Other schema constraints to keep in mind:
- `id` attributes must be unique across the whole document
  (dive ids are `groupN_diveM`).
- `gasdefinitions` and `samples` require at least one child — omit the
  element entirely when empty.
- Repetition groups are formed per calendar day of the dive start.

### Other interchange formats (for reference)

UDDF is the de facto open interchange format. Alternatives: DAN DL7
(profile + medical data, mainly for DAN research submissions), the older
UDCF, Subsurface's own XML/git format, and Garmin FIT. Not currently
targeted by this project.

## Build Commands

Autotools project. Dependencies via Homebrew:

```bash
brew install check libxml2 glib libdivecomputer automake autoconf
```

```bash
./autogen.sh && ./configure   # first time only
make                          # build
make check                    # unit tests + xmllint schema validation
make clean
```

## Running

Live download (IrDA devices typically need a Linux VM; `-d` is the IrDA
address from `/proc/net/irda/discovery` or the serial device path — see
README.md for IrDA adapter chipsets, VM USB passthrough pitfalls, and
link debugging):

```bash
./src/dc2uddf -b smart -d 0x12345678 --save-dump dump.bin -o dives.uddf
```

Offline replay from a dump — fast iteration, no device needed. In this
mode `-d` selects the device *descriptor by product name*; give the exact
product because the Uwatec sample decoding is model-specific (a wrong
model yields "Invalid type bits" errors):

```bash
./src/dc2uddf -b smart -d "Uwatec Galileo Sol" --from-dump dump.bin -o dives.uddf
```

Dump options:
- `--from-dump FILE` — parse dives from a saved dive-data dump. The format
  is back-to-back Uwatec Smart records (`A5 A5 5A 5A` + LE32 length incl.
  header), the same bytes `dctool download` transfers; splitter lives in
  `src/dumpfile.c`. Composes with `-l`, `-s`, `-i`, `-t`.
- `--save-dump FILE` — during a live download, also save the raw dive
  records to FILE (replayable with `--from-dump`; flushed per record so an
  interrupted session keeps a valid prefix).
- `--dump-memory FILE` — save a full device memory image via
  `dc_device_dump()`. Different layout; NOT replayable with `--from-dump`.

Caveat: replayed datetimes use `dc_parser_new2` (no device), which skips
the live clock-drift correction from `DC_EVENT_CLOCK` — timestamps may
differ from a live download by the device's clock offset.

Other options: `-i` (initial pressure fix), `-t` (truncate after
surfacing), `-l N` (limit to N dives), `-s YYYY-MM-DD` (only dives since
date), `--invalid` (emit non-schema `<event>`/`<vendor>` debug elements —
output will NOT validate).

Validate any generated file manually with:

```bash
xmllint --noout --schema xsd/uddf_3.2.3.xsd dives.uddf
```

## libdivecomputer 0.9 API notes

- `dc_device_open()` requires an iostream parameter
- `dc_parser_new()` takes 4 parameters
- Sample callback receives a pointer to `dc_sample_value_t`
- `dc_parser_set_data()` was removed
- `DC_SAMPLE_TIME` value is in **milliseconds**
