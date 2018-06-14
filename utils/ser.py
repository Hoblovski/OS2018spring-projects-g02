from __future__ import print_function
import serial
import threading
import time

def listen(seri):
    global output_format
    while (True):
        if output_format == "hex":
            print(hex(ord(seri.read()))[2:].zfill(2), end=' ')
        else:
            print(seri.read(), end='')
        

ser = serial.Serial('/dev/ttyUSB0', 115200)
lis = ser
send = ser

threads = []
t1 = threading.Thread(target=listen, args=(lis,))
threads.append(t1) 

output_format = "ascii"
if __name__ == '__main__':
    for t in threads:
        t.setDaemon(True)
        t.start()
input_format = "hex"
while (True):
    inp = raw_input("input: ").strip()
    st = False
    if inp.split()[0] == "file":
        with open(inp.split()[1], 'rb') as fp: 
            hex_list = ["{:02x}".format(ord(c)) for c in fp.read()]
            inp = ''.join(hex_list)
        input_format = "hex"
    elif inp.split()[0] == "set":
            if inp.split()[1] == "input":
                if inp.split()[2] == "hex":
                    input_format = "hex"
                    print("change input format into HEX")
                elif inp.split()[2] == "ascii":
                    input_format = "ascii"
                    print("change input format into ASCII")
            if inp.split()[1] == "output":
                if inp.split()[2] == "hex":
                    output_format = "hex"
                    print("change output format into HEX")
                elif inp.split()[2] == "ascii":
                    output_format = "ascii"
                    print("change output format into ASCII")
            continue;
    elif inp == "start":
        inp = "00003000"
        input_format = "hex"
        st = True
    if input_format == "hex":
        while (len(inp) % 8 != 0):
            inp = inp + "0"
        print(len(inp))
        for j in range(0, len(inp) / 8):
            for i in range(4, 0, -1):
                ser.write(chr(int(inp[j*8+i*2-2:j*8+i*2], 16)))
                time.sleep(0.0001)
            time.sleep(0.0001)
        if st:
            input_format = "ascii"
    elif input_format == "ascii":
        for ch in inp:
            ser.write(ch)
            time.sleep(0.0001)
        time.sleep(0.0001)
