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
- `src/tests/check_dif.c` — Check-framework tests; write `test_simple.uddf`
  and `test.uddf`, which `make check` validates against the schema with
  xmllint.

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

```bash
./src/dc2uddf -b smart -d "Uwatec Galileo" -o dives.uddf
```

Options: `-i` (initial pressure fix), `-t` (truncate after surfacing),
`-l N` (limit to N dives), `-s YYYY-MM-DD` (only dives since date),
`--invalid` (emit non-schema `<event>`/`<vendor>` debug elements — output
will NOT validate).

There is **no** working memory-dump (`-m`) input mode; `dumpMemory` is
hardcoded off in `dc2uddf.c`. Downloading requires the actual device (IrDA
devices typically need a Linux VM).

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
