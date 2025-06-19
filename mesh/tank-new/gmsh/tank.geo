//
// ТОПЛИВНЫЙ БАК РК "ЦИКЛОН-4"
//

// Параметры
a = 1000;                       // Масштабный коэффициент
w = 2.8*1/a;     		// Толщина стенки
h = 18000/a;			// Высота конструкции
r2 = 1950/a;			// Внешний радиус
r1 = r2 - w;   			// Внутренний радиус
r3 = 2500/a;			// Радиус днищ
l = 420/a;			// Высота шпангоута
d = 16691/a;			// Расстояние между верхним и нижним шпангоутами
l1 = h - d - l;                 // Высота правого шпангоута
z0 = -Sqrt(r3^2 - r2^2) ;	// Расстояние центра верхнего днища от начала обечайки
z1 = -d + Sqrt(r3^2 - r2^2);	// Расстояние центра нижнего днища от начала обечайки
tt = 2.8*1/a;                    // Толщина переборок на торцах
lt = 550/a;                    // Расстояние от края до переборки
lp = 1502/a;                    // Длина пролета
ss = 62/a;                   // Ширина шпангоута
vs =  30/a;                   // Высота шпангоута
ssk = 100/a;                   // Ширина конусного шпангоута
vsb =  50/a;                   // Высота конусного шпангоута большая
vss =  30/a;                   // Высота конусного шпангоута меньшая
rp = 250/a;                    // Размер полости

rv =  350.0/a;                  // Радиус отверстия в днище

grid = 15*w;	// Параметр густоты сетки
//grid = 5*w;	// Параметр густоты сетки
z_min = 0;		        
z_max = h;

// Опорные точки
Point(1) = {0,  0, z1, grid};	// Центры сфер, образующих днища
Point(2) = {0,  0, z0, grid};

Point(3) = {-rv,  0, z0 + Sqrt(r3^2 - rv^2) - w, grid};
Point(4) = {-rv,  0, z0 + Sqrt(r3^2 - rv^2), grid};

Point(5) = {-r1,  0, z_min + w, grid};
Point(6) = {-r1,  0, z_min + l1, grid};
Point(7) = {-r2,  0, z_min + l1, grid};
Point(8) = {-r2,  0, -d - l, grid};
Point(9) = {-r1,  0, -d - l, grid};
Point(10) = {-r1,  0, z1 -Sqrt(r3^2 - r1^2), grid};
Point(11) = {0,  0, z1 - r3, grid};
Point(12) = {0,  0, z1 - r3 + w, grid};
Point(13) = {-r1,  0, z1 -Sqrt ((r3 - w)^2 - r1^2) , grid};
Point(14) = {-r1,  0, z_min - w, grid};

// Шпангоуты

Point(15) = {-r2,  0, z_min + ss , grid};
Point(16) = {-r2 - vs,  0, z_min + ss, grid};
Point(17) = {-r2 - vs,  0, z_min - ss , grid};
Point(18) = {-r2,  0, z_min - ss, grid};

Point(19) = {-r2,  0, z_min - lp + ss , grid};
Point(20) = {-r2 - vs,  0, z_min - lp + ss, grid};
Point(21) = {-r2 - vs,  0, z_min - lp - ss , grid};
Point(22) = {-r2,  0, z_min - lp - ss, grid};

Point(23) = {-r2,  0, z_min - 2*lp + ss , grid};
Point(24) = {-r2 - vs,  0, z_min - 2*lp + ss, grid};
Point(25) = {-r2 - vs,  0, z_min - 2*lp - ss , grid};
Point(26) = {-r2,  0, z_min - 2*lp - ss, grid};

Point(27) = {-r2,  0, z_min - 3*lp + ss , grid};
Point(28) = {-r2 - vs,  0, z_min - 3*lp + ss, grid};
Point(29) = {-r2 - vs,  0, z_min - 3*lp - ss , grid};
Point(30) = {-r2,  0, z_min - 3*lp - ss, grid};

Point(31) = {-r2,  0, z_min - 4*lp + ss , grid};
Point(32) = {-r2 - vs,  0, z_min - 4*lp + ss, grid};
Point(33) = {-r2 - vs,  0, z_min - 4*lp - ss , grid};
Point(34) = {-r2,  0, z_min - 4*lp - ss, grid};

Point(35) = {-r2,  0, z_min - 5*lp + ss , grid};
Point(36) = {-r2 - vs,  0, z_min - 5*lp + ss, grid};
Point(37) = {-r2 - vs,  0, z_min - 5*lp - ss , grid};
Point(38) = {-r2,  0, z_min - 5*lp - ss, grid};

Point(39) = {-r2,  0, z_min - 6*lp + ss , grid};
Point(40) = {-r2 - vs,  0, z_min - 6*lp + ss, grid};
Point(41) = {-r2 - vs,  0, z_min - 6*lp - ss , grid};
Point(42) = {-r2,  0, z_min - 6*lp - ss, grid};

Point(43) = {-r2,  0, z_min - 7*lp + ssk , grid};
Point(44) = {-r2 - vsb,  0, z_min - 7*lp + ssk, grid};
Point(45) = {-r2 - vss,  0, z_min - 7*lp - ssk, grid};
Point(46) = {-r2,  0, z_min - 7*lp - ssk, grid};

Point(47) = {-r2,  0, z_min - 8*lp + ss , grid};
Point(48) = {-r2 - vs,  0, z_min - 8*lp + ss, grid};
Point(49) = {-r2 - vs,  0, z_min - 8*lp - ss , grid};
Point(50) = {-r2,  0, z_min - 8*lp - ss, grid};

Point(51) = {-r2,  0, z_min - 9*lp + ss , grid};
Point(52) = {-r2 - vs,  0, z_min - 9*lp + ss, grid};
Point(53) = {-r2 - vs,  0, z_min - 9*lp - ss , grid};
Point(54) = {-r2,  0, z_min - 9*lp - ss, grid};

Point(55) = {-r2,  0, z_min - 10*lp + ss , grid};
Point(56) = {-r2 - vs,  0, z_min - 10*lp + ss, grid};
Point(57) = {-r2 - vs,  0, z_min - 10*lp - ss , grid};
Point(58) = {-r2,  0, z_min - 10*lp - ss, grid};

Point(59) = {-r2,  0, z_min - 11*lp + ss , grid};
Point(60) = {-r2 - vs,  0, z_min - 11*lp + ss, grid};
Point(61) = {-r2 - vs,  0, z_min - 11*lp - ss , grid};
Point(62) = {-r2,  0, z_min - 11*lp - ss, grid};

Point(63) = {-Sqrt(r3^2-(z_min + l1 - lt - z0)^2),  0, z_min + l1 - lt, grid};
Point(64) = {-r1,  0, z_min + l1 - lt, grid};
Point(66) = {-r1,  0, z_min + l1 - lt - tt, grid};
Point(65) = {-Sqrt(r3^2-(z_min + l1 - lt - tt - z0)^2),  0, z_min + l1 - lt - tt, grid};

Point(67) = {-r1,  0, z1 -Sqrt(r3^2 - r1^2) - rp - tt, grid};
Point(68) = {-Sqrt(r3^2-(-Sqrt(r3^2 - r1^2) - rp - tt)^2),  0, z1 -Sqrt(r3^2 - r1^2) - rp - tt, grid};
Point(69) = {-r1,  0, z1 -Sqrt(r3^2 - r1^2) - rp, grid};
Point(70) = {-Sqrt(r3^2-(-Sqrt(r3^2 - r1^2) - rp )^2),  0, z1 -Sqrt(r3^2 - r1^2) - rp, grid};


// Линии
Line(1) = {3, 4};
//Line(2) = {5, 6};
Line(3) = {6, 7};
Line(4) = {7, 19};
Line(5) = {8, 9};
Line(6) = {9, 67};
Line(7) = {11, 12};
Line(8) = {13, 14};
//Circle(9) = { 10, 1, 11 };
Circle(10) = { 12, 1, 13 };
Circle(11) = { 14, 2, 3 };
Circle(12) = { 4, 2, 63 };
//Line(13) = {15, 16};
//Line(14) = {16, 17};
//Line(15) = {17, 18};
//Line(16) = {18, 19};

Line(17) = {19, 20};
Line(18) = {20, 21};
Line(19) = {21, 22};
Line(20) = {22, 23};

Line(21) = {23, 24};
Line(22) = {24, 25};
Line(23) = {25, 26};
Line(24) = {26, 27};

Line(25) = {27, 28};
Line(26) = {28, 29};
Line(27) = {29, 30};
Line(28) = {30, 31};

Line(29) = {31, 32};
Line(30) = {32, 33};
Line(31) = {33, 34};
Line(32) = {34, 35};

Line(33) = {35, 36};
Line(34) = {36, 37};
Line(35) = {37, 38};
Line(36) = {38, 39};

Line(37) = {39, 40};
Line(38) = {40, 41};
Line(39) = {41, 42};
Line(40) = {42, 43};

Line(41) = {43, 44};
Line(42) = {44, 45};
Line(43) = {45, 46};
Line(44) = {46, 47};

Line(45) = {47, 48};
Line(46) = {48, 49};
Line(47) = {49, 50};
Line(48) = {50, 51};

Line(49) = {51, 52};
Line(50) = {52, 53};
Line(51) = {53, 54};
Line(52) = {54, 55};

Line(53) = {55, 56};
Line(54) = {56, 57};
Line(55) = {57, 58};
Line(56) = {58, 59};

Line(57) = {59, 60};
Line(58) = {60, 61};
Line(59) = {61, 62};
Line(60) = {62, 8};

Line(62) = {63, 64};
Line(63) = {64, 6};

Line(64) = {5, 66};
Line(65) = {66, 65};
Circle(66) = { 65, 2, 5 };

Line(67) = {67, 68};
Circle(68) = { 68, 1, 11 };
Line(69) = {10, 69};
Line(70) = {69, 70};
Circle(71) = { 70, 1, 10 };
// Контуры
Line Loop(1) = {1, 12, 62, 63, 3, 4, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 5, 6, 67, 68, 7, 10, 8, 11};
Line Loop(2) = {64, 65, 66};
Line Loop(3) = {69, 70, 71};
// Поверхности
Plane Surface(1) = {1, 2, 3};
//Plane Surface(2) = {2};

//s1[] = Rotate {{0,   0, z_max}, {0,   0, z_max}, Pi/20} {
//  Duplicata { Surface{1}; }
//};


// Замкнутые поверхности
Surface Loop(1) = {1}; 
//Surface Loop(2) = {2}; 


Extrude{{0, 0, z_max}, {0, 0, z_max}, Pi/2} { Surface {1, 2, 3}; }

// Объем
//Volume(1) = {1};

