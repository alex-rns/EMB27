# Lesson 1.8 — STM32: CubeMX, CubeIDE and the first program on the Black Pill

[← Module 1](../)

**Board:** WeAct Black Pill (STM32F411CEU6: Cortex-M4F, 512 KB flash, 128 KB RAM) · **Programmer:** ST-Link V2 clone
over SWD · **Tools:** STM32CubeMX 6.18.1, STM32CubeIDE 2.2.0 · **Library:** HAL from STM32Cube FW_F4 V1.28.3

The course program lists a debrief as lesson 1.8 and STM32 only as an optional port in module 7; this lesson
brought the STM32 forward: configure the chip in CubeMX, generate a CubeIDE project, write the loop by hand,
flash it through the ST-Link, then read the board's button.

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|
| 00 | [lecture-blink](00-lecture-blink/) | lecture demo | the KEY button on PA0 (internal pull-up) read with HAL; while it is held, the onboard LED on PC13 toggles every 101 ms; clock 100 MHz from the 25 MHz crystal |

## Working with the steps

STM32 steps are STM32CubeIDE projects, not PlatformIO ones. The step folder, the CubeMX file
(`<step>.ioc`) and the CubeIDE project carry the same name.

- **Open:** CubeIDE → File → Open Projects from File System → the step folder. Don't copy it into the workspace:
  the project then lives in the repo and the workspace only points to it.
- **Change pins or clocks:** open the `.ioc` in STM32CubeMX (File → Load Project), change, Generate Code. Keep own
  code between `/* USER CODE BEGIN … */` and `/* USER CODE END … */`: a regenerate rewrites everything else.
- **Build / flash:** CubeIDE's hammer and Run. Without the IDE window (CubeIDE closed), from the step folder:
  ```
  /Applications/STM32CubeIDE.app/Contents/MacOS/STM32CubeIDE --launcher.suppressErrors -nosplash \
    -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
    -data ~/STM32CubeIDE/workspace -import "$PWD" -build 00-lecture-blink/Debug
  STM32_Programmer_CLI -c port=SWD -w Debug/00-lecture-blink.elf -v -rst
  ```
  `STM32_Programmer_CLI` comes with CubeIDE (inside its plugins) and with STM32CubeProgrammer; it is not on the PATH.
- **In git:** the `.ioc`, `Core/`, `Drivers/` (the HAL CubeMX copies in, so a clone builds without CubeMX),
  the linker scripts, `.project`, `.cproject`, `.mxproject`, `.settings/`. Not in git: `Debug/` (build output and the
  makefiles CubeIDE writes), `*.launch` (debug settings with absolute paths; CubeIDE re-creates them).
  CubeMX writes `Core/` with Windows line endings: `.gitattributes` stores them with LF.
- **CI** runs the repo checks on these files but doesn't build STM32 steps yet (`build-all.sh` builds PlatformIO
  projects only).
