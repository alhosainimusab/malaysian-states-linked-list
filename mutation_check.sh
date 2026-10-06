#!/usr/bin/env bash
# Introduces deliberate bugs into a copy of the library and checks that the
# unit tests catch every one of them. Run from the repository root.
set -u
CC=${CC:-gcc}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

mutants=(
  "flip sort comparison|s/cmp <= 0 : cmp >= 0/cmp >= 0 : cmp <= 0/"
  "delete first partial match|s/if (partial > 1)/if (0)/"
  "empty query matches all|s/return 0; \/\* an empty query/return 1; \/* an empty query/"
  "skip duplicate check|s/return LIST_ERR_DUPLICATE;/;/"
  "case-sensitive compare|s/ca = (unsigned char)tolower((unsigned char)\*a++);/ca = (unsigned char)*a++;/"
  "off-by-one length limit|s/strlen(name) >= MAX_NAME/strlen(name) > MAX_NAME/"
  "0-based search positions|s/matches = 0, pos = 1;/matches = 0, pos = 0;/"
)

survived=0
for m in "${mutants[@]}"; do
  name=${m%%|*}; expr=${m#*|}
  sed "$expr" src/states_list.c > "$TMP/states_list.c"
  if cmp -s src/states_list.c "$TMP/states_list.c"; then
    echo "ERROR    $name (mutation did not apply)"; survived=$((survived + 1)); continue
  fi
  cp src/states_list.h "$TMP/"
  $CC -std=c11 -I"$TMP" tests/test_states_list.c "$TMP/states_list.c" -o "$TMP/t" 2>/dev/null
  if "$TMP/t" > /dev/null; then
    echo "SURVIVED $name"; survived=$((survived + 1))
  else
    echo "killed   $name"
  fi
done
echo "$survived mutant(s) survived"
exit $((survived > 0))
