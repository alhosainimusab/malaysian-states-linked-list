# Changelog

## [2.0.0] - 2026-10 - Post-submission rework (Musaab Fahmi Fadhl Al-Husaini)

### Fixed
- "Ascending/Descending" options sorted by the hard-coded array order instead of alphabetically; newly inserted states were pushed to the top.
- Deleting with an empty line removed the first node (Johore); short input such as `a` removed the first partial match (Kelantan).
- Non-numeric menu input caused an infinite loop.
- Memory was never freed on exit.
- `strcasecmp` was used without a declaration and is POSIX-only; replaced with a portable helper.
- Input longer than the buffer spilled into the next prompt.

### Changed
- Bubble sort with `strcpy` swaps replaced by merge sort that relinks nodes (O(n log n), stable).
- Delete now prefers an exact match and deletes a partial match only when it is unique; ambiguous input lists the candidates.
- Code split into a list library (`src/states_list.c/.h`, no printing) and a CLI (`src/main.c`).
- Menu gained "Display Current Order"; Exit moved to option 7.
- Startup no longer prints 16 "State inserted" lines.

### Added
- Validation: empty names, names over 49 characters and duplicates (case-insensitive) are rejected.
- Clean exit on end-of-file; `--no-color` flag and `NO_COLOR` support.
- 83 unit-test checks, AddressSanitizer/UBSan run, mutation check (7 deliberate bugs, all caught).
- Makefile, GitHub Actions CI, screenshots from real terminal runs, `.gitignore`, `.gitattributes`.

## [1.0.0] - 2025 - Course submission

- Group submission for BIC10404 Data Structures, UTHM, Semester 2 2024/2025. Code preserved in `original/submitted_version.c`.
