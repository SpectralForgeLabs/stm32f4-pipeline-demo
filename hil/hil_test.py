#!/usr/bin/env python3
"""
hil_test.py
Reads STM32 boot message over UART and asserts expected buildinfo.

Usage: python3 hil_test.py --port /dev/ttyUSB0 --branch dev
                            --build-number 42 --sha deadbeef
"""
import argparse
import serial
import sys
import time

BAUD_RATE    = 115200
BOOT_TIMEOUT = 30  # seconds to wait for boot messages

EXPECTED_LINES = [
    "BOOT OK",
    "BRANCH:",
    "BUILD NUMBER:",
    "SHA:",
]

def parse_args():
    parser = argparse.ArgumentParser(description="SpectralForge HIL boot test")
    parser.add_argument("--port",         default="/dev/ttyUSB0")
    parser.add_argument("--branch",       required=True)
    parser.add_argument("--build-number", required=True)
    parser.add_argument("--sha",          required=True)
    return parser.parse_args()

def reset_device(ser):
    """
    Reset STM32 via DTR line.
    Requires DTR wired to NRST on the board.
    TODO: wire DTR to NRST and uncomment.
    """
    # ser.dtr = True   # assert reset
    # time.sleep(0.1)
    # ser.dtr = False  # release reset
    # time.sleep(0.1)
    pass

def read_boot_messages(port, timeout):
    lines = []
    print(f"Opening {port} at {BAUD_RATE} baud...")
    with serial.Serial(port, BAUD_RATE, timeout=1) as ser:
        # Reset device before reading boot messages
        # Remove manual reset once DTR is wired to NRST
        reset_device(ser)

        deadline = time.time() + timeout
        while time.time() < deadline:
            line = ser.readline().decode("utf-8", errors="replace").strip()
            if line:
                print(f"  UART << {line}")
                lines.append(line)
    return lines

def assert_boot(lines, branch, build_number, sha):
    failures = []

    def find(expected):
        return any(expected in line for line in lines)

    if not find("BOOT OK"):
        failures.append("Missing: BOOT OK")

    if not find(f"BRANCH: {branch}"):
        failures.append(f"Missing: BRANCH: {branch}")

    if not find(f"BUILD NUMBER: {build_number}"):
        failures.append(f"Missing: BUILD NUMBER: {build_number}")

    if not find(f"SHA: {sha}"):
        failures.append(f"Missing: SHA: {sha}")

    return failures

def main():
    args = parse_args()

    print("=" * 50)
    print("SpectralForge HIL Boot Test")
    print(f"  Branch:       {args.branch}")
    print(f"  Build Number: {args.build_number}")
    print(f"  SHA:          {args.sha}")
    print("=" * 50)

    lines = read_boot_messages(args.port, BOOT_TIMEOUT)

    if not lines:
        print("FAIL: No data received from UART")
        sys.exit(1)

    failures = assert_boot(lines, args.branch, args.build_number, args.sha)

    print()
    if failures:
        print("FAIL:")
        for f in failures:
            print(f"  ✗ {f}")
        sys.exit(1)
    else:
        print("PASS: All boot assertions passed.")
        sys.exit(0)

if __name__ == "__main__":
    main()