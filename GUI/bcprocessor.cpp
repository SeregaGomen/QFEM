#include "bcprocessor.h"
#include "object/plist.h"
#include "parser/parser.h"
#include "object/object.h"

extern TMessenger* msg;

void TBCProcessor::processVertex(void)
{
    unsigned numThread = 4, //std::thread::hardware_concurrency() - 1,
             size = object->getMesh().getNumBE(),
             step = size / numThread;
    int error = NO_ERR;
    vector<std::thread> thr(numThread);
    auto f_ptr = std::mem_fn(&TBCProcessor::calcParam);

    // Сброс данных
    vertex.clear();
    vertex.resize(int(object->getMesh().getNumVertex()));

    if (paramType == ParamType::Pressure_load or paramType == ParamType::SurfaceLoad)
        f_ptr = std::mem_fn(&TBCProcessor::calcPressureLoad);
    else if (paramType == ParamType::VolumeLoad)
    {
        step = (size = object->getMesh().getNumFE()) / numThread;
        f_ptr = std::mem_fn(&TBCProcessor::calcVolumeLoad);
    }
    else if (paramType == ParamType::ConcentratedLoad)
    {
        step = (size = object->getMesh().getNumVertex()) / numThread;
        f_ptr = std::mem_fn(&TBCProcessor::calcConcentratedLoad);
    }
    else
        step = (size = object->getMesh().getNumVertex()) / numThread;

    isStoped = false;
    msg->setProcess(BC_CREATE_PROCESS, 1, int(size), 5);
    // Обработка граничных условий
    for (unsigned i = 0; i < numThread; i++)
        thr[i] = std::thread(f_ptr, this, i * step, (i == numThread - 1) ? size : (i + 1) * step, ref(error));
    for_each(thr.begin(), thr.end(), [](auto& t) { t.join(); });
//    calcConcentratedLoad(0, size, ref(error));
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
            for (auto it: object->getParams().plist)
                if (paramType == it.getType())
                {
                    object->getMesh().getCoordVertex(i, coord);
                    if (it.getPredicate().length() and not object->getParams().getPredicateValue(it, coord))
                        continue;
                    vertex[int(i)].setW((paramType == ParamType::BoundaryCondition) ? 1 : float(object->getParams().getExpressionValue(it, coord)));
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

void TBCProcessor::calcConcentratedLoad(unsigned begin, unsigned end, int &error)
{
    vector<double> coord;
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
            for (auto it: object->getParams().plist)
                if (it.getType() == ParamType::ConcentratedLoad)
                {
                    object->getMesh().getCoordVertex(i, coord);
                    if (it.getPredicate().length() and not object->getParams().getPredicateValue(it, coord))
                        continue;
                    value = float(object->getParams().getExpressionValue(it, coord));
                    if ((it.getDirect() & DIR_X) == DIR_X or (object->getMesh().isPlate() and (it.getDirect() & DIR_Z) == DIR_Z)) // X или W - для пластины
                        vertex[int(i)].setX(vertex[int(i)].x() + value);
                    if ((it.getDirect() & DIR_Y) == DIR_Y) // Y
                        vertex[int(i)].setY(vertex[int(i)].y() + value);
                    if ((it.getDirect() & DIR_Z) == DIR_Z) // Z
                        vertex[int(i)].setZ(vertex[int(i)].z() + value);
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

            for (auto it: object->getParams().plist)
                if (it.getType() == ParamType::Pressure_load or it.getType() == ParamType::SurfaceLoad)
                {
                    object->getMesh().getCoordBE(i, coord);
                    isTrue = true;
                    for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
                        if (it.getPredicate().length() and not object->getParams().getPredicateValue(it, coord[j][0], coord[j][1], coord[j][2]))
                        {
                            isTrue = false;
                            break;
                        }
                    if (not isTrue)
                        continue;
                    object->getMesh().getCenterBE(i, c_coord);
                    value = object->getParams().getExpressionValue(it, c_coord);
                    if (it.getType() == ParamType::Pressure_load)
                        object->getMesh().normal(i, v);
                    for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
                        if (it.getType() == ParamType::Pressure_load)
                        {
                            // X
                            vertex[int(object->getMesh().getBE(i, j))].setX(vertex[int(object->getMesh().getBE(i, j))].x() + float(value * v[0]));
                            // Y
                            if (object->getMesh().getFreedom() > 1)
                                vertex[int(object->getMesh().getBE(i, j))].setY(vertex[int(object->getMesh().getBE(i, j))].y() + float(value * v[1]));
                            // Z
                            if (object->getMesh().getFreedom() > 2)
                                vertex[int(object->getMesh().getBE(i, j))].setZ(vertex[int(object->getMesh().getBE(i, j))].z() + float(value * v[2]));
                        }
                        else
                        {
                            if ((it.getDirect() & DIR_X) == DIR_X or (object->getMesh().isPlate() and (it.getDirect() & DIR_Z) == DIR_Z)) // X или W - для пластины
                                vertex[int(object->getMesh().getBE(i, j))].setX(vertex[int(object->getMesh().getBE(i, j))].x() + float(value));
                            if ((it.getDirect() & DIR_Y) == DIR_Y) // Y
                                vertex[int(object->getMesh().getBE(i, j))].setY(vertex[int(object->getMesh().getBE(i, j))].y() + float(value));
                            if ((it.getDirect() & DIR_Z) == DIR_Z) // Y
                                vertex[int(object->getMesh().getBE(i, j))].setZ(vertex[int(object->getMesh().getBE(i, j))].z() + float(value));
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

void TBCProcessor::calcVolumeLoad(unsigned begin, unsigned end, int &error)
{
    bool isTrue;
    matrix<double> coord;
    vector<double> c_coord;
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

            for (auto it: object->getParams().plist)
                if (it.getType() == ParamType::VolumeLoad)
                {
                    object->getMesh().getCoordFE(i, coord);
                    isTrue = true;
                    for (unsigned j = 0; j < object->getMesh().getBaseSizeFE(); j++)
                        if (it.getPredicate().length() and not object->getParams().getPredicateValue(it, coord[j][0], coord[j][1], coord[j][2]))
                        {
                            isTrue = false;
                            break;
                        }
                    if (not isTrue)
                        continue;
                    object->getMesh().getCenterFE(i, c_coord);
                    value = object->getParams().getExpressionValue(it, c_coord);
                    for (unsigned j = 0; j < object->getMesh().getBaseSizeFE(); j++)
                    {
                        // X
                        if ((it.getDirect() & DIR_X) == DIR_X or (object->getMesh().isPlate() and (it.getDirect() & DIR_Z) == DIR_Z)) // X или W - для пластины
                            vertex[int(object->getMesh().getFE(i, j))].setX(float(value));
                        // Y
                        if ((it.getDirect() & DIR_Y) == DIR_Y) // Y
                            vertex[int(object->getMesh().getFE(i, j))].setY(float(value));
                        // Z
                        if ((it.getDirect() & DIR_Z) == DIR_Z) // Z
                            vertex[int(object->getMesh().getFE(i, j))].setZ(float(value));
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
