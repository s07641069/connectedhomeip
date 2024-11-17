#!/bin/bash

# Usage: ./external_signing.sh <input_binary_to_sign> <private_key>

IMGTOOL=${ZEPHYR_BASE}/../bootloader/mcuboot/scripts/imgtool.py
MCUBOOT_BIN=../../../examples/light-switch-app/telink/build/zephyr/mcuboot.bin

# Check if both arguments are provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <binary_to_sign> <private_key>"
    exit 1
fi

# Input variables
INPUT_FILE=$1
PRIVKEY=$2

# from .config:
# CONFIG_FLASH_LOAD_OFFSET=0x19000
# CONFIG_FLASH_LOAD_SIZE=0xec000
APP_OFFSET=0x19000 # Adjust as per your memory layout
APP_SIZE=0xec000

# Output files
PUBKEY="pubkey.pem"
SIGNED_PAYLOAD="signed_payload.bin"
TMP_SIG="tmp.sig"
SIGNATURE_B64="signature.b64"
SIGNED_FINAL="zephyr.signed.bin"
MERGED_OUTPUT=merged.bin

rm -rf ${SIGNED_FINAL} ${MERGED_OUTPUT}

# Cerificate Authority (CA) provides the build machine with public part of the key
# which should be embedded in the bootloader during build
openssl ec -in "$PRIVKEY" -pubout -out "$PUBKEY"

# Create the signing vector
"$IMGTOOL" sign -v 0.0.0+0 -H 0x200 -S 0x70000 --align 1 \
    --vector-to-sign payload "$INPUT_FILE" "$SIGNED_PAYLOAD"

# This step must be done only on CA side. After this it's possible to make
# the rest on build machine or sign and verify on CA side and return only signed.bin

# Generate a SHA256 digest and sign it with the private key
openssl dgst -sha256 -sign "$PRIVKEY" -out "$TMP_SIG" "$SIGNED_PAYLOAD"
openssl base64 -in "$TMP_SIG" -out "$SIGNATURE_B64"

# Sign the final binary with the base64-encoded signature
"$IMGTOOL" sign -v 0.0.0+0 -H 0x200 -S "$APP_SIZE" --align 1 \
    --fix-sig "$SIGNATURE_B64" --fix-sig-pubkey "$PUBKEY" "$INPUT_FILE" "$SIGNED_FINAL"

# Verify the signed image
"$IMGTOOL" verify -k "$PUBKEY" "$SIGNED_FINAL"

echo "Signing and verification completed successfully."

# Merge mcuboot and signed application binaries
BOOTLOADER_SIZE=$(stat -c%s "$MCUBOOT_BIN")
CURRENT_SIZE=$(stat -c%s "$SIGNED_FINAL")

if (( BOOTLOADER_SIZE > APP_OFFSET )); then
    echo "Error: Bootloader size exceeds application offset!"
    exit 1
fi

# Create padding if necessary
PADDING_SIZE=$((APP_OFFSET - BOOTLOADER_SIZE))
dd if=/dev/zero bs=1 count="$PADDING_SIZE" 2>/dev/null | tr '\0' '\377' > padding.bin

# Concatenate binaries
cat "$MCUBOOT_BIN" padding.bin "$SIGNED_FINAL" > "$MERGED_OUTPUT"

# Cleanup temporary files
rm "$PUBKEY" "$SIGNED_PAYLOAD" "$TMP_SIG" "$SIGNATURE_B64" padding.bin

echo "Merging completed successfully. Output: $MERGED_OUTPUT"
