#!/usr/bin/python3 -u

# @file gen_secret
# @author Jake Grycel
# @brief Example script to generate header containing secrets for the car
# @date 2023
#
# This source file is part of an example system for MITRE's 2023 Embedded CTF (eCTF).
# This code is being provided only for educational purposes for the 2023 MITRE eCTF
# competition,and may not meet MITRE standards for quality. Use this code at your
# own risk!
#
# @copyright Copyright (c) 2023 The MITRE Corporation

import json
import argparse
import random, string
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--car-id", type=int, required=True)
    parser.add_argument("--secret-dirc", type=Path, required=True)
    parser.add_argument("--secret-file", type=Path, required=True)
    parser.add_argument("--header-file", type=Path, required=True)
    args = parser.parse_args()

    secret = {}
    
    #generate an actual secret stored in EEPROM for encryption and decription 
    car_secret = ''.join(random.choices(string.ascii_letters + string.digits, k=16))
    secret[str(args.car_id)] = car_secret #+str(args.car_id) #this should give about 22 bytes

    sec_location=0x0

    # Save the secret file
    with open(args.secret_file, "w") as fp:
        json.dump(secret, fp, indent=4)

    # Write to header file
    with open(args.header_file, "w") as fp:
        fp.write("#ifndef __CAR_SECRETS__\n")
        fp.write("#define __CAR_SECRETS__\n\n")
        fp.write(f"#define CAR_SECRET_LOC {sec_location}\n\n")
        fp.write(f'#define CAR_ID "{args.car_id}"\n\n')
        fp.write("#endif\n")
    
    
    #make sure write this thing to deployment
    sec_dir=args.secret_dirc
    with open(str(sec_dir)+f"/{args.car_id}_sec_eprom.txt","w") as f:
        f.write(f"{secret[str(args.car_id)]}")


if __name__ == "__main__":
    main()
