# encrypt messages (AES)
# append nonce to pair pin

# To use this function, cryptography need to be installed
# use command pip install cryptography in your virtual environment
# documentation can be found here : https://pypi.org/project/pycrypto/

# Fernet encrypt given data using AES
# https://cryptography.io/en/latest/fernet/
from cryptography.fernet import Fernet

key = Fernet.generate_key()

class Encrypt:
    def __init__(self):
        self._fernet = Fernet(key)
    
    def encrypt(self, plain_text):
        return self._fernet.encrypt(plain_text)
    
    def decrypt(self, cipher_text):
        return self._fernet.decrypt(cipher_text)