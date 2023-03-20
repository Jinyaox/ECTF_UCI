
car_id='1'
car_id_pad = (8 - len(car_id)) * "\0"

f1=0x00
f2=0x00
f3=0x00

feature_number1=False
feature_number2=True
feature_number3=True

if feature_number1:
    f1=0x01
if(feature_number2):
    f2=0x02
if(feature_number3):
    f3=0x04
res=f1|f2|f3

package_message_bytes = (
    b''.join([res.to_bytes(1, 'big'),str.encode(car_id+car_id_pad),str.encode("\n")])
)

print(package_message_bytes)

with open(f"../package", "rb") as fhandle:
    message = fhandle.read()

print(len(message))
print(message)

