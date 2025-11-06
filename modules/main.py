import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel
from PyQt5 import uic
from PyQt5.QtCore import Qt, QThread, pyqtSlot

from serialthread import SerialWorker


class RCCarGUI(QMainWindow):
    def __init__(self):
        """
        Initializes the main window.
        """

        super(RCCarGUI, self).__init__()
        # Load the UI from the specified path
        uic.loadUi("ui_files/rc_car_gui_1.ui", self)

        # --- NEW QT-COMPLIANT THREADING SETUP ---
        self.thread = QThread()
        self.worker = SerialWorker()
        self.worker.moveToThread(self.thread)
        self.thread.started.connect(self.worker.run)
        self.worker.message_received.connect(self.handle_serial_message)
        self.worker.connection_failed.connect(self.handle_connection_error)
        self.thread.start()

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
            self.label_3.setText("STOPP")
        elif message == "free":
            print("ACTION: Free!")
            self.label_3.setText("S")

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
            print("w")
        elif key == Qt.Key_A:
            self.worker.send_message("a")
            print("a")
        elif key == Qt.Key_D:
            self.worker.send_message("d")
            print("d")
        elif key == Qt.Key_S:
            self.worker.send_message("s")
            print("s")


    def keyReleaseEvent(self, event):
        key = event.key()

        if key == Qt.Key_W:
            self.worker.send_message("stopw")
            print("stopw")
        elif key == Qt.Key_A:
            self.worker.send_message("stopa")
            print("stopa")
        elif key == Qt.Key_D:
            self.worker.send_message("stopd")
            print("stopd")
        elif key == Qt.Key_S:
            self.worker.send_message("stops")
            print("stops")





        # --- You can connect your widgets to functions here ---
        # Example: self.my_button.clicked.connect(self.on_my_button_click)


# This is the main execution block
if __name__ == '__main__':
    # Create the application instance
    app = QApplication(sys.argv)

    # Create and show the main window
    window = RCCarGUI()
    window.show()  # It's common practice to call show() here

    # Start the event loop and ensure a clean exit
    sys.exit(app.exec_())