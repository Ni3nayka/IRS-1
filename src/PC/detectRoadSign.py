class RoadSignType:
    def __init__(self):
        self.FORWARD = 1
        self.LEFT = 2
        self.RIGHR = 3
        self.NOT_LEFT = 4
        self.NOT_RIGHR = 5
        self.BUS = 6
        self.PARKING = 7
        self.WARNING = 8
        
class RoadSign:
    def __init__(self, type=None, x=None, y=None, z=None):
        self.type = type # RoadSignType
        # относительные координаты дорожного знака (в кадре)
        self.x = x
        self.y = y
        self.z = z
        # абсолютные координаты дорожного знака 
        # (реализуем "потом")

if __name__ == "__main__":
    print(RoadSignType().FORWARD==RoadSignType().FORWARD)
    print(RoadSignType().FORWARD==RoadSignType().LEFT)
