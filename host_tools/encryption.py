# encrypt messages (AES)
# append nonce to pair pin

import os
import json
import ctypes
from pathlib import Path

key = os.urandom(16)
with open('key.txt', 'wb') as f:
    f.write(key)

path_to_lib = str(Path.cwd() / "encryptlib.so")
encryptlib = ctypes.CDLL(path_to_lib)

# setting up functions
encryption = encryptlib.encryption
encryption.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
encryption.restype = ctypes.c_char_p

decryption = encryptlib.decryption
decryption.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
decryption.restype = ctypes.c_char_p


class Encrypt:
    def __init__(self):
        with open('key.txt', 'rb') as f:
            self.key = f.read()

    def encrypt(self, message):
        message_bytes = str.encode(message)
        pad = (16 - (len(message_bytes) % 16)) * '0'
        plain_text = str.encode(pad) + message_bytes
        return encryption(self.key, plain_text)

    def decrypt(self, cipher_text):
        return decryption(self.key, cipher_text)

