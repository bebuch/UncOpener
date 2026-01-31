## General working rules (apply to every step)

* One step = **one commit**. Update this plan.
* Each commit must include:
  * All relevant **unit tests** (new/updated).
  * All checks passing locally (format, tidy, build, tests).
* Keep implementation incremental: a usable baseline early, then harden behavior and expand features.

---

## Plan (steps with checkboxes)

### Step 0 — Basic setup

* [x] Create a .gitginore
* [x] Create a Readme.md with the project description
* [x] Add MIT License

### Step 1 — Repository skeleton and build presets

* [x] Create a modern CMake project skeleton (`C++17`, Qt-based app target + core library target).
* [x] Add `CMakePresets.json` for:

  * `dev` (Debug), `rel` (Release)
  * clang toolchain usage by default (platform-specific clang is fine).
* [x] Enable strict warnings and **warnings-as-errors** for all targets (compiler- and platform-appropriate).
* [x] Add a minimal "hello Qt" app window (no functionality yet) to validate Qt + toolchain wiring.
* [x] Add a minimal unit test executable (even if only a placeholder test).

### Step 2 — Code hygiene tooling (must be in place early)

* [x] Add `.clang-format` and enforce formatting via a CMake target (e.g., `format` / `format-check`).
* [x] Add `.clang-tidy` and integrate clang-tidy into CMake (toggleable via preset option, but enabled for dev/CI).
* [x] Add basic `.editorconfig`.
* [x] Add `cmake-format`/`cmakelint` only if desired (optional), but at least ensure CMake style is consistent.
* [x] Add `CTest` integration and make `ctest` work from presets.
* [x] Add a "run all checks" script (`tools/`) for local use (format-check + tidy + build + tests).

### Step 3 — Define the "URL contract" and test vectors (backend-owned spec)

* [x] Create a `docs/url-contract.md` capturing the rules derived from the extension:

  * Extension produces: `scheme://server/share/path...` (no `scheme:server/...`).
  * No percent-encoding performed by extension; backend must decode if present.
  * Preserve trailing slash.
  * Collapse repeated slashes, remove `.` segments, reject `..` segments always.
  * Reject non-UNC inputs; accept authority edge cases; restriction via UNC allow-list.
* [x] Add unit tests with comprehensive vectors:

  * Valid examples (including spaces and `#` in path segments).
  * Invalid examples (`..`, missing `//`, missing authority, non-UNC structure).
  * Trailing slash preservation cases.
* [x] Decide and document **query/fragment behavior** (ignore vs append) and test it.

### Step 4 — Core library: parsing + normalization + validation

(Plain C++ library, no Qt UI logic; Qt types allowed if preferred, but keep it testable.)

* [x] Implement `ParseResult` / `ValidationResult` types with reason + remediation strings for dialogs.
* [x] Implement:

  * parse input argument as URL
  * enforce `scheme://...`
  * percent-decode
  * normalization pipeline (collapse slashes, remove `.`, reject `..`, preserve trailing slash)
  * canonical UNC conversion for comparison: `\\authority\path...`
* [x] Unit test everything, including tricky separators and edge characters.

### Step 5 — Security policy: UNC allow-list + filetype allow/deny lists

* [x] Implement case-insensitive UNC allow-list matching:

  * Convert scheme URL → UNC (as above)
  * `startsWith` any allow-list entry (no wildcards)
  * Allow-list entries auto-normalize to backslashes; reject entries containing `/`.
* [x] Implement filetype policy:

  * Mode: whitelist or blacklist, mutually exclusive.
  * Case-insensitive "ends-with string" comparison on the *final open target* string.
  * Reject list entries containing `/` or `\`.
  * Default: whitelist selected but empty; switching mode disabled if the active list is non-empty.
* [x] Unit tests for allow-list and filetype rules.

### Step 6 — Config: schema, location, persistence

* [x] Define config keys (format chosen by implementer, but keys stable):

  * `schemeName`
  * `uncAllowList[]`
  * Linux-only: `smbUsername` (optional, global)
  * filetype policy: `mode` + `whitelist[]` + `blacklist[]`
* [x] Implement per-user config location:

  * Windows: `%APPDATA%/UncOpener/…`
  * Linux: `$XDG_CONFIG_HOME/uncopener/…` fallback `~/.config/…`
* [x] Implement atomic save + safe load (defaults if missing).
* [x] Unit tests around config parsing/validation of lists (forbidden `/` and `\` entries, etc.).

### Step 7 — Platform open-target mapping

* [x] Windows: after validation, convert to final UNC path string and open via `QDesktopServices::openUrl(...)`.
* [x] Linux: build SMB URL:

  * `smb://[username@]server/share/path`
  * Encode username (`DOMAIN\user` via standard URL encoding).
  * Preserve trailing slash.
* [x] Ensure the allow-list check always happens against the UNC form (as specified), then open the OS-specific target.
* [x] Unit tests for SMB URL generation and trailing slash behavior.

### Step 8 — Qt GUI: configuration editor (no scheme registration yet)

* [ ] GUI layout includes:

  * Scheme name
  * UNC allow-list editor
  * Filetype mode selector + list editors + switching constraints
  * Linux-only SMB username field
  * Current config path display (recommended)
* [ ] Validation in GUI with immediate feedback (forbidden characters in lists).
* [ ] Save/apply semantics: explicit “Save” (recommended) or auto-save, but document behavior.
* [ ] Add GUI-level tests if feasible; otherwise keep logic in core and unit-tested there.

### Step 9 — Handler mode UX: strict error dialogs + success flash

* [ ] In one-argument mode:

  * Run validation pipeline.
  * On any error: show modal error dialog (English) with:

    * input
    * reason
    * remediation
    * exit after confirmation, return code 1
  * On success:

    * call `QDesktopServices::openUrl`
    * on failure: error dialog as above, exit 1
    * on success: show icon on green background for 1 second, exit 0
* [ ] Add tests for error classification (unit tests) and minimal integration smoke test if possible.

### Step 10 — Scheme registration: Windows (HKCU)

* [ ] Implement “is registered”:

  * check per-user registration points to **current binary**.
* [ ] Implement register/deregister actions.
* [ ] If another binary is registered:

  * GUI prompts user before overwrite.
* [ ] Add unit tests for registry path computation and decision logic (mock registry access layer).

### Step 11 — Scheme registration: Linux (`.desktop` + xdg-mime)

* [ ] Create per-user `.desktop` file in the standard location (`~/.local/share/applications`).
* [ ] Register via `xdg-mime default <desktop-file> x-scheme-handler/<scheme>`.
* [ ] Deregister removes the `.desktop` file and updates association as needed.
* [ ] “Is registered” detection:

  * check existence and contents of desktop file point to current binary
  * verify association files/state (as per “check the files” requirement)
* [ ] Tests for desktop file generation and detection logic.

### Step 12 — Icons pipeline: SVG → platform assets, committed outputs

* [ ] Add `icon.svg` to repo root (or `assets/`).
* [ ] Add scripts to generate:

  * Windows icon assets (ICO) and app icons
  * Linux icons (PNG sizes as needed for AppImage/desktop)
  * In-app icon usage; use SVG directly where possible.
* [ ] For PNG optimization:

  * script calls optimizer `--help` first (for traceability)
  * run optimizer with maximal settings
* [ ] Generated assets are committed to git and used by build/install packaging.
* [ ] Add a `tools/generate-icons` script and document usage in `docs/`.

### Step 13 — Packaging: Windows ZIP + Linux AppImage

* [ ] Windows:

  * Produce a ZIP containing the executable and required Qt runtime files.
  * Document how scheme registration works for a “portable” ZIP install (still per-user).
* [ ] Linux:

  * Provide AppImage build scripts.
  * Use Docker-based build targeting an older baseline for wider compatibility (as you noted).
  * Ensure Qt plugins are bundled correctly.
* [ ] Add release artifact naming conventions and reproducible build notes.

### Step 14 — CI (added late, as requested)

* [ ] GitHub Actions workflows:

  * PR: build + unit tests on Windows + Linux
  * Run `clang-format` check
  * Run `clang-tidy`
  * Run coverage (at least Linux; publish report artifact)
  * Enforce warnings-as-errors
* [ ] Tag/release workflow:

  * Build Windows ZIP artifact
  * Build Linux AppImage artifact
  * Upload artifacts to GitHub Release
* [ ] Keep scheme registration tests mocked in CI (no real registry/xdg changes on runners).

### Step 15 — Upload to GitHub repo `bebuch/UncOpener`

* [ ] Ensure repository name, README, and links mention it is the backend for the UncClickable extension.
* [ ] Push to the existing remote: `https://github.com/bebuch/UncOpener` (remote already set).
* [ ] Verify default branch protections and PR requirements align with the CI checks.

---

## Minimal “Definition of Done” for the first usable milestone

* [ ] User can configure scheme + UNC allow-list in GUI.
* [ ] Scheme can be registered/deregistered (Windows + Linux) and status is shown.
* [ ] Clicking an extension-generated link triggers handler mode:

  * strict validation
  * allow-list enforcement
  * filetype policy enforcement
  * opens UNC (Windows) / smb (Linux)
  * correct error dialogs and success flash behavior.

If you want to lock down the last ambiguity before implementation starts, decide and document (in Step 3) whether query/fragment are ignored or appended, and how “Windows-valid characters” is evaluated for them.
