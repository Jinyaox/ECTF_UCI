import serial
from encryption import Encrypt
# cc -fPIC -shared -o encryptlib.so encrypt.c


if __name__ == "__main__":
    # Serial connection
    SERIAL_COM_PORT = '/dev/tty.usbmodem0E23342C1' #hard coded value 
    try:
        ser = serial.Serial(SERIAL_COM_PORT, 9600, timeout=3)
    except serial.SerialException:
        sys.exit(f"Serial port {SERIAL_COM_PORT} it not available")

    enc=Encrypt(" ")
    print(enc.encrypt("abcdefg"))

    while True:
        print(ser.read(5)) #this reads from the device

    