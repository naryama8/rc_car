import serial
import time
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot

# --- CONFIGURATION ---
SERIAL_PORT = '/dev/cu.usbserial-0001'  # CHANGE THIS to your port (macOS example)
BAUD_RATE = 115200


# ---------------------

class SerialWorker(QObject):
    """
    Worker object for handling all serial communication in a background thread.
    Inherits from QObject to use Qt's signal/slot mechanism.
    """
    # Signal to emit when a new message is received. The (str) means it carries a string.
    message_received = pyqtSignal(str)

    # Signal to emit if there's a connection error.
    connection_failed = pyqtSignal(str)

    def __init__(self):
        super().__init__()
        self.ser = None
        self.running = True

    @pyqtSlot()
    def run(self):
        """The main work loop of the thread. Connects and listens for serial data."""
        print("Serial worker thread started.")
        try:
            self.ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
            time.sleep(1)  # Give the connection time to settle

            while self.running:
                if self.ser and self.ser.in_waiting > 0:
                    line = self.ser.readline().decode('utf-8', errors='ignore').rstrip()
                    if line:
                        # A message is detected, EMIT THE SIGNAL!
                        self.message_received.emit(line)
        except serial.SerialException as e:
            error_message = f"Failed to connect to {SERIAL_PORT}: {e}"
            print(error_message)
            self.connection_failed.emit(error_message)  # Emit an error signal
        finally:
            if self.ser and self.ser.is_open:
                self.ser.close()
            print("Serial worker thread finished.")

    def stop(self):
        """Stops the listening loop."""
        self.running = False
        print("Stopping serial worker...")

    def send_message(self, message: str):
        """Sends a message to the serial port."""
        if self.ser and self.ser.is_open:
            self.ser.write((message + '\n').encode('utf-8'))
            # We don't print here to keep the worker focused on its task.
            # The GUI can print if needed.