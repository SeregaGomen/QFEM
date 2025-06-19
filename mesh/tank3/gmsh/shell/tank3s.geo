//
// ТОПЛИВНЫЙ БАК РК "ЦИКЛОН-4" (оболочечная модель)
//

// ПАРАМЕТРЫ
// Радиус днищ "сферической" части бака
R = 1.037; 
// Высота цилиндрической части "сферического" бака
L = 0.269; 
// Параметр размера сетки
//grid = 0.05;


//grid = 0.02;
grid = 0.01;


// ОПОРНЫЕ ТОЧКИ
Point(1) = { 0, 0, 0, grid };
Point(2) = { 0, R, 0, grid };
Point(3) = { -R, 0, 0, grid };
Point(4) = { -R, -L, 0, grid };
Point(5) = { 0,	-R - L, 0, grid };
Point(6) = { 0, -L, 0, grid };

Point(7) = { -1.641, -0.643, 0, grid/2 };
Point(8) = { -0.660, -1.944, 0, grid };
Point(9) = { -0.562, -1.944, 0, grid };
Point(10) = { -0.342, -1.724, 0, grid };
Point(11) = { 0, -1.724, 0, grid };

Point(12) = { -1.636, -0.638, 0, grid/2 };
Point(13) = { -1.636, -0.650, 0, grid/2 };

Point(14) = { -1.641, -0.638, 0, grid/2 };
Point(15) = { -1.641, -0.650, 0, grid/2 };


// ОПОРНЫЕ ЛИНИИ И ПОВЕРХНОСТИ
Circle(1) = { 2, 1, 3 };
Circle(2) = { 4, 6, 5 };
Line(3) = { 3, 4 };

Line(4) = { 3, 12 };
Line(5) = { 12, 7 };


Line(6) = { 7, 13 };
Line(7) = { 13, 8 };

Line(8) = { 8, 9 };
Line(9) = { 9, 10 };
Line(10) = { 10, 11 };

Line(11) = { 12, 13 };



// ПОВЕРХНОСТИ
Extrude{ { 0, 1, 0 }, { 0, 0, 0 }, Pi/2 } { Line{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }; }
//Extrude{ { 0, 1, 0 }, { 0, 0, 0 }, -Pi/2 } { Line{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }; }
//+
//Symmetry {1, 0, 0, 0} {
// Duplicata { Surface{14}; Surface{55}; Surface{21}; Surface{25}; Surface{17}; Surface{37}; Surface{66}; Surface{58}; Surface{62}; Surface{78}; Surface{82}; Surface{86}; Surface{89}; Surface{48}; Surface{45}; Surface{41}; Surface{74}; Surface{93}; Surface{70}; Surface{52}; Surface{33}; Surface{29}; }
//}
