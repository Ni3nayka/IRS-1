"""
code write for project:
https://github.com/Ni3nayka/IRS-1/

author: Egor Bakay <egor_bakay@inbox.ru> Ni3nayka
write:  July 2025
modify: July 2025

code write with deepseek
"""

"""
# EXAMPLE

import cv2
from videoHttpStreamer import VideoHttpStreamer

def main():
    # Инициализация видеозахвата
    cap = cv2.VideoCapture(0)
    
    # Инициализация HTTP-стримера (чтобы на другой машине в локальной сети можно было видео смотреть)
    streamer = VideoHttpStreamer()
    streamer.start()
    
    try:
        while True:
            # Чтение кадра с камеры
            success, frame = cap.read()
            if not success:
                break
                
            # Отправка кадра в стример
            streamer.update_frame(frame)
            
            # Для отладки: показ кадра в локальном окне
            cv2.imshow('Camera', frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
                
    finally:
        # Освобождение ресурсов
        cap.release()
        cv2.destroyAllWindows()
        streamer.stop()

if __name__ == '__main__':
    main()
"""


from flask import Flask, Response
import threading
import time
import cv2

class VideoHttpStreamer:
    def __init__(self, host='0.0.0.0', port=5000):
        self.host = host
        self.port = port
        self.frame = None
        self.lock = threading.Lock()
        self.app = Flask(__name__)
        self.server_thread = None
        
        # Настройка маршрутов Flask
        self.app.add_url_rule('/', 'index', self.index)
        self.app.add_url_rule('/video_feed', 'video_feed', self.video_feed)
    
    def update_frame(self, frame):
        """Обновление текущего кадра для трансляции"""
        with self.lock:
            _, buffer = cv2.imencode('.jpg', frame)
            self.frame = buffer.tobytes()
    
    def generate_frames(self):
        """Генератор кадров для потоковой передачи"""
        while True:
            with self.lock:
                if self.frame is not None:
                    yield (b'--frame\r\n'
                          b'Content-Type: image/jpeg\r\n\r\n' + 
                          self.frame + b'\r\n')
            time.sleep(0.033)  # ~30 FPS
    
    def video_feed(self):
        """Маршрут для видео потока"""
        return Response(self.generate_frames(),
                       mimetype='multipart/x-mixed-replace; boundary=frame')
    
    def index(self):
        """Главная страница с видео"""
        return """
        <html>
          <head>
            <title>Video Streaming</title>
          </head>
          <body>
            <h1>Video Streaming</h1>
            <img src="/video_feed" width="640" height="480">
          </body>
        </html>
        """
    
    def start(self):
        """Запуск HTTP-сервера в отдельном потоке"""
        self.server_thread = threading.Thread(
            target=self.app.run,
            kwargs={'host': self.host, 'port': self.port, 'debug': False, 'threaded': True}
        )
        self.server_thread.daemon = True
        self.server_thread.start()
        print(f"HTTP video streamer started at http://{self.host}:{self.port}")
    
    def stop(self):
        """Остановка HTTP-сервера"""
        # Flask не поддерживает штатную остановку, поэтому просто завершаем поток
        if self.server_thread:
            self.server_thread.join(timeout=1)
