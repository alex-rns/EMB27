"""Voltage after PWM + RC, measured by ESP32-S3.

Run: python plot.py --port COM5
Install: python -m pip install -r requirements.txt
"""

import argparse
from collections import deque

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import serial


SAMPLE_PERIOD_S = 0.02
WINDOW_SECONDS = 12
MAX_POINTS = int(WINDOW_SECONDS / SAMPLE_PERIOD_S) + 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True, help="For example COM5")
    parser.add_argument("--baud", type=int, default=115200)
    args = parser.parse_args()

    try:
        connection = serial.Serial(args.port, args.baud, timeout=0)
    except serial.SerialException as exc:
        raise SystemExit(f"Cannot open {args.port}: {exc}") from exc

    times = deque(maxlen=MAX_POINTS)
    voltages = deque(maxlen=MAX_POINTS)
    pending = bytearray()
    sample_number = 0

    fig, ax = plt.subplots(figsize=(10, 5))
    line, = ax.plot([], [], label="Voltage after RC", linewidth=2)
    ax.set(title="ESP32-S3: PWM -> RC -> ADC",
           xlabel="Time (s)", ylabel="Voltage (V)",
           xlim=(0, WINDOW_SECONDS), ylim=(0, 3.3))
    ax.grid(True, alpha=0.3)
    ax.legend(loc="upper right")

    def update(_):
        nonlocal pending, sample_number
        pending.extend(connection.read(connection.in_waiting))

        # Keep incomplete lines until the next update; skip boot messages.
        while b"\n" in pending:
            record, _, pending = pending.partition(b"\n")
            if not record.startswith(b"voltage:"):
                continue
            try:
                voltage = float(record.split(b":", 1)[1])
            except ValueError:
                continue

            times.append(sample_number * SAMPLE_PERIOD_S)
            voltages.append(voltage)
            sample_number += 1

        if times:
            line.set_data(list(times), list(voltages))
            left = max(0, times[-1] - WINDOW_SECONDS)
            ax.set_xlim(left, left + WINDOW_SECONDS)
        return (line,)

    animation = FuncAnimation(fig, update, interval=10,
                              cache_frame_data=False)
    try:
        plt.tight_layout()
        plt.show()
    finally:
        connection.close()
    _ = animation


if __name__ == "__main__":
    main()
