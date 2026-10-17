#!/usr/bin/env bash
# Repo checks that need no toolchain: English only, valid Wokwi circuits, no Wi-Fi secrets in git.
set -u
fail=0

echo "== Cyrillic in tracked text files (the repo is English only)"
if git ls-files -z | xargs -0 perl -CSD -ne 'BEGIN { @ARGV = grep { -T } @ARGV } if (/\p{Cyrillic}/) { print "$ARGV:$.: $_"; $f = 1 } close ARGV if eof; END { exit($f ? 1 : 0) }'; then
  echo "none"
else
  fail=1
fi

echo "== diagram.json is valid JSON"
while IFS= read -r f; do
  python3 -m json.tool "$f" > /dev/null || { echo "BAD JSON: $f"; fail=1; }
done < <(git ls-files '*diagram.json')
echo "checked $(git ls-files '*diagram.json' | wc -l | tr -d ' ') files"

echo "== no secrets.h in git (only secrets.example.h)"
if git ls-files | grep -E '(^|/)secrets\.h$'; then
  fail=1
else
  echo "none"
fi

exit $fail
