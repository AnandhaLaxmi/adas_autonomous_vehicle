#!/usr/bin/env python3

import sys
import cv2
import numpy as np
import serial
import threading
import queue
import time

# ---- Fix library paths (Pi OS) ----
sys.path.append('/usr/lib/python3/dist-packages')
sys.path.append('/usr/local/lib/python3.11/dist-packages')
sys.path.append('/usr/lib/aarch64-linux-gnu')

from picamera2 import Picamera2
from ultralytics import YOLO

# ---------- CONFIG ----------
MODEL_PATH = "models/yolov8n.pt"
SERIAL_PORT = "/dev/ttyACM0"   # change if needed
BAUD_RATE = 9600

TL_CLASS_ID = 9
MIN_COLOR_PIXELS = 40
PAD = 4

STOP_HOLD_TIME = 0.8   # seconds to HOLD stop after detection
# ----------------------------

# ---------- LOAD MODEL ----------
model = YOLO(MODEL_PATH)
print("YOLO model loaded")

# ---------- CAMERA ----------
picam2 = Picamera2()
config = picam2.create_video_configuration(
    main={"size": (640, 480), "format": "RGB888"}
)
picam2.configure(config)
picam2.start()

# ---------- SERIAL ----------
ser = serial.Serial(
    SERIAL_PORT,
    BAUD_RATE,
    timeout=0,
    write_timeout=0
)

if not ser.is_open:
    raise RuntimeError("Serial port not open")

cmd_queue = queue.Queue()
last_command = None

def serial_worker():
    while True:
        cmd = cmd_queue.get()
        if cmd is None:
            break
        ser.write((cmd + "\n").encode())
        ser.flush()

threading.Thread(target=serial_worker, daemon=True).start()

# ---------- TRAFFIC LIGHT COLOR ----------
def detect_tl_color(roi):
    if roi is None or roi.size == 0:
        return None

    hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)

    red1 = cv2.inRange(hsv, (0, 70, 80), (10, 255, 255))
    red2 = cv2.inRange(hsv, (170, 70, 80), (180, 255, 255))
    yellow = cv2.inRange(hsv, (15, 80, 80), (35, 255, 255))
    green = cv2.inRange(hsv, (40, 80, 80), (85, 255, 255))

    r = np.sum(red1 > 0) + np.sum(red2 > 0)
    y = np.sum(yellow > 0)
    g = np.sum(green > 0)

    if max(r, y, g) < MIN_COLOR_PIXELS:
        return None
    if r > y and r > g:
        return "red"
    if y > g:
        return "yellow"
    return "green"

# ---------- STATE ----------
stop_latch = False
last_stop_time = 0

print(" ADAS RUNNING (press q to quit)")

try:
    while True:
        frame = picam2.capture_array()
        now = time.time()

        # ---- YOLO inference ----
        results = model(frame, imgsz=416, conf=0.4, verbose=False)
        r = results[0]
        annotated = r.plot()

        detected_stop = False
        final_status = "PATH CLEAR -> GO"

        # ---- Detection loop ----
        for box in r.boxes:
            cls = int(box.cls[0])
            name = model.names[cls].lower()

            # Traffic light
            if cls == TL_CLASS_ID:
                x1, y1, x2, y2 = box.xyxy[0].int().tolist()
                roi = frame[y1:y2, x1:x2]
                color = detect_tl_color(roi)

                if color in ("red", "yellow"):
                    detected_stop = True
                    final_status = "TRAFFIC LIGHT -> STOP"
                    break

            # Pedestrian
            elif name == "person":
                detected_stop = True
                final_status = "PEDESTRIAN -> STOP"
                break

            # Other obstacles
            elif name in ["car", "truck", "bus", "motorcycle", "bicycle"]:
                detected_stop = True
                final_status = "OBSTACLE -> STOP"
                break

        # ---- STOP LATCH LOGIC ----
        if detected_stop:
            stop_latch = True
            last_stop_time = now
        elif stop_latch and (now - last_stop_time > STOP_HOLD_TIME):
            stop_latch = False

        command = "STOP" if stop_latch else "GO"

        # ---- Send command only if changed ----
        if command != last_command:
            cmd_queue.put(command)
            last_command = command
            print("Sent to Arduino:", command)

        # ---- Display ----
        cv2.putText(
            annotated,
            final_status,
            (10, 40),
            cv2.FONT_HERSHEY_SIMPLEX,
            1,
            (0, 0, 255) if stop_latch else (0, 255, 0),
            2
        )

        cv2.imshow("ADAS", annotated)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

finally:
    cmd_queue.put(None)
    ser.close()
    picam2.stop()
    cv2.destroyAllWindows()
