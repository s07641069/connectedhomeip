#!/usr/bin/env python3
#
#    Copyright (c) 2022-2025 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

"""
This script aims to merge factory data and dac cert firmware into one data block 
because Telink BDT requires 4k aligned during programming.

Example Usage for TL321x:
python3 scripts/tools/telink/merge_factorydata_dac.py \
~/connectedhomeip/factory_data_1141_8005_0x1f9000/1141_8005/11418005202501100001/factory_data_ext.bin \
~/connectedhomeip/factory_data_1141_8005_0x1f9000/1141_8005/11418005202501100001/dac_cert_key.bin \
0x800 \
~/connectedhomeip/factory_data_1141_8005_0x1f9000/1141_8005/11418005202501100001/merged_factorydata_dac.bin

Expected Result:
Firmware merged successfully into /home/ubuntu/connectedhomeip/factory_data_1141_8005_0x1f9000/1141_8005/11418005202501100001/merged_factorydata_dac.bin
"""

import argparse


def merge_factorydata_dac(factory_data_ext_path, dac_cert_key_path,
                          dac_cert_start_address, output_path):
    # Read the content of the factory_data_ext.bin
    with open(factory_data_ext_path, 'rb') as f:
        factory_data_ext = f.read()

    # Read the content of the dac_cert_key.bin
    with open(dac_cert_key_path, 'rb') as f:
        dac_cert_key = f.read()

    # Determine the size of the gap to fill with 0xFF
    gap_size = dac_cert_start_address - len(factory_data_ext)

    if gap_size < 0:
        raise ValueError("factory_data_ext.bin is too large to fit in the specified address space")

    # Fill the gap with 0xFF
    gap = b'\xFF' * gap_size

    # Construct the final merged data
    merged_data = factory_data_ext + gap + dac_cert_key

    # Write the merged data to the output file
    with open(output_path, 'wb') as f:
        f.write(merged_data)

    print(f"Firmware merged successfully into {output_path}")


def main():
    # Setup argument parser
    parser = argparse.ArgumentParser(description='Merge firmware files.')
    parser.add_argument('factory_data_ext_path', type=str,
                        help='Path to the factory_data_ext.bin file')
    parser.add_argument('dac_cert_key_path', type=str,
                        help='Path to the dac_cert_key.bin file')
    parser.add_argument('dac_cert_start_address', type=lambda x: int(x, 0),
                        help='Start address for dac_cert_key.bin (e.g., 0x800)')
    parser.add_argument('output_path', default="merged_factorydata_dac.bin",
                        type=str, help='Path to the output merged firmware file')

    # Parse the arguments
    args = parser.parse_args()

    # Call the merge function with the provided arguments
    merge_factorydata_dac(args.factory_data_ext_path, args.dac_cert_key_path,
                          args.dac_cert_start_address, args.output_path)


if __name__ == '__main__':
    main()
