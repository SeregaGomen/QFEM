grid = 0.1;	// Параметр густоты сетки
h = 5;
z_min = 0;		        
z_max = h;

// Опорные точки
Point(1) = {0,  0, z_min, grid};
Point(2) = {1,  0, z_min, grid};
Point(3) = {0,  1, z_min, grid};





// Линии
Circle(1) = { 2, 1, 3 };

// Поверхности
Extrude { 0, 0, z_max } { Line{1}; }

//Extrude{{0, 0, z_min}, {0, 0, z_max}, 0} { Line {1}; }







