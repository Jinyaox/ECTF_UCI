#!/usr/bin/python3 -u

# @file pair_tool
# @author Kenta, Yiran Wang
# @brief host tool for pairing a new key fob
# @date 2023

import json
import socket
import argparse

# @brief Function to send commands to pair
# a new fob.
# @param unpairmed_fob_bridge, bridged serial connection to unpairmed fob
# @param pairmed_fob_bridge, bridged serial connection to pairmed fob
# @param pair_pin, pin used to pair a new fob
def pair(unpaired_fob_bridge, paired_fob_bridge, pin):

    # Connect to both sockets for serial
    unpaired_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    unpaired_sock.connect(("ectf-net", int(unpaired_fob_bridge)))

    paired_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    paired_sock.connect(("ectf-net", int(paired_fob_bridge)))

    # Send pair commands to both fobs
    unpaired_sock.send(b"pair\n")
    paired_sock.send(b"pair"+bytes(pin, 'utf-8')+b"\n")

    # Set timeout for if pairing fails
    unpaired_sock.settimeout(5)
    
    # Try to receive data - if failed, pairing failed
    try:
        pair_success = unpaired_sock.recv(6)
        print("Fob paired")

    except socket.timeout:
        print("Failed to pair fob, please reset both fobs")

    return 0


# @brief Main function
#
# Main function handles parsing arguments and passing them to pair
# function.
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--unpaired-fob-bridge",
        help="Bridge for the unpaired fob",
        type=int,
        required=True,
    )
    parser.add_argument(
        "--paired-fob-bridge",
        help="Bridge for the paired fob",
        type=int,
        required=True,
    )
    parser.add_argument(
        "--pair-pin", help="Program PIN", type=str, required=True,
    )

    args = parser.parse_args()

    pair(args.unpaired_fob_bridge,args.paired_fob_bridge,args.pair_pin)


if __name__ == "__main__":
    main()
