class RoadSign:
    def __init__(self, type=None, x=None, y=None, z=None):
        self.type = type
        # относительные координаты дорожного знака (в кадре)
        self.x = x
        self.y = y
        self.z = z
        # абсолютные координаты дорожного знака 
        # (реализуем "потом")
