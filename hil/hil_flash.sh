#!/bin/bash
# hil_flash.sh
# Fetches firmware .hex from Nexus and flashes to STM32F407 via ST-Link
#
# Usage: ./hil_flash.sh <branch> <build_number> <short_sha>
# Example: ./hil_flash.sh dev 42 deadbeef

set -e

BRANCH=$1
BUILD_NUMBER=$2
SHORT_SHA=$3

if [ -z "$BRANCH" ] || [ -z "$BUILD_NUMBER" ] || [ -z "$SHORT_SHA" ]; then
    echo "Usage: $0 <branch> <build_number> <short_sha>"
    exit 1
fi

NEXUS_URL="nexus.spectralforge.dev/repository/embedded-artifacts"
# Resolve Nexus credentials (local dev or CI)
NEXUS_PWD="${NEXUS_PWD:-${NEXUS_PASSWORD}}"
HEX_FILE="stm32-${BUILD_NUMBER}-${SHORT_SHA}.hex"
FETCH_URL="${NEXUS_URL}/${BRANCH}/${HEX_FILE}"

if [ -z "$NEXUS_PWD" ]; then
  echo "[ERROR] Nexus password not provided."
  echo "Set NEXUS_PWD (local) or NEXUS_PASSWORD (CI)."
  exit 1
fi

echo "Fetching: ${FETCH_URL}"
curl -f -u admin:${NEXUS_PWD} -o /tmp/${HEX_FILE} ${FETCH_URL}
echo "Downloaded: /tmp/${HEX_FILE}"

echo "Flashing..."
openocd \
    -f interface/stlink.cfg \
    -f target/stm32f4x.cfg \
    -c "program /tmp/${HEX_FILE} verify reset exit"

echo "Flash complete."