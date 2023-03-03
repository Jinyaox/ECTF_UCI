# encrypt messages (AES)
# append nonce to pair pin

# To use this function, cryptography need to be installed
# use command pip install cryptography in your virtual environment
# documentation can be found here : https://pypi.org/project/pycrypto/

import os
import json

# Fernet encrypt given data using AES-CBC
# https://cryptography.io/en/latest/fernet/
from cryptography.fernet import Fernet

# key and nonce must be stored in a file
key = Fernet.generate_key()
nonce = os.urandom(4)
     
class Encrypt:
    def __init__(self):
        if not os.path.isfile('key.json'):
            with open('key.json', 'w') as f:
                json.dump(key.hex(), f)
        if not os.path.isfile('nonce.json'):
            with open('nonce.json', 'w') as f:
                json.dump(nonce.hex(), f)
        self._fernet = Fernet(key)
    
    def encrypt(self, message):
        message_bytes = str.encode(message) + nonce # append 4 bytes nonce to message
        pad = (16 - (len(message_bytes) % 16)) * '0' # padding with 0
        plain_text = str.encode(pad) + message_bytes
        return self._fernet.encrypt(plain_text)
    
    def decrypt(self, cipher_text):
        return self._fernet.decrypt(cipher_text)

    # fails to update nonce
    def update_nonce(self):
        global nonce
        nonce = os.urandom(4)
        with open('nonce.json', 'w') as f:
            json.dump(nonce.hex(), f)
