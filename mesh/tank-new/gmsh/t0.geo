// номера физических объектов
LEFT_BOUNDARY = 3001;
RIGHT_BOUNDARY = 3002;
FRONT_BOUNDARY = 3003;
BACK_BOUNDARY = 3004;
BOTTOM_BOUNDARY = 3005;
TOP_BOUNDARY = 3006;
CUBE = 4001;
SPHERE = 4002;

// опции
Mesh.Algorithm = 6; // двумерный алгоритм - Фронтальный
Mesh.Algorithm3D = 4; // трехмерный алгоритм - Фронтальный
Mesh.OptimizeNetgen = 1; // включить Netgen-оптимизацию сетки
Mesh.Hexahedra  = 2; 
Geometry.ExtrudeReturnLateralEntities = 0;

// параметры
clCube = 0.4; // характеристические длины
clSphere = 0.2;
rad = 0.25; // радиус сферы
X_BEG = 0; X_END = 1; // координаты границ параллелепипеда
Y_BEG = 0; Y_END = 1;
Z_BEG = 0; Z_END = 1;
lenX = X_END - X_BEG; // длины сторон параллелепипеда
lenY = Y_END - Y_BEG;
lenZ = Z_END - Z_BEG;
x_cen = (X_BEG + X_END) / 2.0; // координаты центра сферы
y_cen = (Y_BEG + Y_END) / 2.0;
z_cen = (Z_BEG + Z_END) / 2.0;
// наименьшая сторона параллелепипеда
min = (lenX < lenY ? lenX : lenY);
minSide = (lenZ < min ? lenZ : min);

// ---------- ТОЧКИ -----------

// прямоугольник
Point(1) = { X_BEG, Y_BEG, Z_BEG, clCube * minSide };
Point(2) = { X_END, Y_BEG, Z_BEG, clCube * minSide };
Point(3) = { X_BEG, Y_BEG, Z_END, clCube * minSide };
Point(4) = { X_END, Y_BEG, Z_END, clCube * minSide };
// полуокружность
Point(11) = { x_cen, y_cen, z_cen, clSphere * rad };
Point(12) = { x_cen - rad, y_cen, z_cen, clSphere * rad };
Point(13) = { x_cen, y_cen, z_cen - rad, clSphere * rad };
Point(14) = { x_cen + rad, y_cen, z_cen, clSphere * rad };

// ---------- ЛИНИИ -----------

// прямоугольник
Line(1) = { 1, 2 };
Line(2) = { 1, 3 };
Line(3) = { 2, 4 };
Line(4) = { 3, 4 };
// полуокружность
Circle(21) = { 12, 11, 13 };
Circle(22) = { 13, 11, 14 };

// ---------- КОНТУРЫ ЛИНИЙ -----------

// прямоугольник
Line Loop(41) = { 1, 3, -4, -2 };

// ---------- ПОВЕРХНОСТИ -----------

// прямоугольник
Plane Surface(1) = { 41 };

// ---------- ФУНКЦИИ -----------

// поверхности параллелепипеда
s1[] = Extrude { 0, Y_END, 0 } { Line{1, 2, 3, 4}; };
s2[] = Translate { 0, Y_END, 0 } { Duplicata{ Surface{1}; } };

Printf("%g %g %g %g %g %g %g %g", s1[0], s1[1], s1[2], s1[3], s1[4], s1[5], s1[6], s1[7]);
Printf("%g %g %g %g", s1[1], s1[3], s1[5], s1[7]);



// поверхности сфера
sph1[] = Extrude { { 1, 0, 0 }, { x_cen, y_cen, z_cen }, 2 * Pi / 3 } { Line{21, 22}; };
sph2[] = Extrude { { 1, 0, 0 }, { x_cen, y_cen, z_cen }, 2 * Pi / 3 } { Line{sph1[0], sph1[2]}; };
sph3[] = Extrude { { 1, 0, 0 }, { x_cen, y_cen, z_cen }, 2 * Pi / 3 } { Line{sph2[0], sph2[2]}; };

// ---------- КОНТУРЫ ПОВЕРХНОСТЕЙ -----------

// параллелепипед
Surface Loop(21) = { 1, s1[1], s1[3], s1[5], s1[7], s2[0] };
// сфера
Surface Loop(22) = { sph1[1], sph1[3], sph2[1], sph2[3], sph3[1], sph3[3] };

// ---------- ОБЪЕМЫ -----------

// параллелепипед без сферы
Volume(1) = { 21, 22 };
// сфера
Volume(2) = { 22 };

// ---------- ФИЗИЧЕСКИЕ СУЩНОСТИ -----------

Physical Surface(LEFT_BOUNDARY) = { s1[3] };
Physical Surface(RIGHT_BOUNDARY) = { s1[5] };
Physical Surface(FRONT_BOUNDARY) = { 1 };
Physical Surface(BACK_BOUNDARY) = { s2[0] };
Physical Surface(BOTTOM_BOUNDARY) = { s1[1] };
Physical Surface(TOP_BOUNDARY) = { s1[7] };
Physical Volume(CUBE) = { 1 };
Physical Volume(SPHERE) = { 2 };
