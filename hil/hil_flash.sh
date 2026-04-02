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
REPO=$4

if [ -z "$BRANCH" ] || [ -z "$BUILD_NUMBER" ] || [ -z "$SHORT_SHA" ] || [ -z "$REPO" ]; then
    echo "Usage: $0 <branch> <build_number> <short_sha> <repo>"
    exit 1
fi

SAFE_BRANCH="${GITHUB_REF_NAME//\//_}"
NEXUS_URL="nexus.spectralforge.dev/repository/embedded-artifacts"
# Resolve Nexus credentials (local dev or CI)
NEXUS_PWD="${NEXUS_PWD:-${NEXUS_PASSWORD}}"
TAR_FILE="stm32-${SAFE_BRANCH}-${BUILD_NUMBER}-${SHORT_SHA}.tar"
HEX_FILE="stm32-${SAFE_BRANCH}-${BUILD_NUMBER}-${SHORT_SHA}.hex"
FETCH_URL="${NEXUS_URL}/${REPO}/${BRANCH}"

if [ -z "$NEXUS_PWD" ]; then
  echo "[ERROR] Nexus password not provided."
  echo "Set NEXUS_PWD (local) or NEXUS_PASSWORD (CI)."
  exit 1
fi

echo "Fetching: ${FETCH_URL}/${TAR_FILE}"
curl -f -u admin:${NEXUS_PWD} -o /tmp/${TAR_FILE} ${FETCH_URL}/${TAR_FILE}
echo "Downloaded: /tmp/${TAR_FILE}"
echo "Extracting firmware..."
mkdir -p /tmp/firmware
tar -xf /tmp/${TAR_FILE} -C /tmp/firmware
echo "Firmware extracted to /tmp/firmware"
ls -l /tmp/firmware

echo "Flashing..."
openocd \
    -f interface/stlink.cfg \
    -f target/stm32f4x.cfg \
    -c "program /tmp/${HEX_FILE} verify reset exit"

echo "Flash complete."