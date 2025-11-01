import math

class Vector:
    def __init__(self, x, y, dir=0, name=None):
        self.x = x
        self.y = y
        self.dir = dir  # направление (угол в градусах или радианах)
        self.name = name
    
    def __add__(self, other):
        """Сложение векторов"""
        return Vector(self.x + other.x, self.y + other.y, self.dir)
    
    def __sub__(self, other):
        """Вычитание векторов"""
        return Vector(self.x - other.x, self.y - other.y, self.dir)
    
    def __mul__(self, scalar):
        """Умножение на скаляр"""
        return Vector(self.x * scalar, self.y * scalar, self.dir)
    
    def dot(self, other):
        """Скалярное произведение"""
        return self.x * other.x + self.y * other.y
    
    def cross(self, other):
        """Векторное произведение (для 2D возвращает скаляр)"""
        return self.x * other.y - self.y * other.x
    
    def magnitude(self):
        """Длина вектора"""
        return math.sqrt(self.x**2 + self.y**2)
    
    def normalize(self):
        """Нормализация вектора"""
        mag = self.magnitude()
        if mag == 0:
            return Vector(0, 0, self.dir)
        return Vector(self.x / mag, self.y / mag, self.dir)
    
    def angle_between(self, other):
        """Угол между векторами в радианах"""
        dot_product = self.dot(other)
        mag_product = self.magnitude() * other.magnitude()
        if mag_product == 0:
            return 0
        return math.acos(max(-1, min(1, dot_product / mag_product)))
    
    def copy(self, name=None):
        """Создание копии вектора"""
        return Vector(self.x, self.y, self.dir, name)
    
    def print(self):
        """Вывод информации о векторе"""
        print("coordinates", end='')
        if self.name != None: 
            print(" (" + str(self.name) + ")", end='')
        print(": x=" + str(self.x) + " y=" + str(self.y) + " dir=" + str(self.dir))
    
    def rotate(self, angle_degrees):
        """Поворот вектора на угол в градусах"""
        angle_rad = math.radians(angle_degrees)
        cos_angle = math.cos(angle_rad)
        sin_angle = math.sin(angle_rad)
        new_x = self.x * cos_angle - self.y * sin_angle
        new_y = self.x * sin_angle + self.y * cos_angle
        return Vector(new_x, new_y, self.dir + angle_degrees)
    
    def distance_to(self, other):
        """Расстояние до другого вектора (точки)"""
        return math.sqrt((self.x - other.x)**2 + (self.y - other.y)**2)
    
    def __repr__(self):
        name_str = f", name='{self.name}'" if self.name else ""
        return f"Vector(x={self.x}, y={self.y}, dir={self.dir}{name_str})"

# Пример использования
v1 = Vector(1, 2, 0, "Вектор A")
v2 = Vector(4, 5, 90, "Вектор B")

print("Исходные векторы:")
v1.print()
v2.print()

print(f"\nСложение: {v1 + v2}")
print(f"Скалярное произведение: {v1.dot(v2)}")
print(f"Угол между векторами: {math.degrees(v1.angle_between(v2)):.2f}°")

# Создание копии
v3 = v1.copy("Копия вектора A")
print("\nКопия вектора:")
v3.print()

# Поворот вектора
v_rotated = v1.rotate(90)
print(f"\nПовернутый вектор: {v_rotated}")

# Расстояние между точками
print(f"Расстояние между точками: {v1.distance_to(v2):.2f}")