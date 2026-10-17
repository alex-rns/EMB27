#!/usr/bin/env bash
# Build every PlatformIO project in the repo.
# Fails on a build error or on a compiler warning in our own code (build_src_flags = -Wall -Wextra).
# Local run: PIO=~/.platformio/penv/bin/pio bash .github/scripts/build-all.sh
set -u
PIO=${PIO:-pio}
fail=0
summary=""
while IFS= read -r ini; do
  dir=$(dirname "$ini")
  log=$(mktemp)
  echo "::group::$dir"
  if "$PIO" run -d "$dir" >"$log" 2>&1; then
    if grep -q "warning:" "$log"; then
      grep "warning:" "$log"
      summary="$summary"$'\n'"WARN  $dir"
      fail=1
    else
      summary="$summary"$'\n'"OK    $dir"
    fi
  else
    tail -n 40 "$log"
    summary="$summary"$'\n'"FAIL  $dir"
    fail=1
  fi
  echo "::endgroup::"
  rm -f "$log"
done < <(find module-* -name platformio.ini -not -path '*/.pio/*' | sort)
echo "$summary"
exit $fail
