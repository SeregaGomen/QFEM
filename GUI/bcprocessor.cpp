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

    if (paramType == PRESSURE_LOAD_PARAMETER || paramType == SURFACE_LOAD_PARAMETER)
        f_ptr = std::mem_fn(&TBCProcessor::calcPressureLoad);
    else if (paramType == VOLUME_LOAD_PARAMETER)
    {
        step = (size = object->getMesh().getNumFE()) / numThread;
        f_ptr = std::mem_fn(&TBCProcessor::calcVolumeLoad);
    }
    else if (paramType == CONCENTRATED_LOAD_PARAMETER)
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
            foreach (auto it, object->getParams().plist)
                if (paramType == it.getType())
                {
                    object->getMesh().getCoordVertex(i, coord);
                    if (it.getPredicate().length() && !object->getParams().getPredicateValue(it, coord))
                        continue;
                    vertex[int(i)].setW((paramType == BOUNDARY_CONDITION_PARAMETER) ? 1 : float(object->getParams().getExpressionValue(it, coord)));
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
            foreach (auto it, object->getParams().plist)
                if (it.getType() == CONCENTRATED_LOAD_PARAMETER)
                {
                    object->getMesh().getCoordVertex(i, coord);
                    if (it.getPredicate().length() && !object->getParams().getPredicateValue(it, coord))
                        continue;
                    value = float(object->getParams().getExpressionValue(it, coord));
                    if ((it.getDirect() & DIR_X) == DIR_X || (object->getMesh().isPlate() && (it.getDirect() & DIR_Z) == DIR_Z)) // X или W - для пластины
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

            foreach (auto it, object->getParams().plist)
                if (it.getType() == PRESSURE_LOAD_PARAMETER || it.getType() == SURFACE_LOAD_PARAMETER)
                {
                    object->getMesh().getCoordBE(i, coord);
                    isTrue = true;
                    for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
                        if (it.getPredicate().length() && !object->getParams().getPredicateValue(it, coord[j][0], coord[j][1], coord[j][2]))
                        {
                            isTrue = false;
                            break;
                        }
                    if (!isTrue)
                        continue;
                    object->getMesh().getCenterBE(i, c_coord);
                    value = object->getParams().getExpressionValue(it, c_coord);
                    if (it.getType() == PRESSURE_LOAD_PARAMETER)
                        object->getMesh().normal(i, v);
                    else
                        v[0] = v[1] = v[2] = 1;
                    for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
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

            foreach (auto it, object->getParams().plist)
                if (it.getType() == VOLUME_LOAD_PARAMETER)
                {
                    object->getMesh().getCoordFE(i, coord);
                    isTrue = true;
                    for (unsigned j = 0; j < object->getMesh().getBaseSizeFE(); j++)
                        if (it.getPredicate().length() && !object->getParams().getPredicateValue(it, coord[j][0], coord[j][1], coord[j][2]))
                        {
                            isTrue = false;
                            break;
                        }
                    if (!isTrue)
                        continue;
                    object->getMesh().getCenterFE(i, c_coord);
                    value = object->getParams().getExpressionValue(it, c_coord);
                    for (unsigned j = 0; j < object->getMesh().getBaseSizeFE(); j++)
                    {
                        // X
                        if ((it.getDirect() & DIR_X) == DIR_X || (object->getMesh().isPlate() && (it.getDirect() & DIR_Z) == DIR_Z)) // X или W - для пластины
                            vertex[int(object->getMesh().getFE(i, j))].setX(vertex[int(object->getMesh().getFE(i, j))].x() + float(value));
                        // Y
                        if ((it.getDirect() & DIR_Y) == DIR_Y) // Y
                            vertex[int(object->getMesh().getFE(i, j))].setY(vertex[int(object->getMesh().getFE(i, j))].y() + float(value));
                        // Z
                        if ((it.getDirect() & DIR_Z) == DIR_Z) // Z
                            vertex[int(object->getMesh().getFE(i, j))].setZ(vertex[int(object->getMesh().getFE(i, j))].z() + float(value));
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
