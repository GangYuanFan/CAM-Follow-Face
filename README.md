# CAM-Follow-Face

1. About the project.
I used python(dlib, serial, cv2, numpy, imageio, imutils) to detect face and transmit packages to SLAVE Arduino-board via Virture Com Port(VCP).
Then using UART in arduino, transmitting packages between SLAVE Arduino-board & MASTER Arduino-board.
MASTER Arduino-board takes over the controll power of CAM, and also transmitting packages to Processing IDE via VCP too.
Processing IDE will show face-animations at the relative position.

2. How to do?
First, using pip install python-toolkits(dlib, serial, cv2, numpy, imageio, imutils). You can download dlib 68-landmarks model in my google cloud: https://drive.google.com/open?id=1F1sfq1o4gNrnJQMuxK3rFo19cLAxEtQQ
Second, using two Arduino UNO board, one is Master, the other is Slave board.
In Slave board: check the COM-PORT, and edit python-code @sp.port = 'COM?'.(In Windows. If Mac or Ubuntu, using /dev/tty.?)
Connection: Slave-board pin-10 to Master-board pin-11, Slave-board pin-11 to Master-board pin-10, and GND also.
In Master board: check the COM-PORT, and edit processing-code @portName = 'COM?'.(In Windows. If Mac or Ubuntu, using /dev/tty.?)
Connection: yaw-servo attach at 9, pitch-servo attach at 6, JoySticker(x:A1, y:A2, z:3), User-Button at 2.

3. Enjoy the Face follower.
