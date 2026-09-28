# 📘 DIARY.md

## Architectural Decision Log

*This document serves as a chronological record of architectural decisions, trade-offs, and reasoning throughout the project's lifecycle. Just as Git tracks code changes, this diary tracks the **why** behind the code.*

---

## 📋 Decision Index

| Date | Decision Area | Choice | Status |
|------|---------------|--------|--------|
| 2026-09-06 | Language & Stack | C99 with GNU extensions | ✅ Confirmed |
| 2026-09-06 | Architecture Pattern | Feature-first vertical slices | ✅ Confirmed |
| 2026-09-06 | Modular Decomposition | Model/Repository/Service | ✅ Confirmed |
| 2026-09-06 | Error Handling | Unified PowerStatus enum | ✅ Confirmed |
| 2026-09-06 | Observability | Structured logging with levels | ✅ Confirmed |
| 2026-09-06 | Build System | GNU Make with standard targets | ✅ Confirmed |
| 2026-09-27 | Feature Implementation: Interactive Menu | dmenu-protocol launcher with fallback chain | ✅ Confirmed |
| 2026-09-28 | Built-in Menu | Optional Xlib + Xft window behind `MENU=builtin` | ✅ Confirmed |
| 2026-09-28 | Menu Confirmation | Second launcher pass for destructive actions | ✅ Confirmed |

---

## 📝 Decision Entries

### Decision 1: Language & Stack Selection

**Date:** 2026-09-23  
**Status:** Confirmed

---

#### Context / Background

The original `power.c` was a monolithic 500+ line file written in C. The refactoring needed to preserve the existing functionality while making the codebase modular, maintainable, and professional. I needed to choose a language and compiler standard that balanced portability, performance, and modern features.

---

#### Options Considered

**Option A: C89 with strict ISO compliance**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Maximum portability across all compilers <br> • Works on embedded systems and older UNIX variants |
| **Disadvantages** | • No `//` comments <br> • No `stdint.h` <br> • No inline functions <br> • No variable declarations in for loops |
| **Implementation Difficulty** | Easy (but restrictive) |
| **Fit with Constraints** | Too restrictive – we want modern C features while staying lightweight |

**Option B: C99 with GNU extensions (`-std=gnu99`)**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Modern features: `//` comments, `stdint.h`, for-loop declarations <br> • GNU extensions enable `__VA_ARGS__` macros for logging <br> • `_GNU_SOURCE` exposes all POSIX functions (sync, reboot) <br> • Widely supported (gcc, clang) |
| **Disadvantages** | • Not strictly ISO-compliant <br> • May not compile on non-GCC compilers (e.g., MSVC) |
| **Implementation Difficulty** | Easy |
| **Fit with Constraints** | Perfect – Linux-only tool, GCC/clang assumed |

**Option C: C11**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Modern standard with threading support <br> • Better type safety |
| **Disadvantages** | • Less widely adopted than C99 <br> • Features we don't need (threading, atomics) <br> • Still requires GNU extensions for some POSIX functions |
| **Implementation Difficulty** | Medium |
| **Fit with Constraints** | Overkill – C99+gnu99 gives us everything we need |

---

#### Decision & Rationale

**Chosen Option:** C99 with GNU extensions (`-std=gnu99`)

**Reasoning:**

I chose C99 with GNU extensions because:

1. **Features we need** – `//` comments, `stdint.h`, for-loop declarations, variadic macros (`__VA_ARGS__`) for logging.
2. **POSIX compatibility** – `_GNU_SOURCE` exposes `sync()`, `reboot()`, `strerror()`, and other POSIX functions without extra feature test macros.
3. **Compiler support** – Both GCC and Clang support `-std=gnu99` across all Linux distros.
4. **Simplicity** – No need to write preprocessor hacks for missing features.
5. **Portfolio consideration** – C is a good choice for system-level tools; it's widely understood and respected.

**Trade-offs accepted:**
- **Non-standard compliance** – The tool is Linux-only, so relying on GNU extensions is acceptable.
- **Compilation warnings** – The `-Wpedantic` flag warns about zero-argument variadic macros, but we handle these with `##__VA_ARGS__` and the warnings are harmless.

---

#### Implementation Notes

- Use `-std=gnu99 -D_GNU_SOURCE` in `CFLAGS`.
- Use `-Wall -Wextra -Wpedantic` for rigorous checking.
- Use `#define _GNU_SOURCE` only in the Makefile, not in source files, to avoid redefinition warnings.
- All source files use `/* */` comments for consistency (except `//` in headers, which is allowed in C99).

---

#### References

- [GCC C Dialect Options](https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html)
- [POSIX.1-2008 Specification](https://pubs.opengroup.org/onlinepubs/9699919799/)

---

#### Review / Update Log

| Date | Update | Author |
|------|--------|--------|
| 2026-09-06 | Initial decision | deltaog-117 |

---

### Decision 2: Architecture Pattern (Feature-First vs Layer-First)

**Date:** 2026-09-06  
**Status:** Confirmed

---

#### Context / Background

The original `power.c` was a single file with all logic mixed together. The refactoring needed to organise the code in a way that was maintainable, testable, and aligned with professional standards. The key question was: top-level directories by technical layers (controllers, models, services) or by business capabilities (features)?

The constraints:
- Must pass the "Delete Test" – deleting a feature folder should not break other features.
- Must allow for easy addition of new power operations (e.g., hibernate).
- Must be understandable to new developers.

---

#### Options Considered

**Option A: Layer-First Architecture**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Familiar to many developers (MVC style) <br> • Clear separation of concerns (models, services, repositories) |
| **Disadvantages** | • Violates the "Delete Test" – a feature is spread across multiple directories <br> • Adding a new feature requires editing files in multiple places <br> • High coupling between features (models are shared) |
| **Implementation Difficulty** | Easy (common pattern) |
| **Fit with Constraints** | Poor – does not pass the Delete Test |

**Option B: Feature-First Architecture (Vertical Slices)**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Each feature is self-contained (model, service, repository inside the feature folder) <br> • Passes the Delete Test – delete a feature folder, app still works <br> • New features are isolated; no ripple effects <br> • Encourages loose coupling |
| **Disadvantages** | • Less familiar to developers used to MVC <br> • Some duplication of shared code (e.g., logging) |
| **Implementation Difficulty** | Medium |
| **Fit with Constraints** | Excellent – fully supports modularity and the Delete Test |

**Option C: Hybrid Approach**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Shared infrastructure in a `shared/` folder <br> • Features are top-level, but can import from `shared/` <br> • Balances modularity and code reuse |
| **Disadvantages** | • Requires discipline to keep `shared/` read-only <br> • Slightly more complex than pure feature-first |
| **Implementation Difficulty** | Medium-Hard |
| **Fit with Constraints** | Good – we adopted this approach as the final design |

---

#### Decision & Rationale

**Chosen Option:** Feature-First with a `shared/` infrastructure layer (Hybrid)

**Reasoning:**

I chose the feature-first approach for these reasons:

1. **Delete Test compliance** – Deleting `src/power/` removes all power functionality but the rest of the app (logging, utilities) remains intact.
2. **Separation of concerns** – Each layer (model, service, repository) lives inside the feature folder, making it clear where each piece of logic belongs.
3. **Scalability** – Adding a new feature (e.g., `hibernate`) only requires creating a new folder and implementing its layers. No modifications to existing features.
4. **Observability** – Shared infrastructure (logging, utils) sits in `shared/` and is imported by features, but never imports back from features.

**Trade-offs accepted:**
- **Duplication** – Some code might be duplicated across features, but this is acceptable for isolation.
- **Learning curve** – Developers unfamiliar with vertical slices may need a few minutes to understand the structure.
- **`shared/` discipline** – Must enforce the rule that `shared/` never imports from features.

---

#### Implementation Notes

- Top-level directories under `src/` are features (`power/`).
- Inside `src/power/`: `model/`, `repository/`, `service/`.
- `src/shared/` contains `logging/` and `utils/` – read-only infrastructure.
- `src/main.c` is the thin entrypoint.
- Tests mirror the source structure: `tests/unit/power/`, `tests/integration/power/`.

---

#### References

- [The "Delete Test" – Yegor Bugayenko](https://www.yegor256.com/2020/10/13/delete-test.html)
- [Vertical Slice Architecture – Jimmy Bogard](https://jimmybogard.com/vertical-slice-architecture/)

---

#### Review / Update Log

| Date | Update | Author |
|------|--------|--------|
| 2026-09-06 | Initial decision | deltaog-117 |

---

### Decision 3: Modular Decomposition (Model/Repository/Service)

**Date:** 2026-09-06  
**Status:** Confirmed

---

#### Context / Background

Within the `power/` feature folder, the code needed to be further decomposed. The original `power.c` mixed parsing, system calls, and orchestration in a single file. The decision was how to split these responsibilities into distinct, testable layers.

---

#### Options Considered

**Option A: Single File with Static Functions**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Simple; no header files needed <br> • Quick to implement |
| **Disadvantages** | • Poor separation of concerns <br> • Hard to test individual parts <br> • No clear interfaces |
| **Implementation Difficulty** | Easy |
| **Fit with Constraints** | Poor – the opposite of what we're trying to achieve |

**Option B: Model/Service/Repository (chosen)**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Clear separation of responsibilities <br> • Model: data structures and parsing <br> • Repository: low-level system calls <br> • Service: orchestration and logging <br> • Each layer can be tested independently <br> • Repository can be mocked for testing |
| **Disadvantages** | • More files to maintain <br> • Slightly more boilerplate |
| **Implementation Difficulty** | Medium |
| **Fit with Constraints** | Excellent – aligns with professional engineering practices |

**Option C: Actor Model with Message Passing**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Strong decoupling <br> • Good for concurrency |
| **Disadvantages** | • Overkill for a synchronous CLI tool <br> • Adds unnecessary complexity and overhead |
| **Implementation Difficulty** | Hard |
| **Fit with Constraints** | Poor – not needed for this use case |

---

#### Decision & Rationale

**Chosen Option:** Model/Service/Repository (Option B)

**Reasoning:**

I chose the Model/Service/Repository decomposition because:

1. **Single Responsibility** – Each layer has a distinct purpose:
   - **Model** – Enums, status codes, command parsing (pure data).
   - **Repository** – Low-level system calls (suspend, poweroff, reboot, lock, logout).
   - **Service** – Orchestration: parse command, call repository, log, return status.
2. **Testability** – The repository can be mocked, allowing unit testing of the service without touching the real system.
3. **Clarity** – A new developer can look at `service/` to understand the high-level flow, then drill down into `repository/` for implementation details.
4. **Separation of concerns** – Changes to system calls (e.g., adding logind support) only affect the repository, not the service.

**Trade-offs accepted:**
- **More files** – This increases the file count, but each file is small and focused.
- **Boilerplate** – Each layer needs its own header file, but this improves the API surface.

---

#### Implementation Notes

- `src/power/model/` – `command.h`, `command.c`, `status.h`, `status.c`
- `src/power/repository/` – `system_repo.h`, `system_repo.c`
- `src/power/service/` – `power_service.h`, `power_service.c`
- `src/main.c` – thin entrypoint
- All public functions are documented with Doxygen-style comments.

---

#### References

- [Clean Architecture – Robert C. Martin](https://blog.cleancoder.com/uncle-bob/2012/08/13/the-clean-architecture.html)
- [Repository Pattern – Martin Fowler](https://martinfowler.com/eaaCatalog/repository.html)

---

#### Review / Update Log

| Date | Update | Author |
|------|--------|--------|
| 2026-09-06 | Initial decision | deltaog-117 |

---

### Decision 4: Error Handling Strategy

**Date:** 2026-09-06  
**Status:** Confirmed

---

#### Context / Background

The original `power.c` used raw `int` return codes (0 for success, 1 for failure) and `perror()` for error messages. This was inconsistent and made it hard to propagate detailed error information. A unified error handling strategy was needed.

---

#### Options Considered

**Option A: Raw Integers with `errno`**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Simple <br> • Familiar to C developers |
| **Disadvantages** | • No semantic meaning (0 = success, -1 = error) <br> • Hard to differentiate between error types <br> • Requires looking at `errno` for details <br> • Silent failures possible |
| **Implementation Difficulty** | Easy |
| **Fit with Constraints** | Poor – doesn't provide clear, actionable errors |

**Option B: Unified PowerStatus Enum**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Clear semantic meaning (`POWER_STATUS_OK`, `POWER_STATUS_ERR_NO_LOCKER`) <br> • Easy to extend with new error types <br> • Human-readable string conversion `power_status_to_string()` <br> • Works well with logging |
| **Disadvantages** | • Slightly more boilerplate than raw ints |
| **Implementation Difficulty** | Easy-Medium |
| **Fit with Constraints** | Excellent – aligns with observability and clarity principles |

**Option C: Error Callbacks / Error Context**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Allows rich error context (file, line, message) |
| **Disadvantages** | • Complex to implement in C <br> • Adds overhead (function pointers, context structs) <br> • Overkill for a simple CLI tool |
| **Implementation Difficulty** | Hard |
| **Fit with Constraints** | Poor – too complex for this project |

---

#### Decision & Rationale

**Chosen Option:** Unified PowerStatus Enum

**Reasoning:**

I chose the `PowerStatus` enum for these reasons:

1. **Semantic clarity** – Each error has a name (`POWER_STATUS_ERR_NO_LOCKER`) that describes exactly what went wrong.
2. **String conversion** – `power_status_to_string()` provides human-readable messages for logging.
3. **Extensibility** – New error types can be added without breaking existing code.
4. **Integration with logging** – The service layer logs the error using `power_status_to_string()`.
5. **No silent failures** – Every function returns a `PowerStatus` and logs errors.

**Trade-offs accepted:**
- **Enum values** – More lines of code than raw ints, but worth it for clarity.
- **No error context** – If more context is needed (e.g., filename, line number), we can add it later via logging or error chaining.

---

#### Implementation Notes

- Defined `PowerStatus` enum in `src/power/model/status.h`.
- Each function returns `PowerStatus`.
- `power_status_to_string()` provides human-readable messages.
- Service layer calls repository functions and logs the result using `LOG_ERROR("%s", power_status_to_string(status))`.
- No use of `perror()` – all errors go through the logging system.

---

#### References

- [Error Handling in C – SEI CERT C Coding Standard](https://wiki.sei.cmu.edu/confluence/display/c/MSC32-C.+Properly+check+for+errors+when+calling+functions)

---

#### Review / Update Log

| Date | Update | Author |
|------|--------|--------|
| 2026-09-06 | Initial decision | deltaog-117 |

---

### Decision 5: Observability (Logging Strategy)

**Date:** 2026-09-06  
**Status:** Confirmed

---

#### Context / Background

The original `power.c` used `printf()` for help and `perror()` for errors. There was no way to control verbosity, no timestamps, and no distinction between different types of messages. A structured logging system was needed to support debugging and observability.

---

#### Options Considered

**Option A: Raw `printf` / `perror`**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Simple <br> • No dependencies |
| **Disadvantages** | • No log levels <br> • No timestamps <br> • No way to suppress messages <br> • All output goes to stdout, making it hard to separate logs from command output |
| **Implementation Difficulty** | Easy |
| **Fit with Constraints** | Poor – doesn't support observability goals |

**Option B: Simple Logging with Macros (chosen)**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Log levels (DEBUG, INFO, WARN, ERROR, CRITICAL) <br> • Timestamps <br> • Can be suppressed via `POWER_LOG_LEVEL` <br> • Outputs to stderr (separate from stdout) <br> • No external dependencies <br> • Varargs support for formatted messages |
| **Disadvantages** | • Not JSON-structured (but can be added later) <br> • No file logging (but can be added later) |
| **Implementation Difficulty** | Easy-Medium |
| **Fit with Constraints** | Excellent – supports observability without adding complexity |

**Option C: External Library (e.g., `syslog`, `spdlog`)**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Feature-rich (file rotation, syslog integration, JSON output) |
| **Disadvantages** | • External dependency <br> • May not be available on all systems <br> • Adds compilation complexity |
| **Implementation Difficulty** | Medium |
| **Fit with Constraints** | Overkill – we want minimal dependencies for a system tool |

---

#### Decision & Rationale

**Chosen Option:** Simple Logging with Macros

**Reasoning:**

I chose the custom logging module because:

1. **Minimal dependencies** – The logger uses only `stdio.h`, `stdlib.h`, `string.h`, `time.h`, and `stdarg.h` – no external libraries.
2. **Log levels** – `DEBUG`, `INFO`, `WARN`, `ERROR`, `CRITICAL` – each with a macro (`LOG_INFO`, `LOG_ERROR`).
3. **Environment control** – `POWER_LOG_LEVEL` environment variable controls verbosity (default: INFO).
4. **Timestamped** – Every log line includes `YYYY-MM-DD HH:MM:SS`.
5. **Stderr output** – Logs go to stderr, leaving stdout clean for command output (e.g., help, future `--version`).
6. **Observability** – Logs provide a trace of what `power` attempted, making debugging easy.

**Trade-offs accepted:**
- **Not JSON-structured** – Adding JSON later would be a small change (replace `fprintf` with a JSON formatter).
- **No file logging** – Users can redirect stderr to a file if needed (`power lock 2> power.log`).

---

#### Implementation Notes

- `src/shared/logging/logger.h` – defines `LogLevel` enum, `logger_set_level()`, `log_message()`, and macros.
- `src/shared/logging/logger.c` – implements timestamp formatting, level checking, and `vfprintf` output.
- `main.c` – parses `POWER_LOG_LEVEL` and calls `logger_set_level()`.
- All output goes to stderr with `fprintf(stderr, ...)`.

---

#### References

- [OpenTelemetry Logging Specification](https://opentelemetry.io/docs/specs/otel/logs/)
- [The Twelve-Factor App – Logs](https://12factor.net/logs)

---

#### Review / Update Log

| Date | Update | Author |
|------|--------|--------|
| 2026-09-06 | Initial decision | deltaog-117 |

---

### Decision 6: Build System (Makefile Design)

**Date:** 2026-09-06  
**Status:** Confirmed

---

#### Context / Background

The original `power.c` was compiled manually with `gcc`. A professional build system was needed to support development, testing, and installation.

---

#### Options Considered

**Option A: Manual Build Script (shell script)**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Simple <br> • No learning curve |
| **Disadvantages** | • Hard to maintain <br> • No dependency tracking <br> • No `clean` or `install` targets <br> • Poor user experience |
| **Implementation Difficulty** | Easy |
| **Fit with Constraints** | Poor – not professional |

**Option B: GNU Make (chosen)**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Industry standard for C projects <br> • `all`, `clean`, `install`, `uninstall`, `test` targets <br> • Automatic dependency tracking (via `.d` files, if needed) <br> • Works on all Linux systems <br> • Simple syntax |
| **Disadvantages** | • Requires learning Makefile syntax <br> • Some complexity with automatic path generation |
| **Implementation Difficulty** | Medium |
| **Fit with Constraints** | Excellent – standard, portable, professional |

**Option C: CMake**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | • Cross-platform <br> • Supports IDE integration <br> • More powerful than Make |
| **Disadvantages** | • Adds complexity <br> • Requires CMake to be installed <br> • Overkill for a small C project |
| **Implementation Difficulty** | Hard |
| **Fit with Constraints** | Overkill – `power` is Linux-only and small |

---

#### Decision & Rationale

**Chosen Option:** GNU Make

**Reasoning:**

I chose GNU Make because:

1. **Ubiquity** – `make` is installed on every Linux system by default.
2. **Standard targets** – `all`, `clean`, `install`, `uninstall`, `test` align with professional expectations.
3. **Automatic source discovery** – `find` collects all `.c` files recursively.
4. **Parallel builds** – `make -j` works automatically.
5. **No extra dependencies** – No need to install CMake or other build tools.

**Trade-offs accepted:**
- **Complexity** – The `find` + `patsubst` logic can be tricky for beginners, but it's well-documented.
- **No incremental build** – We don't track dependencies (`.d` files) yet, but can add them later.

---

#### Implementation Notes

- `all` – builds `bin/power`.
- `clean` – removes `build/` and `bin/`.
- `install` – copies `bin/power` to `/usr/local/bin`.
- `uninstall` – removes `/usr/local/bin/power`.
- `test` – placeholder for future unit tests.
- Uses `-Wall -Wextra -Wpedantic -std=gnu99 -D_GNU_SOURCE`.
- Build objects in `build/` directory, binary in `bin/`.

---

#### References

- [GNU Make Manual](https://www.gnu.org/software/make/manual/make.html)
- [Makefile Best Practices](https://makefiletutorial.com/)

---

#### Review / Update Log

| Date | Update | Author |
|------|--------|--------|
| 2026-09-06 | Initial decision | deltaog-117 |

---

### Decision 7: Feature Implementation — Interactive Power Menu

**Date:** 2026-09-27
**Status:** ✅ Confirmed

---

#### Context / Background

`power` needed a way to trigger a pop-up menu of its six core actions (lock,
suspend, hibernate, poweroff, reboot, logout) from a window-manager keybind
(e.g. Super+Ctrl), instead of always invoking a specific subcommand. The
constraints:

- Must stay a pure CLI tool — no GUI toolkit dependency, matching the
  project's minimal, distro-agnostic philosophy.
- Must work across distros/launchers without hardcoding one specific tool.
- Must keep working once a custom launcher/bar (planned as a future,
  separate project) replaces whatever launcher is used today.

---

#### Options Considered

**Option A: dmenu/rofi/wofi as picker backend**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | No new toolkit dependency; idiomatic for tiling WMs; minimal code (shell out, read one line back) |
| **Disadvantages** | Look/feel tied to whichever launcher is installed; needs at least one dmenu-protocol launcher present |
| **Implementation Difficulty** | Easy |
| **Fit with Constraints** | Excellent — matches the CLI-first philosophy and needs no GUI dependency |

**Option B: Custom GTK3 popup window**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | Full control over look and feel; no external launcher dependency |
| **Disadvantages** | Pulls in the entire GTK3/Pango/Cairo/GLib stack (13 link-time libs, tens of MB installed); slower startup; new build dependency (`pkg-config gtk+-3.0`) |
| **Implementation Difficulty** | Medium |
| **Fit with Constraints** | Poor — contradicts the project's "stay a tiny dependency-free CLI" philosophy |

**Option C: ncurses TUI in a floating terminal**

| Aspect | Assessment |
|--------|------------|
| **Advantages** | No GUI toolkit dependency; fits a terminal-first aesthetic |
| **Disadvantages** | Requires a terminal emulator to be spawned and a floating-window WM rule for it; ncurses cannot draw outside a terminal grid |
| **Implementation Difficulty** | Medium |
| **Fit with Constraints** | Workable, but adds a terminal-emulator dependency and WM-side configuration the other options don't need |

---

#### Decision & Rationale

**Chosen Option:** A — dmenu-protocol launcher, with a fallback chain

**Reasoning:**

> I chose the dmenu-protocol approach because it keeps `power` a pure CLI
> tool with zero new dependencies at build time, and it is the standard
> pattern already used by every WM-integrated menu on Linux/Wayland.
>
> The dmenu protocol itself (read newline-separated options on stdin, print
> the chosen one on stdout) is implemented identically by dmenu, rofi
> `-dmenu`, wofi `--dmenu`, bemenu, and fuzzel `--dmenu` — so `power menu`
> does not depend on any one of them specifically. To stay distro-agnostic
> to the maximum, `system_repo_run_launcher()` tries a fallback chain
> (config-specified `menu_launcher_order`, else rofi → wofi → bemenu →
> fuzzel → dmenu), checking each candidate with `command -v` before
> attempting to launch it — the same fallback-chain pattern already used
> for `locker_order`/`logout_order`.
>
> This also directly answers the forward-compatibility question that drove
> the choice: since the planned future custom launcher only needs to
> implement the same stdin/stdout convention, `power` requires zero code
> changes when that launcher replaces rofi/dmenu — only a `menu_launcher_order`
> config update.
>
> **Trade-offs accepted:**
> - The pop-up's look and feel depends on whichever launcher is configured/available.
> - At least one dmenu-protocol launcher must be installed, or the user must configure their own.

---

#### Implementation Notes

> - New `CMD_MENU` value in `command.h`/`command.c`, mapped to the `menu` subcommand.
> - `system_repo_run_launcher()` (repository layer) owns only the "pipe options to
>   an external launcher, read back the pick" primitive — it does not know what
>   the options mean.
> - `run_menu()` (service layer, `power_service.c`) owns the six menu labels and
>   their mapping back to command names, then re-enters `power_service_execute()`
>   so lock-before-sleep, logging, and error handling stay identical to running
>   the action directly from the CLI.
> - New `menu_launcher_order` key in `power.conf`, parsed/freed with the same
>   `parse_list()` helper already used for `locker_order`/`logout_order`.
> - New `POWER_STATUS_ERR_NO_LAUNCHER` status for when no launcher is available.
> - A closed/cancelled menu (no stdout from the launcher) is not an error —
>   `run_menu()` logs it and returns `POWER_STATUS_OK` with no action taken.

---

#### References

- [dmenu(1) — the protocol every option here implements](https://tools.suckless.org/dmenu/)
- [rofi -dmenu mode](https://github.com/davatorium/rofi)

---

#### Review / Update Log

| Date | Update | Author |
|------|--------|--------|
| 2026-09-27 | Initial decision | deltaog-117 |

---

### Decision 8: Built-in Menu — Optional X11 Window

**Date:** 2026-09-28
**Status:** Confirmed

---

#### Context / Background

The launcher-based menu (Decision 7) depends on rofi, dmenu or similar being installed and looks however that tool looks. I wanted `power` to be able to show a good-looking menu on its own, without making the default binary heavier or adding dependencies.

---

#### Options Considered

**Option A: Built-in X11 menu (Xlib + Xft).** Fits X11 setups, needs only libX11/libXft at runtime; X11 only.
**Option B: Terminal menu.** No dependencies and works on Wayland too, but needs a terminal window and a floating rule.
**Option C: X11 plus native Wayland backends.** Works everywhere and is the most work; deferred.

I first asked for options that also covered Wayland, then chose A after seeing the trade-offs. Wayland is left as a roadmap item.

---

#### Decision

**Chosen:** Option A, gated behind `make MENU=builtin`, with Xft for fonts.

**Reasoning:**

> - The default build stays at ~40 KB with no new dependencies; the built-in build measured ~50 KB and links libX11, libXft and libXinerama.
> - Colours come from `power.conf`, then X resources, then defaults, so it follows any theme without configuration.
> - `builtin` is an entry in `menu_launcher_order`, so it composes with the existing fallback chain instead of adding a second selection mechanism.
> - Wayland sessions are skipped on purpose (keyboard grabs are unreliable under XWayland) and fall through to external launchers.

**Trade-offs accepted:**
- X11 only for now.
- The built-in build needs the X development packages, and Xft pulls in fontconfig and freetype.

---

#### Implementation Notes

> - `repository/menu_x11.c` owns the window, drawing and event loop; `model/menu_nav.c` holds the pure navigation logic so it can be unit-tested without X.
> - Number keys only move the highlight and Enter is the sole confirmation, so a stray key cannot poweroff the machine.
> - The Makefile builds each variant in `build/<variant>/` and copies the result to `bin/power`.
> - Not run against a live display in this session; verified by building both variants warning-free and by the `menu_nav` tests.

---

#### Review / Update Log

| Date | Update | Author |
|------|--------|--------|
| 2026-09-28 | Initial decision | deltaog-117 |

---

### Decision: Menu Confirmation for Destructive Actions

**Date:** 2026-09-28  
**Status:** Confirmed

---

#### Context

A stray pick in `power menu` could power off, reboot or log out in one step. Recoverable actions (lock, suspend, hibernate) do not carry that risk.

---

#### Options Considered

**Option A: Second launcher pass.** Reopen the same launcher with `Confirm <action>` and `Cancel`. Works with every launcher and is one code path.
**Option B: In-window confirm in the built-in menu.** Best feel, but only covers the built-in menu.
**Option C: Both.** Best experience, two code paths to maintain.

---

#### Decision

**Chosen:** Option A.

**Reasoning:**

> - Every launcher speaks the same dmenu protocol, so one implementation covers the built-in menu, rofi, wofi, bemenu, fuzzel and dmenu.
> - The "is this destructive?" rule and the label builder are pure functions in `model/menu_confirm.c`, so they are unit- and property-tested without a display.
> - The confirm row is first, so `Enter` accepts it, as requested; `Esc`, `Cancel` or a closed launcher all cancel.

**Trade-offs accepted:**
- A second window appears for destructive actions.
- Option B can still be layered on later for the built-in menu, turning this into Option C.

---

#### Implementation Notes

> - Only `power menu` confirms. Direct commands (`power poweroff`) are unchanged so scripts and keybinds keep working.
> - `menu_confirm = false` disables it; unset means on.
> - Verified with a fake launcher and a harmless logout command: confirm runs it, `Cancel`/empty cancel, `menu_confirm = false` skips the second pass, and Lock stays one step. Not exercised in a real menu window.

---

#### Review / Update Log

| Date | Update | Author |
|------|--------|--------|
| 2026-09-28 | Initial decision | deltaog-117 |
