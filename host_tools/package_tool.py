#!/usr/bin/python3 -u

# @file package_tool
# @author Frederich Stine
# @brief host tool for packaging a feature for a fob
# @date 2023
#
# This source file is part of an example system for MITRE's 2023 Embedded
# CTF (eCTF). This code is being provided only for educational purposes for the
# 2023 MITRE eCTF competition, and may not meet MITRE standards for quality.
# Use this code at your own risk!
#
# @copyright Copyright (c) 2023 The MITRE Corporation

import argparse


# @brief Function to create a new feature package
# @param package_name, name of the file to output package data to
# @param car_id, the id of the car the feature is being packaged for
# @param feature_number, the feature number being packaged
def package(package_name, car_id, feature_number1, feature_number2, feature_number3):

    # Pad id lenth to 8 bytes
    car_id_len = len(car_id)
    car_id_pad = (8 - car_id_len) * "\0"
    f1=0x00
    f2=0x00
    f3=0x00

    if feature_number1:
        f1=0x01
    if(feature_number2):
        f2=0x02
    if(feature_number3):
        f3=0x04
    res=f1|f2|f3 #this REALLY needs to be tested

    # Create package to match defined structure on fob
    package_message_bytes = (
        b''.join([str.encode(car_id+car_id_pad),res.to_bytes(1, 'big'),str.encode("\n")])
    )

    # Write data out to package file
    # /package_dir/ is the mounted location inside the container - should not change

    print(package_message_bytes) #for testing only

    with open(f"/package_dir/{package_name}", "wb") as fhandle:
        fhandle.write(package_message_bytes)

    print("Feature packaged")


# @brief Main function
#
# Main function handles parsing arguments and passing them to program
# function.
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--package-name", help="Name of the package file", type=str, required=True,
    )
    parser.add_argument(
        "--car-id", help="Car ID", type=str, required=True,
    )
    parser.add_argument(
        "--feature-number1",
        help="Number of the feature to be packaged",
        type=int,
        required=True,
    )
    parser.add_argument(
        "--feature-number2",
        help="Number of the feature to be packaged",
        type=int,
        required=True,
    )
    parser.add_argument(
        "--feature-number3",
        help="Number of the feature to be packaged",
        type=int,
        required=True,
    )

    args = parser.parse_args()

    package(args.package_name, args.car_id, args.feature_number1,args.feature_number2,args.feature_number3)


if __name__ == "__main__":
    main()