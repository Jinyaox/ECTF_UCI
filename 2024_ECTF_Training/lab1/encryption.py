# encrypt messages (AES)
# append nonce to pair pin

# To use this function, cryptography need to be installed
# use command pip install cryptography in your virtual environment
# documentation can be found here : https://pypi.org/project/pycrypto/

import os
import json
import ctypes
from pathlib import Path

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
    def __init__(self, path, key=b'abcdabcdabcdabcd',option=2):
        if option==1:
            with open(path) as f:
                content=f.read()
            content=content[:16].encode()
            self.key=content
        else:
            self.key=key

    def encrypt(self, message):
        message_bytes = str.encode(message)
        return encryption(self.key, message_bytes)
    
    def encrypt_byte(self,message_byte):
        return encryption(self.key, message_byte)

    def decrypt(self, cipher_text):
        return decryption(self.key, cipher_text)