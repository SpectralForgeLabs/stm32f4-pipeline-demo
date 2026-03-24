#!/usr/bin/env python3
"""
hil_test.py
SpectralForge HIL Boot Test
Resets STM32, waits for boot, sends STATUS command, parses Spectral response.

Usage: python3 hil_test.py --port /dev/ttyUSB0 --branch dev
                            --build-number 42 --sha deadbeef
"""
import argparse
import serial
import sys
import time

# ─── Config ───────────────────────────────────────────────────────────────────
BAUD_RATE       = 115200
BOOT_WAIT_SEC   = 5       # seconds to wait after reset before sending command
READ_TIMEOUT    = 3       # seconds to wait for response after sending command

# Spectral protocol constants
FRAME_SOF  = 0xAA
FRAME_EOF  = 0xBB
CMD_STATUS = 0x03

# STATUS command frame: AA AA 01 03 02 BB BB
STATUS_CMD = bytes([0xAA, 0xAA, 0x01, 0x03, 0x02, 0xBB, 0xBB])

# ─── Args ─────────────────────────────────────────────────────────────────────
def parse_args():
    parser = argparse.ArgumentParser(description="SpectralForge HIL boot test")
    parser.add_argument("--port",         default="/dev/ttyUSB0")
    parser.add_argument("--branch",       required=True)
    parser.add_argument("--build-number", required=True)
    parser.add_argument("--sha",          required=True)
    return parser.parse_args()

# ─── Reset hook ───────────────────────────────────────────────────────────────
def reset_device(ser):
    """
    Reset STM32 via DTR line.
    Requires DTR wired to NRST on the board.
    TODO: wire DTR to NRST and uncomment the lines below.
    """
    print("  [RESET] Triggering device reset...")
    # ser.dtr = True
    # time.sleep(0.1)
    # ser.dtr = False
    # time.sleep(0.1)
    print("  [RESET] Reset hook — manual reset required (DTR not yet wired)")

# ─── CRC ──────────────────────────────────────────────────────────────────────
def compute_crc(payload):
    """XOR CRC: LEN byte + all payload bytes"""
    crc = len(payload)
    for b in payload:
        crc ^= b
    return crc & 0xFF

# ─── Frame parser ─────────────────────────────────────────────────────────────
def parse_spectral_frame(raw_bytes):
    """
    Hunt for a valid Spectral frame in raw bytes.
    Frame: AA AA LEN PAYLOAD CRC BB BB
    Returns payload bytes or None.
    """
    i = 0
    while i < len(raw_bytes) - 6:
        if raw_bytes[i] == FRAME_SOF and raw_bytes[i+1] == FRAME_SOF:
            length = raw_bytes[i+2]
            if length == 0 or length > 253:
                i += 1
                continue
            frame_end = i + 3 + length + 3  # SOF(2)+LEN(1)+DATA+CRC(1)+EOF(2)
            if frame_end > len(raw_bytes):
                i += 1
                continue
            payload  = raw_bytes[i+3 : i+3+length]
            crc      = raw_bytes[i+3+length]
            eof1     = raw_bytes[i+3+length+1]
            eof2     = raw_bytes[i+3+length+2]
            if eof1 != FRAME_EOF or eof2 != FRAME_EOF:
                i += 1
                continue
            expected_crc = compute_crc(payload)
            if crc != expected_crc:
                print(f"  [WARN] CRC mismatch: got {crc:02X} expected {expected_crc:02X}")
                i += 1
                continue
            return bytes(payload)
        i += 1
    return None

# ─── Status payload parser ────────────────────────────────────────────────────
def parse_status_payload(payload):
    """
    Parse STATUS response payload.
    Payload: CMD_ID(1) + ASCII "branch buildnum sha"
    Returns (branch, build_number, sha) or None.
    """
    if not payload or payload[0] != CMD_STATUS:
        return None
    try:
        info_str = payload[1:].decode("utf-8").strip()
        parts    = info_str.split()
        if len(parts) < 3:
            print(f"  [WARN] Status payload too short: '{info_str}'")
            return None
        return parts[0], parts[1], parts[2]
    except Exception as e:
        print(f"  [WARN] Failed to decode status payload: {e}")
        return None

# ─── Main test ────────────────────────────────────────────────────────────────
def run_hil_test(port, branch, build_number, sha):
    print(f"Opening {port} at {BAUD_RATE} baud...")

    with serial.Serial(port, BAUD_RATE, timeout=0.1) as ser:

        # 1. Reset device
        reset_device(ser)

        # 2. Wait for boot
        print(f"  [BOOT] Waiting {BOOT_WAIT_SEC}s for device to boot...")
        boot_buf = bytearray()
        deadline = time.time() + BOOT_WAIT_SEC
        while time.time() < deadline:
            chunk = ser.read(256)
            if chunk:
                boot_buf.extend(chunk)
                try:
                    for line in chunk.decode("utf-8", errors="replace").splitlines():
                        line = line.strip()
                        if line:
                            print(f"  UART << {line}")
                except Exception:
                    pass

        # 3. Send STATUS command
        print(f"\n  [CMD] Sending STATUS command: {STATUS_CMD.hex(' ').upper()}")
        ser.write(STATUS_CMD)
        ser.flush()

        # 4. Read response
        print(f"  [CMD] Waiting {READ_TIMEOUT}s for response...")
        resp_buf = bytearray()
        deadline = time.time() + READ_TIMEOUT
        while time.time() < deadline:
            chunk = ser.read(256)
            if chunk:
                resp_buf.extend(chunk)

    print(f"\n  Response hex: {resp_buf.hex(' ').upper() if resp_buf else '(none)'}")

    # 5. Parse frame
    payload = parse_spectral_frame(resp_buf)
    if payload is None:
        print("FAIL: No valid Spectral frame in response")
        return False

    # 6. Parse status
    result = parse_status_payload(payload)
    if result is None:
        print("FAIL: Could not parse STATUS payload")
        return False

    got_branch, got_build, got_sha = result
    print(f"\n  Parsed STATUS response:")
    print(f"    Branch:       {got_branch}")
    print(f"    Build Number: {got_build}")
    print(f"    SHA:          {got_sha}")
    print()

    # 7. Assert
    failures = []
    if got_branch != branch:
        failures.append(f"Branch:       got '{got_branch}' expected '{branch}'")
    if got_build != build_number:
        failures.append(f"Build Number: got '{got_build}' expected '{build_number}'")
    if got_sha != sha:
        failures.append(f"SHA:          got '{got_sha}' expected '{sha}'")

    if failures:
        print("FAIL:")
        for f in failures:
            print(f"  ✗ {f}")
        return False

    print("PASS: All assertions passed.")
    return True

# ─── Entry point ──────────────────────────────────────────────────────────────
def main():
    args = parse_args()

    print("=" * 50)
    print("SpectralForge HIL Boot Test")
    print(f"  Branch:       {args.branch}")
    print(f"  Build Number: {args.build_number}")
    print(f"  SHA:          {args.sha}")
    print("=" * 50)
    print()

    success = run_hil_test(args.port, args.branch, args.build_number, args.sha)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()