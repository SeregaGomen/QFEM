#include <iostream>
#include <cmath>
#include "object/object.h"

TMessenger* msg;

// Тестовый расчет бака
void calcTank(void)
{
    TFEMObject object;
    // Свойства материала АМг6М
    matrix<double> ssc = {
                            { 1.30E+8, 0.0020 },
                            { 1.40E+8, 0.0024 },
                            { 1.50E+8, 0.0030 },
                            { 1.60E+8, 0.0040 },
                            { 1.70E+8, 0.0055 },
                            { 1.80E+8, 0.0079 },
                            { 1.90E+8, 0.0130 },
                            { 2.00E+8, 0.0150 },
                            { 2.10E+8, 0.0248 },
                            { 2.20E+8, 0.0320 },
                            { 2.30E+8, 0.0361 },
                            { 2.70E+8, 0.0500 },
                            { 3.09E+8, 0.1000 },
                            { 3.30E+8, 0.1500 }
                          };

    if (!object.setMeshFile("../../QFEM/mesh/tank-0_25.trp"))
        return;
    object.setTaskParam(FEMType::StaticProblem);

    object.setEps(1.0E-8);
    object.setWidth(15);
    object.setPrecision(5);

    // Упругие характеристики
    //        object.addYoungModulus("6.5e+10");
    //        object.addPoissonRatio("0.3");
    object.addYoungModulus([](double, double, double, double){ return 6.5e+10; });
    object.addPoissonRatio([](double, double, double, double){ return 0.3; });
    // Диаграмма деформирования
    object.addStressStrainCurve(ssc);
    // Шаг по нагрузке
    object.setLoadStep(10);
    // Способ расчета пластичности
    object.setPlasticityMethod(PlasticityMethod::MVS);

    // Граничные условия
    //        object.addBoundaryCondition(DIR_X | DIR_Y | DIR_Z, "0", "z == -4.7");
    //        object.addBoundaryCondition(DIR_X, "0", "x == 0");
    //        object.addBoundaryCondition(DIR_Y, "0", "y == 0");

    object.addBoundaryCondition(Direction::X | Direction::Y | Direction::Z, [](double, double, double, double){ return 0.0; }, [](double, double, double z, double){ return (abs(z + 4.7) <= 1.0E-10) ? 1.0 : 0.0; });
    object.addBoundaryCondition(Direction::X | Direction::Y | Direction::Z, [](double, double, double, double){ return 0.0; }, [](double x, double, double, double){ return (x == 0.0) ? 1.0 : 0.0; });
    object.addBoundaryCondition(Direction::X | Direction::Y | Direction::Z, [](double, double, double, double){ return 0.0; }, [](double, double y, double, double){ return (y == 0.0) ? 1.0 : 0.0; });



    //        // Распределенная поверхностная нагрузка
    //        // ...по обичайке
    //        object.addSurfaceLoad(DIR_X, "8.2 * 10 ** 4 * cos(atan2(y, x))", "(z >= -1.406 AND z <= 1.405) AND (abs(x ** 2 + y ** 2 - 1.4973 ** 2) < eps)");
    //        object.addSurfaceLoad(DIR_Y, "8.2 * 10 ** 4 * sin(atan2(y, x))", "(z >= -1.406 AND z <= 1.405) AND (abs(x ** 2 + y ** 2 - 1.4973 ** 2) < eps)");

    //        // по верхнему и нижнему днищу
    //        object.addSurfaceLoad("8.2 * 10 ** 4 * cos(atan2(y, x))*sin(atan2((x ** 2+y ** 2) ** 0.5, (z - 0.08715)))", "(z > 1.406) AND (abs(x ** 2 + y ** 2 + (z - 0.08715) ** 2 - 2 ** 2) < eps)", DIR_X);
    //        object.addSurfaceLoad("8.2 * 10 ** 4 * sin(atan2(y, x))*sin(atan2((x ** 2+y ** 2) ** 0.5, (z - 0.08715)))", "(z > 1.406) AND (abs(x ** 2 + y ** 2 + (z - 0.08715) ** 2 - 2 ** 2) < eps)", DIR_Y);
    //        object.addSurfaceLoad("8.2 * 10 ** 4 * cos(atan2((x ** 2 + y ** 2) ** 0.5, (z - 0.08715)))", "(z > 1.406) AND (abs(x ** 2 + y ** 2 + (z - 0.08715) ** 2 - 2 ** 2) < eps)", DIR_Z);

    //        object.addSurfaceLoad("8.2 * 10 ** 4 * cos(atan2(y,x))*sin(atan2((x ** 2 + y ** 2) ** 0.5, (z + 0.08715)))", "(z <- 1.406 AND z >-2.785 ) AND (abs(x ** 2 + y ** 2 + (z + 0.08715) ** 2 - 2 ** 2) < eps)", DIR_X);
    //        object.addSurfaceLoad("8.2 * 10 ** 4 * sin(atan2(y,x))*sin(atan2((x ** 2 + y ** 2) ** 0.5, (z + 0.08715)))", "(z <- 1.406 AND z >-2.785 ) AND (abs(x ** 2 + y ** 2 + (z + 0.08715) ** 2 - 2 ** 2) < eps)", DIR_Y);
    //        object.addSurfaceLoad("8.2 * 10 ** 4 * cos(atan2((x ** 2 + y ** 2) ** 0.5, (z + 0.08715)))", "(z < -1.406 AND z > -2.785 ) AND (abs(x ** 2 + y ** 2 + (z + 0.08715) ** 2 - 2 ** 2) < eps)", DIR_Z);

    // Внутреннее давление
    // ...по обичайке
    //        object.addPressureLoad("8.2E+4", "(z >= -1.406 AND z <= 1.405) AND (abs(x ** 2 + y ** 2 - 1.4973 ** 2) < eps)");
    object.addPressureLoad([](double, double, double, double){ return 8.2E+4; }, [](double x, double y, double z, double){ return ((z >= -1.406 && z <= 1.405) && (abs(x * x + y * y - 1.4973 * 1.4973) < 1.0E-10)) ? 1.0 : 0.0; });

    // по верхнему и нижнему днищу
    //        object.addPressureLoad("8.2E+4", "(z > 1.406) AND (abs(x ** 2 + y ** 2 + (z - 0.08715) ** 2 - 2 ** 2) < eps)");
    //        object.addPressureLoad("8.2E+4", "(z < -1.406 AND z > -2.785 ) AND (abs(x ** 2 + y ** 2 + (z + 0.08715) ** 2 - 2 ** 2) < eps)");

    object.addPressureLoad([](double, double, double, double){ return 8.2E+4; }, [](double x, double y, double z, double){ return ((z > 1.406) && (abs(x * x + y * y + (z - 0.08715) * (z - 0.08715) - 4) < 1.0E-10)) ? 1.0 : 0.0; });
    object.addPressureLoad([](double, double, double, double){ return 8.2E+4; }, [](double x, double y, double z, double){ return ((z < -1.406 && z > -2.785 ) && (abs(x * x + y * y + (z + 0.08715) * (z + 0.08715) - 4) < 1.0E-10)) ? 1.0 : 0.0; });


    if (object.start())
    {
        object.saveResult(object.stdResName());
        object.printResult(object.stdTxtResName());
    }
}

// Тестовый расчет балки
void calcBalka(void)
{
    // Свойства материала АМг6М
    TFEMObject object;

    if (!object.setMeshFile("../../QFEM/mesh/balka.trp"))
        return;
    object.setTaskParam(FEMType::StaticProblem);
    object.setNumThread(8);

    object.setEps(1.0E-8);
    object.setWidth(15);
    object.setPrecision(5);

    // Упругие характеристики
    object.addYoungModulus("6.5e+10");
    object.addPoissonRatio("0.3");
    // Распределенная поверхностная нагрузка
    object.addSurfaceLoad(Direction::Y, "-6 * 10 ** 7", "y == 4");
    // Граничные условия
    object.addBoundaryCondition(Direction::X | Direction::Y | Direction::Z, "0", "y == 0");

    // Запуск расчета
    if (object.start())
    {
        object.saveResult(object.stdResName());
        object.printResult(object.stdTxtResName());
    }
}


// Сравнение с pyfem
void pyfem_test(void)
{
    // Свойства материала АМг6М
    TFEMObject object;
    double z_min = 0.0,
           z_max = 4.014;

    if (!object.setMeshFile("../../../../python/pyfem/mesh/shell-tube6.trpa"))
        return;
    object.setTaskParam(FEMType::StaticProblem);

    // Упругие характеристики
    object.addYoungModulus(203200);
    object.addPoissonRatio(0.27);
    // Толщина КЭ
    object.addThickness(0.0369);
    // Распределенная поверхностная нагрузка
    object.addPressureLoad(0.05);
    // Граничные условия
    object.addBoundaryCondition(Direction::X | Direction::Y | Direction::Z, 0.0, [&](double, double, double z, double){ return (z == z_min || z == z_max) ? 1.0 : 0.0; });

    // Запуск расчета
    if (object.start())
    {
        object.saveResult(object.stdResName());
        object.printResult(object.stdTxtResName());
    }

}

void calcTank3ds(void)
{
    TFEMObject object;

    if (!object.setMeshFile("d:/work/python/pyfem/mesh/tank3ds.trpa"))
        return;
    object.setTaskParam(FEMType::StaticProblem);
    // Упругие характеристики
    object.addYoungModulus([](double, double, double, double){ return 6.5E+10; }, [](double, double, double z, double){ return (z < -7) ? 1.0 : 0.0; });
    //        object.addPoissonRatio([](double, double, double, double){ return 0.3; }, [](double, double, double z, double){ return (z < -7) ? 1.0 : 0.0; });
    object.addYoungModulus([](double, double, double, double){ return 9.5E+10; }, [](double, double, double z, double){ return (z >= -7) ? 1.0 : 0.0; });
    object.addPoissonRatio([](double, double, double, double){ return 0.3; });
    // Толщина КЭ
    object.addThickness("0.0028");
    // Распределенная поверхностная нагрузка
    object.addSurfaceLoad(Direction::X, [](double x, double y, double, double){ return 1000 * cos(atan2(y, x)); }, [](double, double, double z, double){ return (z <= 0 && z >= -16.691) ? 1.0 : 0.0; });
    object.addSurfaceLoad(Direction::Y, [](double x, double y, double, double){ return 1000 * sin(atan2(y, x)); }, [](double, double, double z, double){ return (z <= 0 && z >= -16.691) ? 1.0 : 0.0; });

    object.addSurfaceLoad(Direction::X, [](double x, double y, double z, double){ return 1000 * cos(atan2(y, x)) * sin(atan2(pow(x * x + y * y, 0.5), (z + 1.565))); }, [](double x, double y, double z, double){ return (abs(x * x + y * y + (z + 1.565) * (z + 1.565) - 2.5 * 2.5) <= 1.0E-2) ? 1.0 : 0.0; });
    object.addSurfaceLoad(Direction::Y, [](double x, double y, double z, double){ return 1000 * sin(atan2(y, x)) * sin(atan2(pow(x * x + y * y, 0.5), (z + 1.565))); }, [](double x, double y, double z, double){ return (abs(x * x + y * y + (z + 1.565) * (z + 1.565) - 2.5 * 2.5) <= 1.0E-2) ? 1.0 : 0.0; });
    object.addSurfaceLoad(Direction::Z, [](double x, double y, double z, double){ return 1000 * cos(atan2(pow(x * x + y *y, 0.5), (z + 1.565))); }, [](double x, double y, double z, double){ return (abs(x * x + y * y + (z + 1.565) * (z + 1.565) - 2.5 * 2.5) <= 1.0E-2) ? 1.0 : 0.0; });

    object.addSurfaceLoad(Direction::X, [](double x, double y, double z, double){ return 1000 * cos(atan2(y, x)) * sin(atan2(pow(x * x + y * y, 0.5), (z + 15.126))); }, [](double x, double y, double z, double){ return (abs(x * x + y * y + (z + 15.126) * (z + 15.126) - 2.5 * 2.5) <= 1.0E-2) ? 1.0 : 0.0; });
    object.addSurfaceLoad(Direction::Y, [](double x, double y, double z, double){ return 1000 * sin(atan2(y, x)) * sin(atan2(pow(x * x + y * y, 0.5), (z + 15.126))); }, [](double x, double y, double z, double){ return (abs(x * x + y * y + (z + 15.126) * (z + 15.126) - 2.5 * 2.5) <= 1.0E-2) ? 1.0 : 0.0; });
    object.addSurfaceLoad(Direction::Z, [](double x, double y, double z, double){ return 1000 * cos(atan2(pow(x * x + y* y, 0.5), (z + 15.126))); }, [](double x, double y, double z, double){ return (abs(x * x + y * y + (z + 15.126) * (z + 15.126) - 2.5 * 2.5) <= 1.0E-2) ? 1.0 : 0.0; });
    // Граничные условия
    object.addBoundaryCondition(Direction::X | Direction::Y | Direction::Z, [](double, double, double, double){ return 0.0; }, [](double, double, double z, double){ return (abs(z + 17.626) <= 1.0E-2) ? 1.0 : 0.0; });
    object.addBoundaryCondition(Direction::X, [](double, double, double, double){ return 0.0; }, [](double x, double, double, double){ return (abs(x) <= 1.0E-2) ? 1.0 : 0.0; });
    object.addBoundaryCondition(Direction::Y, [](double, double, double, double){ return 0.0; }, [](double, double y, double, double){ return (abs(y) <= 1.0E-2) ? 1.0 : 0.0; });

    // Запуск расчета
    if (object.start())
    {
        object.saveResult(object.stdResName());
        object.printResult(object.stdTxtResName());
    }
}

void calcTank3s6(void)
{
    TFEMObject object;

    if (!object.setMeshFile("d:/work/python/pyfem/mesh/tank3s6.trpa"))
        return;
    object.setTaskParam(FEMType::StaticProblem);
    // Упругие характеристики
    object.addYoungModulus(6.5E+10);
    object.addPoissonRatio(0.3);
    // Толщина КЭ
    //        object.addThickness("0.0015");
    object.addThickness(0.0015);
    // Граничные условия
    object.addBoundaryCondition(Direction::X | Direction::Y | Direction::Z, 0.0, [](double x, double y, double z, double){ return (y == -0.643 && abs(x * x + z * z -1.641 * 1.641) <= 0.01) ? 1.0 : 0.0; });
    // Распределенная поверхностная нагрузка
    object.addPressureLoad(5000.0, [](double x, double y, double z, double){ return ((y <= 0 && y>=-0.269) && (abs(x * x + z * z - 1.037 * 1.037) <= 0.01)) ? 1.0 : 0.0; });
    object.addPressureLoad(5000.0, [](double x, double y, double z, double){ return ((y < -0.269) && (abs(x * x + z * z + (y + 0.269) * (y + 0.269) - 1.037 * 1.037)) <= 0.01) ? 1.0 : 0.0; });
    object.addPressureLoad(10000.0, [](double x, double y, double z, double){ return ((y > 0) && (abs(x * x + y * y + z * z - 1.037 * 1.037) <= 0.01)) ? 1.0 : 0.0; });
    object.addPressureLoad(5000.0, [](double x, double y, double z, double){ return ((y == -1.724) && (x * x + z * z - 0.342 * 0.342 <= 0.01)) ? 1.0 : 0.0; });
    object.addPressureLoad(5000.0, [](double x, double y, double z, double){ return ((y == -1.944) && (x * x + z * z - 0.660 * 0.660 <= 0.01)) ? 1.0 : 0.0; });
    object.addPressureLoad(5000.0, [](double x, double y, double z, double){ return ((y > -0.641 && y <-0.0234) && abs(y - sqrt(x * x + z * z) * (-1.0644108554) - 1.1013629509) <= 0.01) ? 1.0 : 0.0; });
    object.addPressureLoad(5000.0, [](double x, double y, double z, double){ return ((y > -1.944 && y < -1.7235) && abs(y - sqrt(x * x + z * z) * (-1.0018498686) + 1.3808172524) <= 0.01) ? 1.0 : 0.0; });
    object.addPressureLoad(5000.0, [](double x, double y, double z, double){ return ((y > -1.944 && y < -0.6431) && abs(y - sqrt(x * x + z * z) * 1.3260378897 + 2.8163434974) <= 0.01) ? 1.0 : 0.0; });

    // Запуск расчета
    if (object.start())
    {
        object.saveResult(object.stdResName());
        object.printResult(object.stdTxtResName());
    }

}

/*
double tank3_new_thickness(double, double y, double, double)
{
    double L = 0.22,
           shpangout_top = 0.0435,
           shpangout_bot = 0.047,
           R = 1.037;

    if (y > (L / 2 + shpangout_top + sqrt(R*R - 0.17*0.17))) // d = 340
        return 0.0032;
    if (y > (L / 2 + shpangout_top + sqrt(R*R - 0.365*0.365))) // d = 730
        return 0.0019;
    if (y > (L / 2 + shpangout_top + sqrt(R*R - 0.418*0.418))) // d = 836
        return 0.0032;
    if (y > (L / 2 + shpangout_top + sqrt(R*R - 0.94*0.94))) // d = 1880
        return 0.0019;
    if (y > (L / 2 + shpangout_top + sqrt(R*R - 0.9645*0.9645))) // d = 1929
        return 0.003;
    if (y > (L / 2 + shpangout_top + sqrt(R*R - 1.029*1.029))) // d = 2058
        return 0.0019;
    if (y > (L / 2 + 0.061))
        return 0.003;
    if (y > (L / 2 + 0.031))
        return 0.058; //
    if (y > L / 2 - 0.020)
        return 0.003;
    if (abs(y) <= L / 2 - 0.020)
        return 0.0023;
    if (y > -L / 2)
        return 0.003;
    if (y > -(L/2 + shpangout_bot))
        return 0.007;
    if (y > -(L/2 + shpangout_bot + sqrt(R*R - 0.479*0.479)))
        return 0.0015;
    if (y > -(L/2 + shpangout_bot + sqrt(R*R - 0.4365*0.4365)))
        return 0.0024;
    if (y > -(L/2 + shpangout_bot + sqrt(R*R - 0.425*0.425)))
        return 0.007;
    if (y > -(L/2 + shpangout_bot + sqrt(R*R - 0.403*0.403)))
        return 0.0125;
    return 0.0024;
}
*/
double tank3_new_thickness(double, double y, double, double)
{
    double L = 0.22,
           Lt = 0.0435,
           Lb = 0.047,
           R = 1.037;

    if (y > (L / 2 + Lt + sqrt(R*R - 0.17*0.17)) or y > (L / 2 + Lt + sqrt(R*R - 0.418*0.418)))
        return 0.0032;
    if (y > (L / 2 + Lt + sqrt(R*R - 0.365*0.365)) or y > (L / 2 + Lt + sqrt(R*R - 0.94*0.94)) or y > (L / 2 + Lt + sqrt(R*R - 1.029*1.029)))
        return 0.0019;
    if (y > (L / 2 + Lt + sqrt(R*R - 0.9645*0.9645)) or y > (L / 2 + 0.061) or y > L / 2 - 0.020 or y > -L / 2)
        return 0.003;
    if (y > (L / 2 + 0.031))
        return 0.058;
    if (y > -(L / 2 - 0.020))
        return 0.0023;
    if (y > -(L/2 + Lb) or y > -(L/2 + Lb + sqrt(R*R - 0.425*0.425)))
        return 0.007;
    if (y > -(L/2 + Lb + sqrt(R*R - 0.479*0.479)))
        return 0.0015;
    if (y > -(L/2 + Lb + sqrt(R*R - 0.4365*0.4365)))
        return 0.0024;
    if (y > -(L/2 + Lb + sqrt(R*R - 0.403*0.403)))
        return 0.0125;
    return 0.0024;
}

void calcNewTank3(void)
{
    TFEMObject object;
    double eps = 0.001,
           L = 0.22,
           Lt = 0.0435,
           P = 1;
    matrix<double> ssc = { {1.25525e+08, 0.001882}, {1.27486e+08, 0.002}, {1.37293e+08, 0.00241}, {1.471e+08, 0.0031}, {1.56906e+08, 0.0041}, {1.66713e+08, 0.0055}, {1.7652e+08, 0.008}, {1.86326e+08, 0.013}, {1.96133e+08, 0.0188}, {3.13813e+08, 0.12}};

    if (!object.setMeshFile("../../QFEM/mesh/tank3-new/tank3-new.trpa"))
        return;
    object.setNumThread(8);
    object.setTaskParam(FEMType::StaticProblem);
    // Упругие характеристики
    object.addYoungModulus(6.67E+10);
    object.addPoissonRatio(0.3);
    // Толщина КЭ
    object.addThickness(tank3_new_thickness);

    // Граничные условия
    object.addBoundaryCondition(Direction::X | Direction::Y | Direction::Z, 0.0, [&](double, double y, double, double){ return (abs(y - L / 2 - Lt) <= eps) ? 1.0 : 0.0; });
    object.addBoundaryCondition(Direction::X, 0.0, [&](double x, double, double, double){ return (abs(x) < eps) ? 1.0 : 0.0; });
    object.addBoundaryCondition(Direction::Z, 0.0, [&](double, double, double z, double){ return (abs(z) < eps) ? 1.0 : 0.0; });
    // Распределенная поверхностная нагрузка
    object.addPressureLoad(P, [&](double, double y, double, double){ return (y - L / 2 - Lt >= 0) ? 1.0 : 0.0; });
    object.addPressureLoad(P / 2.0, [&](double, double y, double, double){ return (y - L / 2 - Lt < 0) ? 1.0 : 0.0; });
    // Диаграмма деформирования
    object.addStressStrainCurve(ssc);
    // Шаг по нагрузке
    object.setLoadStep(5);
    // Способ расчета пластичности
    object.setPlasticityMethod(PlasticityMethod::MVS);


    // Запуск расчета
    if (object.start())
    {
        object.saveResult(object.stdResName());
        object.printResult(object.stdTxtResName());
    }

}

//void simpleTest(void)
//{
//    TMesh mesh;
//    TResultList result;
//    TFEMParams param;
//    list<string> note;
//    TFEM *fem;


//    if (mesh.setMesh("/home/serg/work/python/pyfem/mesh/plate4.trpa"))
//        return;
//    param.plist.addYoungModulus(203200, "");
//    param.plist.addPoissonRatio(0.27, "");
//    param.plist.addThickness(0.01, "");
//    param.plist.addBoundaryCondition("0", "x == -0.5 or x == 0.5 or y == -0.5 or y == 0.5", Direction::X | Direction::Y | Direction::Z);
//    param.plist.addPressureLoad(0.05,"");

//    fem = new TFEMStatic<TEigenSolver, TShape2D4, TFE2DP>("Plate", &mesh, &result, &note);
//    try
//    {
//        fem->setParams(param);
//        fem->startProcess();
//    }
//    catch(...)
//    {
//        cerr << "Error!" << endl;
//    }
//    delete fem;
//}

//#include "fem/femstatic.h"
//#include "analyse/analyse.h"
//#include "solver/eigensolver.h"

//TMessenger *msg =  new TMessenger();

//int main()
//{
//    matrix<double> x = {{-0.5, -0.5}, {0, -0.5}, {0.5, -0.5}, {0.5, 0}, {0.5, 0.5}, {0, 0.5}, {-0.5, 0.5}, {-0.5, 0}, {0, 0}};
//    matrix<unsigned> fe = {{0, 1, 8, 7}, {1, 2, 3, 8}, {3, 4, 5, 8}, {5, 6, 7, 8}},
////    matrix<unsigned> fe = {{0, 1, 8}, {0, 8, 7}, {1, 2, 8}, {2, 3, 8}, {3, 4, 8}, {4, 5, 8}, {5, 6, 8}, {6, 7, 8}},
//                     be = fe;
////    TMesh mesh(FEType::fe2d3p, x, fe, be);
//    TMesh mesh(FEType::fe2d4p, x, fe, be);
//    TResults result;
//    TFEMParams params;
////    TFEMStatic<TEigenSolver, TShape2D3, TFE2DP> fem("Plate", &mesh, &result);
//    TFEMStatic<TEigenSolver, TShape2D4, TFE2DP> fem("Plate", &mesh, &result);

//    params.addYoungModulus(203200);
//    params.addPoissonRatio(0.27);
//    params.addThickness(0.01);
//    params.addBoundaryCondition(Direction::X | Direction::Y | Direction::Z, 0, "x == -0.5 or x == 0.5 or y == -0.5 or y == 0.5");
//    params.addPressureLoad(0.05);

//    fem.setParams(params);
//    fem.startProcess();

//    delete msg;
//    return 0;
//}


void calcNewTank1(void)
{
    TFEMObject object;
    matrix<double> ssc1 = {{1.27E+08, 0.002}, {1.57E+08, 0.004}, {1.77E+08, 0.008}, {1.96E+08, 0.02}, {3.14E+08, 0.12}};
    matrix<double> ssc2 = {{1.96E+08, 0.003}, {2.55E+08, 0.005}, {2.75E+08, 0.006}, {3.14E+08, 0.015}, {3.92E+08, 0.0725}};
    double eps = 0.001,
           e1 = 6.5e+10,
           e2 = 7.3e+10,
           // E2 = E1,
           c = 1.454,
           cx_bot = 20.7657,
           cx_top = -8.5497,
           // D = 3.9,
           fi_b = -0.872665,
           fi_t = -2.26893,
           h = 0.06,
           k2_bot = 0.0520196,
           k2_top = 0.0520196,
           l = 12.216,
           // L1 = 1.767,
           l2 = 2.122,
           l3 = 1.654,
           l4 = 1.09,
           p = 142196,
           r = 2.5;

    if (!object.setMeshFile("/home/serg/work/tank-new/tank_1_4.trpa"))
    //if (!object.setMeshFile("/home/serg/work/tank-new/tank.trpa"))
    // if (!object.setMeshFile("D:/work/tank-new/tank_1.trpa"))



    // if (!object.setMeshFile("D:/work/tank-new/tank_2.trpa"))
    // if (!object.setMeshFile("D:/work/tank-new/tank_1_4.trpa"))
        return;
    object.setNumThread(7);
    object.setTaskParam(FEMType::StaticProblem);

    // Упругие характеристики
    object.addYoungModulus(e1, [&](double x, double y, double z, double) {
        return (abs(r * r - ((x - c) * (x - c) + y * y + z * z)) <= eps and x <= (r * cos(fi_t) + c)) or (abs(r * r - ((x - l + c) * (x - l + c) + y * y + z * z)) <= eps and x >= (r * cos(fi_b) + l - c)) ? 1.0 : 0.0;
    });
    object.addYoungModulus(e2);
    object.addPoissonRatio(0.3);

    // Толщина КЭ
    object.addThickness(0.0046, [&](double x, double y, double z, double) {
        return ((abs(r * r - ((x - c) * (x - c) + y * y + z * z)) <= eps) && (x <= (r * cos(fi_t) + c))) || ((abs(r * r - ((x - l + c) * (x - l + c) + y * y + z * z)) <= eps) && (x >= (r * cos(fi_b)) + l - c)) ? 1.0 : 0.0;
    });
    object.addThickness(0.05, [&](double x, double, double, double) {
        return ((x >= (r * cos(fi_t) + c)) && (x <= 0.)) || ((x >= l) && (x <= (r * cos(fi_b) + l - c))) || ((x >= 4. * l3 - h / 2.) && (x <= 4. * l3 + h / 2.)) ? 1.0 : 0.0;
    });
    object.addThickness(0.0255, [&](double x, double, double, double) {
        return ((x >= l3 - h / 2.0) && (x <= l3 + h / 2.)) || ((x >= 2. * l3 - h / 2.) && (x <= 2. * l3 + h / 2.)) || ((x >= 5. * l3 - h / 2.) && (x <= 5. * l3 + h / 2.)) || ((x >= 6. * l3 - h / 2.) && (x <= 6. * l3 + h / 2.)) || ((x >= 6. * l3 - h / 2. + l4) && (x <= 6. * l3 + h / 2. + l4)) ? 1.0 : 0.0;
    });
    object.addThickness(0.04, [&](double x, double, double, double) {
        return (x >= 3. * l3 - h) && (x <= 3. * l3 + h) ? 1.0 : 0.0;
    });
    object.addThickness(0.0045, [&](double x, double, double, double) {
        return (x >= 0. && x <= (l3 - h / 2.)) || (x >= (l3 + h / 2.) && x <= (2. * l3 - h / 2.)) || (x >= (2. * l3 + h / 2.) && x <= (3. * l3 - h)) || (x >= (4. * l3 + h / 2.) && x <= (5. * l3 - h / 2.)) || (x >= (5. * l3 + h / 2.) && x <= (6. * l3 - h / 2.)) ? 1.0 : 0.0;
    });
    object.addThickness(0.0046, [&](double x, double, double, double) {
        return (x >= (3. * l3 + h)) && (x <= (4. * l3 - h / 2.)) ? 1.0 : 0.0;
    });
    object.addThickness(0.0052, [&](double x, double, double, double) {
        return (x >= (6. * l3 + h / 2.) && x <= (6. * l3 - h / 2. + l4)) || (x >= (6. * l3 + h / 2. + l4) && x <= l) ? 1.0 : 0.0;
    });
    object.addThickness(0.0143, [&](double x, double, double, double) {
        return x < 0. ? 1.0 : 0.0;
    });
    object.addThickness(0.016);


    // Граничные условия
    object.addBoundaryCondition(Direction::X | Direction::Y | Direction::Z, 0.0, [&](double x, double, double, double) { return abs(x - l - l2) <= eps ? 1.0 : 0.0; });
    object.addBoundaryCondition(Direction::Y, 0.0, [&](double, double y, double, double) { return abs(y) <= eps ? 1.0 : 0.0; });
    object.addBoundaryCondition(Direction::Z, 0.0, [&](double, double, double z, double) { return abs(z) <= eps ? 1.0 : 0.0; });

    // Распределенная поверхностная нагрузка
    object.addPressureLoad(p, [&](double x, double, double, double) {
        return x >= 0 and x <= l ? 1.0 : 0.0;
    });
    object.addPressureLoad(p, [&](double x, double y, double z, double) {
        return abs(r * r - ((x - c) * (x - c) + y * y + z * z)) <= eps and x <= (r * cos(fi_t) + c) ? 1.0 : 0.0;
    });
    object.addPressureLoad(p, [&](double x, double y, double z, double) {
        return abs(r * r - ((x - l + c) * (x - l + c) + y * y + z * z)) <= eps and x >= (r * cos(fi_b) + l - c) ? 1.0 : 0.0;
    });
    object.addPressureLoad(p, [&](double x, double y, double z, double) {
        return (x>= (r * cos(fi_t) + c) and x <= 0) and abs(y * y + z * z - k2_top * (x - cx_top) * (x - cx_top)) < eps ? 1.0 : 0.0;
    });
    object.addPressureLoad(p, [&](double x, double y, double z, double) {
      return (x >= l and x <= (r * cos(fi_b) + l - c)) and abs(y * y + z * z  - k2_bot * (x - cx_bot) * (x - cx_bot)) < eps ? 1.0 : 0.0;
    });
    // Диаграмма деформирования
    object.addStressStrainCurve(ssc1, [&](double x, double y, double z, double) {
        return (abs(r * r - ((x - c) * (x - c) + y * y + z * z)) <= eps and x <= (r * cos(fi_t) + c)) or (abs(r * r - ((x - l + c) * (x - l + c) + y * y + z * z)) <= eps and x >= (r * cos(fi_b) + l - c)) ? 1.0 : 0.0;
    });
    object.addStressStrainCurve(ssc2);
    // Шаг по нагрузке
    object.setLoadStep(5);
    // Способ расчета пластичности
    object.setPlasticityMethod(PlasticityMethod::MVS);


    // Запуск расчета
    if (object.start())
    {
        object.saveResult(object.stdResName());
        object.printResult(object.stdTxtResName());
    }

}

// Тестовый расчет оболочки
void calcShell(void)
{
    // Свойства материала АМг6М
    TFEMObject object;
    double z_min = 0.0,
           z_max = 4.014,
           P = 3.65E+07;
    matrix<double> ssc = {{1.27E+08, 0.002}, {1.57E+08, 0.004}, {1.77E+08, 0.008}, {1.96E+08, 0.02}, {3.14E+08, 0.12}};

    if (!object.setMeshFile("../../QFEM/mesh/shell-tube-3.trpa"))
        return;

    object.setNumThread(7);
    object.setTaskParam(FEMType::StaticProblem);

    // object.setEps(1.0E-8);
    object.setWidth(15);
    object.setPrecision(5);

    // Упругие характеристики
    object.addYoungModulus(6.5e+10);
    object.addPoissonRatio(0.3);
    // Толщина КЭ
    object.addThickness(0.0045);
    // Распределенная поверхностная нагрузка
    object.addPressureLoad([&](double, double, double, double){ return P; });
    // Граничные условия
    object.addBoundaryCondition(Direction::X | Direction::Y | Direction::Z, [](double, double, double, double){ return 0.0; }, [&](double, double, double z, double){ return z == z_min || z == z_max ? 1.0 : 0.0; });
    // Диаграмма деформирования
    object.addStressStrainCurve(ssc);
    // Шаг по нагрузке
    object.setLoadStep(5);
    // Способ расчета пластичности
    object.setPlasticityMethod(PlasticityMethod::MVS);

    // Запуск расчета
    if (object.start())
    {
        object.saveResult(object.stdResName());
        object.printResult(object.stdTxtResName());
    }

}

// Расчет задачи для И.В. Зиновеева
void calcZinoveev(void)
{
    TFEMObject object;
    double y_min = 0.0,
           y_max = 10,
           h = 1,
           e0 = 3.86e+10,
           e1 = 3.31e+3,
           m0 = 0.2,
           m1 = 0.3;

    if (!object.setMeshFile("../../QFEM/mesh/body.vol"))
        return;

    object.setNumThread(4);
    object.setTaskParam(FEMType::StaticProblem);

    // Упругие характеристики
    object.addYoungModulus(e0,[&](double, double y, double, double){ return y >= y_max - h ? 1.0 : 0.0; });
    object.addYoungModulus(e1,[&](double, double y, double, double){ return y < y_max - h ? 1.0 : 0.0; });
    object.addPoissonRatio(m0,[&](double, double y, double, double){ return y >= y_max - h ? 1.0 : 0.0; });
    object.addPoissonRatio(m1,[&](double, double y, double, double){ return y < y_max - h ? 1.0 : 0.0; });

    // Распределенная поверхностная нагрузка
    object.addPressureLoad([&](double x, double, double, double){ return -1.0 / (x * x + 1.0); }, [&](double, double y, double , double){ return y == y_max ? 1.0 : 0.0; });

    // Граничные условия
    object.addBoundaryCondition(Direction::X | Direction::Y | Direction::Z, [](double, double, double, double){ return 0.0; }, [&](double, double y, double , double){ return y == y_min ? 1.0 : 0.0; });

    // Запуск расчета
    if (object.start())
    {
        object.saveResult(object.stdResName());
        object.printResult(object.stdTxtResName());
    }

}


int main()
{
    msg = new TMessenger();

    // calcNewTank1();
    // calcNewTank3();
    // calcTank();
    // calcBalka();
    // calcShell();
    // pyfem_test();
    // calcTank3ds();
    // calcTank3s6();
    calcZinoveev();

    delete msg;
    return 0;
}

