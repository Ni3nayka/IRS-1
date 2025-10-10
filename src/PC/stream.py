import cv2
from videoHttpStreamer import VideoHttpStreamer

def main():
    # Инициализация видеозахвата
    cap = cv2.VideoCapture(2)
    
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
            
                
    finally:
        # Освобождение ресурсов
        cap.release()
        cv2.destroyAllWindows()
        streamer.stop()

if __name__ == '__main__':
    main()
