#include "bcprocessor.h"
#include "object/plist.h"
#include "parser/parser.h"
#include "object/object.h"

extern TMessenger* msg;

void TBCProcessor::processVertex(void)
{
    unsigned numThread = 4, //std::thread::hardware_concurrency(),
             size = object->getMesh().getNumBE(),
             step = size / numThread;
    int error = NO_ERR;
    vector<std::thread> thr(numThread);
    auto f_ptr = std::mem_fn(&TBCProcessor::calcParam);


    if (paramType == PRESSURE_LOAD_PARAMETER)
        f_ptr = std::mem_fn(&TBCProcessor::calcPressureLoad);
    else if (paramType == SURFACE_LOAD_PARAMETER)
        f_ptr = std::mem_fn(&TBCProcessor::calcSurfaceLoad);
    else if (paramType == VOLUME_LOAD_PARAMETER)
        f_ptr = std::mem_fn(&TBCProcessor::calcVolumeLoad);
    else if (paramType == CONCENTRATED_LOAD_PARAMETER)
        f_ptr = std::mem_fn(&TBCProcessor::calcConcentratedLoad);
    else
        step = (size = object->getMesh().getNumVertex()) / numThread;

    vertex.resize(int(size));
    isStoped = false;
    msg->setProcess(BC_CREATE_PROCESS, 1, int(size), 5);
    // Обработка граничных условий
    for (unsigned i = 0; i < numThread; i++)
        thr[i] = std::thread(f_ptr, this, i * step, (i == numThread - 1) ? size : (i + 1) * step, ref(error));
    for_each(thr.begin(), thr.end(), [](auto& t) { t.join(); });
//    calcParam(0, size, ref(error));
    msg->stopProcess();
    if (error)
        cerr << endl << sayError(ErrorCode(error)) << endl;
}

void TBCProcessor::calcParam(unsigned begin, unsigned end, int& error)
{
    vector<double> coord;

    try
    {
        for (unsigned i = begin; i < end; i++)
        {
            msg->addProgress();
            if (isStoped)
            {
                error = ABORT_ERR;
                return;
            }
            for (auto it = object->getParams().plist.begin(); it != object->getParams().plist.end(); it++)
            {
                if (paramType != it->getType())
                    continue;
                object->getMesh().getCoordVertex(i, coord);
                if (it->getPredicate().length() && !object->getParams().getPredicateValue(*it, coord))
                    continue;
                vertex[int(i)].setW(float(object->getParams().getExpressionValue(*it, coord)));
                break;
            }
        }
    }
    catch (ErrorCode err)
    {
        error = err;
        return;
    }
}

void TBCProcessor::calcPressureLoad(unsigned begin, unsigned end, int &error)
{
    bool isTrue;
    matrix<double> coord;
    vector<double> c_coord,
                   v(3);
    double value;

    try
    {
        for (unsigned i = begin; i < end; i++)
        {
            msg->addProgress();
            if (isStoped)
            {
                error = ABORT_ERR;
                return;
            }

            for (auto it = object->getParams().plist.begin(); it != object->getParams().plist.end(); it++)
            {
                if (paramType != it->getType())
                    continue;
                object->getMesh().getCoordBE(i, coord);
                isTrue = true;
                for (unsigned j = 0; j < coord.size1(); j++)
                {
                    if (it->getPredicate().length() && !object->getParams().getPredicateValue(*it, coord[j][0], coord[j][1], coord[j][2]))
                    {
                        isTrue = false;
                        break;
                    }
                }
                if (!isTrue)
                    continue;
                object->getMesh().getCenterBE(i, c_coord);
                value = object->getParams().getExpressionValue(*it, c_coord);
                object->getMesh().normal(i, v);
                for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
                {
                    // X
                    vertex[int(object->getMesh().getBE(i, j))].setX(float(value * v[0]));
                    // Y
                    if (object->getMesh().getFreedom() > 1)
                        vertex[int(object->getMesh().getBE(i, j))].setY(float(value * v[1]));
                    // Z
                    if (object->getMesh().getFreedom() > 2)
                        vertex[int(object->getMesh().getBE(i, j))].setZ(float(value * v[2]));
                }
            }
        }
    }
    catch (ErrorCode err)
    {
        error = err;
        return;
    }
}

void TBCProcessor::calcSurfaceLoad(unsigned begin, unsigned end, int &error)
{

}

void TBCProcessor::calcVolumeLoad(unsigned begin, unsigned end, int &error)
{

}

void TBCProcessor::calcConcentratedLoad(unsigned begin, unsigned end, int &error)
{

}


//void TBCProcessor::calcPressureLoad(unsigned index, TParameter& p, int &error)
//{
//    double value;
//    vector<double> coord,
//            v(3);

//    try
//    {
//        if (p.getPredicate().length())
//            for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
//            {
//                object->getMesh().getCoordVertex(object->getMesh().getBE(index, j), coord);
//                if (!object->getParams().getPredicateValue(p, coord))
//                    return;
//            }
//        object->getMesh().getCenterBE(index, coord);
//        value = object->getParams().getExpressionValue(p, coord);
//        object->getMesh().normal(index, v);
//        for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
//        {
//            // X
//            vertex[int(object->getMesh().getBE(index, j))].setX(float(value * v[0]));
//            // Y
//            if (object->getMesh().getFreedom() > 1)
//                vertex[int(object->getMesh().getBE(index, j))].setY(float(value * v[1]));
//            // Z
//            if (object->getMesh().getFreedom() > 2)
//                vertex[int(object->getMesh().getBE(index, j))].setZ(float(value * v[2]));
//        }
//    }
//    catch (ErrorCode err)
//    {
//        error = err;
//        return;
//    }
//}

//void TBCProcessor::calcSurfaceLoad(unsigned index, TParameter& p, int &error)
//{
//    double value;
//    vector<double> coord;

//    try
//    {
//        if (p.getPredicate().length())
//            for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
//            {
//                object->getMesh().getCoordVertex(object->getMesh().getBE(index, j), coord);
//                if (!object->getParams().getPredicateValue(p, coord))
//                    return;
//            }
//        object->getMesh().getCenterBE(index, coord);
//        value = object->getParams().getExpressionValue(p, coord);
//        for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
//        {
//            // X
//            if ((p.getDirect() & DIR_X) == DIR_X || (object->getMesh().isPlate() && (p.getDirect() & DIR_Z) == DIR_Z)) // X или W - для пластины
//                vertex[int(object->getMesh().getBE(index, j))].setX(float(value));
//            // Y
//            if ((p.getDirect() & DIR_Y) == DIR_Y)
//                vertex[int(object->getMesh().getBE(index, j))].setY(float(value));
//            // Z
//            if ((p.getDirect() & DIR_Z) == DIR_Z)
//                vertex[int(object->getMesh().getBE(index, j))].setZ(float(value));
//        }
//    }
//    catch (ErrorCode err)
//    {
//        error = err;
//        return;
//    }
//}
