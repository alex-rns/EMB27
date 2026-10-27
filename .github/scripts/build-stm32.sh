#!/usr/bin/env bash
# Build every STM32CubeIDE project in the repo (a folder with a .cproject) without CubeIDE:
# the same arm-none-eabi-gcc flags CubeIDE 2.2.0 uses for its Debug build, plus -Wextra for our code.
# Fails on a build error or on a compiler warning in Core/ (our code and CubeMX's); the HAL in Drivers/ is
# compiled without warning flags, like the framework in PlatformIO steps.
# Local run, with the compiler inside CubeIDE:
#   ARM_GCC_BIN="$(ls -d /Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/*gnu-tools-for-stm32*macos*/tools/bin)" \
#     bash .github/scripts/build-stm32.sh
set -u
BIN=${ARM_GCC_BIN:+$ARM_GCC_BIN/}
CC="${BIN}arm-none-eabi-gcc"
CXX="${BIN}arm-none-eabi-g++"
SIZE="${BIN}arm-none-eabi-size"

ARCH=(-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard --specs=nano.specs)
DEFS=(-DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE)
INCS=(-ICore/Inc -IDrivers/STM32F4xx_HAL_Driver/Inc -IDrivers/STM32F4xx_HAL_Driver/Inc/Legacy
      -IDrivers/CMSIS/Device/ST/STM32F4xx/Include -IDrivers/CMSIS/Include)
COMMON=("${ARCH[@]}" "${DEFS[@]}" "${INCS[@]}" -O0 -g3 -ffunction-sections -fdata-sections)
CFLAGS=(-std=gnu11 "${COMMON[@]}")
CXXFLAGS=(-std=gnu++14 "${COMMON[@]}" -fno-exceptions -fno-rtti -fno-use-cxa-atexit)
OURS=(-Wall -Wextra)

fail=0
summary=""

build() {  # $1 = project folder; prints the log, returns non-zero on an error or a warning in Core/
  local dir=$1 out log src obj cpp=0 err=0
  out=$(mktemp -d)
  log="$out/log"
  # CubeMX writes the chip into the project; every step so far is the Black Pill's STM32F411CE.
  if ! grep -q 'STM32F411xE' "$dir/.cproject"; then
    echo "not an STM32F411xE project: update build-stm32.sh" >"$log"; cat "$log"; return 1
  fi
  (
    cd "$dir" || exit 1
    for src in $(find Core/Src -name '*.c' -o -name '*.cpp' | sort); do
      obj="$out/${src//\//_}.o"
      case "$src" in
        *.cpp) "$CXX" "${CXXFLAGS[@]}" "${OURS[@]}" -c "$src" -o "$obj" || exit 1 ;;
        *)     "$CC" "${CFLAGS[@]}" "${OURS[@]}" -c "$src" -o "$obj" || exit 1 ;;
      esac
    done
    for src in $(find Drivers -name '*.c' | sort); do
      "$CC" "${CFLAGS[@]}" -c "$src" -o "$out/${src//\//_}.o" || exit 1
    done
    for src in $(find Core/Startup -name '*.s'); do
      "$CC" "${ARCH[@]}" -g3 -DDEBUG -x assembler-with-cpp -c "$src" -o "$out/${src//\//_}.o" || exit 1
    done
  ) >"$log" 2>&1 || err=1
  if [ $err -eq 0 ]; then
    find "$dir/Core/Src" -name '*.cpp' | grep -q . && cpp=1
    local linker=$CC libs=(-lc -lm)
    [ $cpp -eq 1 ] && linker=$CXX && libs=(-lc -lm -lstdc++ -lsupc++)
    "$linker" -o "$out/firmware.elf" "$out"/*.o "${ARCH[@]}" -T"$dir/STM32F411CEUX_FLASH.ld" \
      --specs=nosys.specs -Wl,--gc-sections -static -Wl,--start-group "${libs[@]}" -Wl,--end-group >>"$log" 2>&1 || err=1
  fi
  cat "$log"
  [ $err -eq 0 ] && "$SIZE" "$out/firmware.elf"
  [ $err -eq 0 ] && grep -q "warning:" "$log" && err=2
  rm -rf "$out"
  return $err
}

while IFS= read -r cproject; do
  dir=$(dirname "$cproject")
  echo "::group::$dir"
  build "$dir"
  case $? in
    0) summary="$summary"$'\n'"OK    $dir" ;;
    2) summary="$summary"$'\n'"WARN  $dir"; fail=1 ;;
    *) summary="$summary"$'\n'"FAIL  $dir"; fail=1 ;;
  esac
  echo "::endgroup::"
done < <(find module-* -name .cproject -not -path '*/Debug/*' | sort)
echo "$summary"
exit $fail
