import sys
import os
import math
from PyQt5.QtGui import QFontDatabase
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QPushButton
from PyQt5 import uic
from PyQt5.QtCore import Qt, QThread, pyqtSlot

from serialthread import SerialWorker
from controller import ControllerWorker
import asset

class RCCarGUI(QMainWindow):
    def __init__(self):
        """
        Initializes the main window.
        """

        super(RCCarGUI, self).__init__()

        self.load_custom_font()
        uic.loadUi("ui_files/RC_Controller.ui", self)

        # --- NEW QT-COMPLIANT THREADING SETUP ---
        self.thread = QThread()
        self.worker = SerialWorker()
        self.worker.moveToThread(self.thread)
        self.thread.started.connect(self.worker.run)
        self.worker.message_received.connect(self.handle_serial_message)
        self.worker.connection_failed.connect(self.handle_connection_error)
        self.thread.start()

        self.axis_states = {0: 0.0, 1: 0.0} # We'll track Left Stick X (axis 0) and Y (axis 1)
        self.movement_states = {'w': False, 'a': False, 'd': False, 's': False}

        # Standard thread setup
        self.controller_thread = QThread()
        self.controller_worker = ControllerWorker()
        self.controller_worker.moveToThread(self.controller_thread)

        # --- KEY STEP: Connect the worker's signals to the MainWindow's slots ---
        self.controller_worker.joystick_moved.connect(self.handle_joystick_movement) # Connect to your handler

        self.controller_thread.started.connect(self.controller_worker.run)
        self.controller_thread.start()



        # WASD Keys
        self.pushButton_4.pressed.connect(self.click) # w
        self.pushButton_4.released.connect(self.release)
        self.pushButton_2.pressed.connect(self.click) # a
        self.pushButton_2.released.connect(self.release)
        self.pushButton_3.pressed.connect(self.click) # d
        self.pushButton_3.released.connect(self.release)
        self.pushButton.pressed.connect(self.click) # s
        self.pushButton.released.connect(self.release)

        # Predefined Movements
        self.pushButton_5.clicked.connect(self.predefined) # left 90
        self.pushButton_7.clicked.connect(self.predefined) # left 180
        self.pushButton_6.clicked.connect(self.predefined) # right 90
        self.pushButton_8.clicked.connect(self.predefined) # right 180

        self.controller_worker.button_pressed.connect(self.handle_button_press)


    def load_custom_font(self):
        """Loads the custom font and verifies its success."""
        font_path = 'ui_files/Assets/Font/Micro5-Regular.ttf'

        if not os.path.exists(font_path):
            print(f"Error: Font file not found at '{font_path}'")
            return

        font_id = QFontDatabase.addApplicationFont(font_path)

        if font_id < 0:
            print(f"Error: Could not load font from '{font_path}'.")
            return

        font_families = QFontDatabase.applicationFontFamilies(font_id)
        if font_families:
            self.custom_font_family = font_families[0]
            print(f"Successfully loaded font '{self.custom_font_family}'")
        else:
            print("Error: Could not retrieve font family name after loading.")

    @pyqtSlot(str)
    def handle_serial_message(self, message):
        """
        This function is activated automatically whenever a message is received.
        It runs safely in the main GUI thread.
        """
        print(f"{message}")
        # Update your GUI here based on the message
        # Example: self.statusLabel.setText(f"Received: {message}")

        if message == "hold":
            print("ACTION: Obstacle detected!")
            self.pushButton.setText("OBSTACLE DETECTED!\nParking Mode Active")
            self.pushButton.setStyleSheet("QPushButton {background-color: red; color: white; border-radius: 30px; font-size: 30px; font-family:Micro 5;font-weight: bold;font-style: normal;}")
        elif message == "free":
            print("ACTION: Free!")
            self.pushButton.setText("S")
            self.pushButton.setStyleSheet(
                "QPushButton {background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(218, 0, 0, 200), stop:1 rgba(205, 157, 0, 200)); color: white; border-radius: 30px; font-size: 123px; font-family:Micro 5;font-weight: bold;font-style: normal;}")

    @pyqtSlot(str)
    def handle_connection_error(self, error_message):
        """Handles connection failure."""
        self.statusBar().showMessage(error_message, 0) # Show permanently
        # You could also disable buttons here, etc.

    def keyPressEvent(self, event):
        if event.isAutoRepeat():
            return
        key = event.key()

        if key == Qt.Key_W:
            self.worker.send_message("w")
            self.pushButton_4.setDown(True)
            print("w")
        elif key == Qt.Key_A:
            self.worker.send_message("a")
            self.pushButton_2.setDown(True)
            print("a")
        elif key == Qt.Key_D:
            self.worker.send_message("d")
            self.pushButton_3.setDown(True)
            print("d")
        elif key == Qt.Key_S:
            self.worker.send_message("s")
            self.pushButton.setDown(True)
            print("s")

    
    def keyReleaseEvent(self, event):
        key = event.key()

        if key == Qt.Key_W:
            self.worker.send_message("stopw")
            self.pushButton_4.setDown(False)
            print("stopw")
        elif key == Qt.Key_A:
            self.worker.send_message("stopa")
            self.pushButton_2.setDown(False)
            print("stopa")
        elif key == Qt.Key_D:
            self.worker.send_message("stopd")
            self.pushButton_3.setDown(False)
            print("stopd")
        elif key == Qt.Key_S:
            self.worker.send_message("stops")
            self.pushButton.setDown(False)
            print("stops")


    def click(self):
        button = self.sender()
        print(button)
        if button.objectName() == "pushButton_4":
            self.worker.send_message("w")
            print("w")
        elif button.objectName() == "pushButton_2":
            self.worker.send_message("a")
            print("a")
        elif button.objectName() == "pushButton_3":
            self.worker.send_message("d")
            print("d")
        elif button.objectName() == "pushButton":
            self.worker.send_message("s")
            print("s")

    def release(self):
        button = self.sender()
        if button.objectName() == "pushButton_4":
            self.worker.send_message("stopw")
            print("stopw")
        elif button.objectName() == "pushButton_2":
            self.worker.send_message("stopa")
            print("stopa")
        elif button.objectName() == "pushButton_3":
            self.worker.send_message("stopd")
            print("stopd")
        elif button.objectName() == "pushButton":
            self.worker.send_message("stops")
            print("stops")

    def predefined(self):
        button = self.sender()
        if button.objectName() == "pushButton_5":
            self.worker.send_message("l90")
            print("l90")
        if button.objectName() == "pushButton_7":
            self.worker.send_message("l180")
            print("l180")
        if button.objectName() == "pushButton_6":
            self.worker.send_message("r90")
            print("r90")
        if button.objectName() == "pushButton_8":
            self.worker.send_message("r180")
            print("r180")

    def handle_button_press(self, button_id):
        if button_id == 0:
            self.worker.send_message("r180")
            print("r180")
        if button_id == 1:
            self.worker.send_message("r90")
            print("r90")
        if button_id == 2:
            self.worker.send_message("l90")
            print("l90")
        if button_id == 3:
            self.worker.send_message("l180")
            print("l180")

    def handle_joystick_movement(self, axis, value):

        # Add a "deadzone" to prevent drift from sensitive joysticks
        if abs(value) < 0.:
            value = 0.0

        # Update the state for the axis that moved
        if axis in self.axis_states:
            self.axis_states[axis] = value

        prevmovement_states = self.movement_states.copy()

        angle_radians = math.atan2(-self.axis_states[1], self.axis_states[0])
        joystick_angle = math.degrees(angle_radians)
        if joystick_angle < 0:
            joystick_angle = 360 + joystick_angle

        magnitude = math.sqrt(self.axis_states[1] ** 2 + self.axis_states[0] ** 2)
        DEAD_ZONE = 0.35  # You can tune this value

        if joystick_angle < 22.5 or joystick_angle > 337:
            self.movement_states['d'] = True
            self.movement_states['w'] = False
            self.movement_states['a'] = False
            self.movement_states['s'] = False
            self.pushButton_3.setDown(True)
            self.pushButton_2.setDown(False)
            self.pushButton.setDown(False)
            self.pushButton_4.setDown(False)
        elif joystick_angle > 67.5 and joystick_angle < 112.5:
            self.movement_states['w'] = True
            self.movement_states['a'] = False
            self.movement_states['s'] = False
            self.movement_states['d'] = False
            self.pushButton_4.setDown(True)
            self.pushButton_2.setDown(False)
            self.pushButton_3.setDown(False)
            self.pushButton.setDown(False)
        elif joystick_angle > 22.5 and joystick_angle < 67.5:
            self.movement_states['w'] = True
            self.movement_states['d'] = True
            self.movement_states['a'] = False
            self.movement_states['s'] = False
            self.pushButton_4.setDown(True)
            self.pushButton_3.setDown(True)
            self.pushButton_2.setDown(False)
            self.pushButton.setDown(False)
        elif joystick_angle > 112.5 and joystick_angle < 157.5:
            self.movement_states['w'] = True
            self.movement_states['a'] = True
            self.movement_states['d'] = False
            self.movement_states['s'] = False
            self.pushButton_4.setDown(True)
            self.pushButton_2.setDown(True)
            self.pushButton_3.setDown(False)
            self.pushButton.setDown(False)
        elif joystick_angle > 157.5 and joystick_angle < 202.5:
            self.movement_states['w'] = False
            self.movement_states['a'] = True
            self.movement_states['d'] = False
            self.movement_states['s'] = False
            self.pushButton_2.setDown(True)
            self.pushButton_4.setDown(False)
            self.pushButton_3.setDown(False)
            self.pushButton.setDown(False)
        elif joystick_angle > 202.5 and joystick_angle < 247.5:
            self.movement_states['w'] = False
            self.movement_states['a'] = True
            self.movement_states['d'] = False
            self.movement_states['s'] = True
            self.pushButton_2.setDown(True)
            self.pushButton.setDown(True)
            self.pushButton_4.setDown(False)
            self.pushButton_3.setDown(False)
        elif joystick_angle > 247.5 and joystick_angle < 292.5:
            self.movement_states['w'] = False
            self.movement_states['a'] = False
            self.movement_states['d'] = False
            self.movement_states['s'] = True
            self.pushButton.setDown(True)
            self.pushButton_4.setDown(False)
            self.pushButton_2.setDown(False)
            self.pushButton_3.setDown(False)
        elif joystick_angle > 292.5 and joystick_angle < 337.5:
            self.movement_states['w'] = False
            self.movement_states['a'] = False
            self.movement_states['d'] = True
            self.movement_states['s'] = True
            self.pushButton_3.setDown(True)
            self.pushButton.setDown(True)
            self.pushButton_4.setDown(False)
            self.pushButton_2.setDown(False)

        if math.sqrt(self.axis_states[1]**2 + self.axis_states[0]**2) <= DEAD_ZONE:
            self.movement_states['w'] = False
            self.movement_states['a'] = False
            self.movement_states['s'] = False
            self.movement_states['d'] = False
            self.pushButton_3.setDown(False)
            self.pushButton.setDown(False)
            self.pushButton_4.setDown(False)
            self.pushButton_2.setDown(False)

        for key in prevmovement_states:
            if prevmovement_states[key] != self.movement_states[key]:
                if self.movement_states[key]:
                    self.worker.send_message(key)
                    print(key)
                else:
                    message = "stop" + key
                    self.worker.send_message(message)
                    print(message)


# This is the main execution block
if __name__ == '__main__':
    # Create the application instance
    app = QApplication(sys.argv)

    # Create and show the main window
    window = RCCarGUI()
    window.show()  # It's common practice to call show() here

    # Start the event loop and ensure a clean exit
    sys.exit(app.exec_())