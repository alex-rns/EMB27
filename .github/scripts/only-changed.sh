# Sourced by build-all.sh and build-stm32.sh.
# only_changed reads project folders (one per line) on stdin and prints the ones to build.
# Without CHANGED_LIST (a local run, a push to main) that is all of them. On a pull request the workflow sets
# CHANGED_LIST to a file listing the paths the pull request changes: then only the folders holding one of those paths
# are printed, or all of them when the pull request changes the CI itself (.github/).
only_changed() {
  if [ -z "${CHANGED_LIST:-}" ] || grep -q '^\.github/' "$CHANGED_LIST"; then
    cat
    return
  fi
  local dir
  while IFS= read -r dir; do
    # a changed path starts with "<folder>/": exact prefix, no regex (folder names hold dots)
    awk -v p="$dir/" 'index($0, p) == 1 { found = 1 } END { exit !found }' "$CHANGED_LIST" && echo "$dir"
  done
}
