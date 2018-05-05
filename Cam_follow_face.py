import dlib
from imutils import face_utils
import numpy as np
import cv2
import serial
import imageio
fname = 'D:/DeepLearning/face/Face-LandMark_with_Dlib/facial-landmarks/shape_predictor_68_face_landmarks.dat'
detector = dlib.get_frontal_face_detector()
predictor = dlib.shape_predictor(fname)
sp = serial.Serial()
sp.port = 'COM21'
sp.baudrate = 115200
sp.timeout = 0
sp.open()
max_people = 5
output_path = 'C:/Users/Jerry/Desktop/IoT_mid_project/output/'
fourcc = cv2.VideoWriter_fourcc(*'XVID')
out = cv2.VideoWriter(output_path + 'output_1.avi', fourcc, 5.0, (640, 480))
face_gif = imageio.read(output_path[0:-7] + 'face.gif')
gifs = np.array([cv2.cvtColor(gif, cv2.COLOR_RGBA2BGRA) for gif in face_gif])
for _ in range(len(gifs)):
    for __ in range(len(gifs[_])):
        for ___ in range(len(gifs[_][__])):
            if gifs[_][__][___][3] == 0:
                gifs[_][__][___][0:3] = 0
            else:
                continue

def get_detect(gray):
    try:
        rects = detector(gray, 1)
        x = []
        y = []
        w = []
        h = []
        for (i, rect) in enumerate(rects):
            (_x, _y, _w, _h) = face_utils.rect_to_bb(rect)
            x.append(_x)
            y.append(_y)
            w.append(_w)
            h.append(_h)
        x = np.array(x)
        y = np.array(y)
        w = np.array(w)
        h = np.array(h)
        return x, y, w, h
    except:
        print("detect None")


def sendData(_tmp):     # send 3 bytes per each call
    if _tmp < 256:
        if _tmp > 99:
            sp.write(b'%d' % _tmp)
        elif _tmp > 9:
            sp.write(b'%d' % 0)
            sp.write(b'%d' % _tmp)
        else:
            sp.write(b'%d' % 0)
            sp.write(b'%d' % 0)
            sp.write(b'%d' % _tmp)
    return


img_counter = 1
video_counter = 1
gif_counter = 0
if __name__ == '__main__':
    cap = cv2.VideoCapture(0)
    print(sp.readline())
    while True:
        ret, img = cap.read()
        if ret:
            try:
                gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
                img = cv2.cvtColor(img, cv2.COLOR_BGR2BGRA)
                x, y, w, h = get_detect(gray)
                if len(x) <= max_people:
                    num_people = len(x)
                else:
                    num_people = max_people

                sendData(254)  # start bytes
                for _ in range(num_people):
                    sendData(255 - int((x[_]+w[_]/2)/img.shape[1] * 255))       # send middle-x packages
                    sendData(int((y[_]+h[_]/2)/img.shape[0] * 255))             # send middle-y packages
                    sendData(int(w[_]/img.shape[1] * 255))                      # send width packages
                    cv2.rectangle(img, (x[_], y[_]), (x[_] + w[_], y[_] + h[_]), (0, 255, 0), 2)
                    cv2.putText(img, "Face #{}".format(_ + 1), (x[_], y[_] - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.7,
                                (0, 255, 0), 2)
                if len(x) <= max_people:
                    tmp = max_people - len(x)
                    for _ in range(tmp):
                        sendData(0)
                        sendData(0)
                        sendData(0)
                sendData(255)   # stop bytes
                for _ in range(num_people):
                    gif = cv2.resize(gifs[gif_counter], (w[_], h[_]), interpolation=cv2.INTER_LINEAR)
                    cv2.ellipse(img, (x[_]+int(w[_]/2), y[_]+int(h[_]/2)), (int(w[_]*0.447), int(h[_]*0.475)), 0, 0, 360, (0, 0, 0, 0), -1)
                    img[y[_]:y[_] + h[_], x[_]:x[_] + w[_], :] += gif[:, :, :]
                gif_counter += 1
                if gif_counter == 6:
                    gif_counter = 0
                cv2.imshow('detect', img)
                cv2.waitKey(1)
                tmp_read = sp.readline()
                if tmp_read != b'':
                    read = tmp_read
                print(read)
                if read == b'1\r\n':       # img save
                    cv2.imwrite(output_path + 'img_%d.jpg' % img_counter, cv2.cvtColor(img, cv2.COLOR_BGRA2BGR))
                    read = b'\r\n'
                    print('image captured')
                    img_counter += 1
                elif read == b'2\r\n':      # video save-start
                    print('video recording')
                    out.write(cv2.cvtColor(img, cv2.COLOR_BGRA2BGR))
                elif read == b'0\r\n':      # video save-end
                    read = b'\r\n'
                    out.release()
                    print('video released')
                    video_counter += 1
                    del out
                    out = cv2.VideoWriter(output_path + 'output_%d.avi' % video_counter, fourcc, 5.0, (640, 480))
                del x, y, w, h
            except:
                print("no face detected!!!")
