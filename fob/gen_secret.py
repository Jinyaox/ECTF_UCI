#!/usr/bin/python3 -u

# @file gen_secret
# @author Jake Grycel
# @brief Example script to generate header containing secrets for the fob
# @date 2023
#
# This source file is part of an example system for MITRE's 2023 Embedded CTF (eCTF).
# This code is being provided only for educational purposes for the 2023 MITRE eCTF
# competition, and may not meet MITRE standards for quality. Use this code at your
# own risk!
#
# @copyright Copyright (c) 2023 The MITRE Corporation

import json
import argparse
import random, string
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--car-id", type=int)
    parser.add_argument("--secret-dirc", type=Path, required=True)
    parser.add_argument("--pair-pin", type=str)
    parser.add_argument("--secret-file", type=Path)
    parser.add_argument("--header-file", type=Path)
    parser.add_argument("--paired", action="store_true")
    args = parser.parse_args()
    sec_location=0x0
    glob_sec_location=0x10

    id_pin_pair={}

    if args.paired:
        # Write to header file
        with open(args.header_file, "w") as fp:
            fp.write("#ifndef __FOB_SECRETS__\n")
            fp.write("#define __FOB_SECRETS__\n\n")
            fp.write("#define PAIRED 1\n")
            fp.write(f'#define CAR_ID "{args.car_id}"\n')
            fp.write(f"#define AES_SECRET_LOC {sec_location}\n\n")
            fp.write(f"#define HOST/FOB_SECT {glob_sec_location}\n\n")
            fp.write("#endif\n")
        
        sec_dir=args.secret_dirc
        with open(str(sec_dir)+f"id_pin_pair.txt","a") as f:
            f.write('\n'+str(args.car_id)+",",)

        with open(str(sec_dir)+'car_secrets.json', "r") as fp:
            id_pin_pair = json.load(fp)
        
        id_pin_pair[str(args.car_id)]=str(hash(str(args.pair_pin)))
        
        with open(str(sec_dir)+'car_secrets.json', "w") as fp:
            json.dump(id_pin_pair, fp, indent=4)
        
    else:
        # Write to header file
        with open(args.header_file, "w") as fp:
            fp.write("#ifndef __FOB_SECRETS__\n")
            fp.write("#define __FOB_SECRETS__\n\n")
            fp.write("#define PAIRED 0\n")
            fp.write('#define CAR_ID "000000"\n')
            fp.write(f"#define AES_SECRET_LOC {sec_location}\n\n")
            fp.write(f"#define HOST/FOB_SECT {glob_sec_location}\n\n")
            fp.write("#endif\n")
        
        sec_dir=args.secret_dirc
        with open(str(sec_dir)+f"Unpaired_fob_sec_eprom.txt","w") as f:
            f.write(''.join(random.choices("0", k=16)))


if __name__ == "__main__":
    main()
