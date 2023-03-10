# encrypt messages (AES)
# append nonce to pair pin

# To use this function, cryptography need to be installed
# use command pip install cryptography in your virtual environment
# documentation can be found here : https://pypi.org/project/pycrypto/

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


# # key and nonce must be stored in a file
# key = Fernet.generate_key()
# nonce = os.urandom(4)
     
# class Encrypt:
#     def __init__(self):
#         if not os.path.isfile('key.json'):
#             with open('key.json', 'w') as f:
#                 json.dump(key.hex(), f)
#         if not os.path.isfile('nonce.json'):
#             with open('nonce.json', 'w') as f:
#                 json.dump(nonce.hex(), f)
#         self._fernet = Fernet(key)
    
#     def encrypt(self, message):
#         message_bytes = str.encode(message) + nonce # append 4 bytes nonce to message
#         pad = (16 - (len(message_bytes) % 16)) * '0' # padding with 0
#         plain_text = str.encode(pad) + message_bytes
#         return self._fernet.encrypt(plain_text)
    
#     def decrypt(self, cipher_text):
#         return self._fernet.decrypt(cipher_text)

#     # fails to update nonce
#     def update_nonce(self):
#         global nonce
#         nonce = os.urandom(4)
#         with open('nonce.json', 'w') as f:
#             json.dump(nonce.hex(), f)
