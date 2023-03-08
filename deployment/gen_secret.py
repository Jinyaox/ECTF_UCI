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

import argparse
import json
import random, string
from pathlib import Path


def main():
    id_pin_pair={}
    parser = argparse.ArgumentParser()
    parser.add_argument("--secret-dirc", type=Path, required=True)
    args = parser.parse_args()
    sec_dir=args.secret_dirc
    with open(str(sec_dir)+"/global_secrets.txt","w") as f:
        f.write(''.join(random.choices(string.ascii_letters + string.digits, k=16)))
    
    #for this, we create an empty file
    with open(str(sec_dir)+'/car_secrets.json', "w") as fp:
        json.dump(id_pin_pair, fp, indent=4)
    

if __name__ == "__main__":
    main()
