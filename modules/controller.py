import sys
import os
os.environ["SDL_VIDEODRIVER"] = "dummy"
import pygame
from PyQt5.QtWidgets import QApplication, QMainWindow, QTextEdit, QVBoxLayout, QWidget, QLabel
from PyQt5.QtCore import QThread, pyqtSignal, QObject, Qt

# 1. MODIFY THE WORKER CLASS
class ControllerWorker(QObject):
    """
    This worker's only job is to watch for Pygame events and emit signals.
    It should not contain any application logic.
    """
    # --- Define a new signal that will carry the joystick data (axis ID, value) ---
    joystick_moved = pyqtSignal(int, float)

    # A simple signal for status messages
    controller_message = pyqtSignal(str)

    button_pressed = pyqtSignal(int)
    button_released = pyqtSignal(int)

    def __init__(self):
        super().__init__()
        self.running = True

    def run(self):
        pygame.init()
        pygame.joystick.init()

        joystick_count = pygame.joystick.get_count()
        if joystick_count == 0:
            self.controller_message.emit("No controller detected.")
            return

        joystick = pygame.joystick.Joystick(0)
        joystick.init()
        self.controller_message.emit(f"Controller detected: {joystick.get_name()}")


        while self.running:
            for event in pygame.event.get():
                if event.type == pygame.JOYAXISMOTION:
                    self.joystick_moved.emit(event.axis, event.value)
                elif event.type == pygame.JOYBUTTONDOWN:
                    self.button_pressed.emit(event.button)
                elif event.type == pygame.JOYBUTTONUP:
                    self.button_released.emit(event.button)

                # You can add signals for buttons too
                # if event.type == pygame.JOYBUTTONDOWN:

            pygame.time.wait(10)

        pygame.quit()
