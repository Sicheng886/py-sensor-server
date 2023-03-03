from fastapi import FastAPI
import serial
import time

import uvicorn

# set up the serial line
ser = serial.Serial('/dev/ttyACM0', 9600)
time.sleep(2)


app = FastAPI()


def get_cpu_temp():
    tempFile = open("/sys/class/thermal/thermal_zone0/temp")
    cpu_temp = tempFile.read()
    tempFile.close()
    return float(cpu_temp)/1000


@app.get("/")
async def root():
    # Read and record the data
    temp = 0
    humi = 0
    for i in range(3):
        b = ser.readline()         # read a byte string
        string_n = b.decode()      # decode byte string into Unicode
        string = string_n.rstrip()  # remove \n and \r
        data = string.split(",")
        temp += float(data[0])/100
        humi += float(data[1])/100

        time.sleep(0.1)            # wait (sleep) 0.1 seconds
    temp /= 3
    humi /= 3
    cpu_temp = get_cpu_temp()
    timeData = int(time.time())
    return {"temp": temp, "humi": humi, "time": timeData, "cpu": cpu_temp}

if __name__ == "__main__":
    uvicorn.run('main:app', port=3001,
                reload=True, log_level="info")
