#include "bcprocessor.h"
#include "object/plist.h"
#include "parser/parser.h"
#include "object/object.h"

extern TMessenger* msg;

void TBCProcessor::processVertex(void)
{
    unsigned numThread = 8, //std::thread::hardware_concurrency(),
             step = object->getMesh().getNumBE() / numThread;
    int error = NO_ERR;
    vector<std::thread> thr(numThread);

    vertex.resize(int(object->getMesh().getNumVertex()));
    isStoped = false;
    msg->setProcess(BC_CREATE_PROCESS, 1, int(object->getMesh().getNumBE()), 5);
    // Обработка граничных условий
    for (unsigned i = 0; i < numThread; i++)
        thr[i] = std::thread(&TBCProcessor::calc, this, PRESSURE_LOAD_PARAMETER, i * step, (i == numThread - 1) ? object->getMesh().getNumBE() : (i + 1) * step, ref(error));
    for_each(thr.begin(), thr.end(), [](auto& t) { t.join(); });
//    calc(PRESSURE_LOAD_PARAMETER, 0, object->getMesh().getNumBE(), ref(error));
//    calc(BOUNDARY_CONDITION_PARAMETER, 0, object->getMesh().getNumBE(), ref(error));
    msg->stopProcess();
    if (error)
        cerr << endl << sayError(ErrorCode(error)) << endl;
}

void TBCProcessor::calc(int type, unsigned begin, unsigned end, int& error)
{
    bool isOK;
    double value;
    vector<double> v(3),
                   coord;

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
            if (type != it->getType())
                continue;
            if ((type == VOLUME_LOAD_PARAMETER || type == SURFACE_LOAD_PARAMETER || type == CONCENTRATED_LOAD_PARAMETER || type == BOUNDARY_CONDITION_PARAMETER) && it->getDirect() == 0)
                continue;
            if (type == PRESSURE_LOAD_PARAMETER)
                calcPressureLoad(i, *it, error);
            else if (type == SURFACE_LOAD_PARAMETER)
                calcSurfaceLoad(i, *it, error);
            else
            {
                try
                {
                    for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
                    {
                        if (it->getPredicate().length())
                        {
                            object->getMesh().getCoordVertex(object->getMesh().getBE(i, j), coord);
                            if (!(isOK = object->getParams().getPredicateValue(*it, coord)))
                                break;
                        }
                        if (!isOK)
                            continue;
                        object->getMesh().getCoordVertex(i, coord);
                        value = object->getParams().getExpressionValue(*it, coord);
                        if (type == VOLUME_LOAD_PARAMETER || type == CONCENTRATED_LOAD_PARAMETER  || type == BOUNDARY_CONDITION_PARAMETER)
                        {
                            if (type == BOUNDARY_CONDITION_PARAMETER && it->getDirect())
                                vertex[int(object->getMesh().getBE(i, j))].setX(1.0);
                            else
                            {
                                if (it->getDirect() & DIR_X)
                                    vertex[int(object->getMesh().getBE(i, j))].setX(float(value));
                                if (it->getDirect() & DIR_Y)
                                    vertex[int(object->getMesh().getBE(i, j))].setY(float(value));
                                if (it->getDirect() & DIR_Z)
                                    vertex[int(object->getMesh().getBE(i, j))].setZ(float(value));
                            }
                        }
                        else
                            vertex[int(object->getMesh().getBE(i, j))].setX(float(value));
                    }
                }
                catch (ErrorCode err)
                {
                    error = err;
                    return;
                }
            }
            if (error)
                return;
        }
    }

}

void TBCProcessor::calcPressureLoad(unsigned index, TParameter& p, int &error)
{
    double value;
    vector<double> coord,
            v(3);

    try
    {
        if (p.getPredicate().length())
            for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
            {
                object->getMesh().getCoordVertex(object->getMesh().getBE(index, j), coord);
                if (!object->getParams().getPredicateValue(p, coord))
                    return;
            }
        object->getMesh().getCenterBE(index, coord);
        value = object->getParams().getExpressionValue(p, coord);
        object->getMesh().normal(index, v);
        for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
        {
            // X
            vertex[int(object->getMesh().getBE(index, j))].setX(float(value * v[0]));
            // Y
            if (object->getMesh().getFreedom() > 1)
                vertex[int(object->getMesh().getBE(index, j))].setY(float(value * v[1]));
            // Z
            if (object->getMesh().getFreedom() > 2)
                vertex[int(object->getMesh().getBE(index, j))].setZ(float(value * v[2]));
        }
    }
    catch (ErrorCode err)
    {
        error = err;
        return;
    }
}

void TBCProcessor::calcSurfaceLoad(unsigned index, TParameter& p, int &error)
{
    double value;
    vector<double> coord;

    try
    {
        if (p.getPredicate().length())
            for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
            {
                object->getMesh().getCoordVertex(object->getMesh().getBE(index, j), coord);
                if (!object->getParams().getPredicateValue(p, coord))
                    return;
            }
        object->getMesh().getCenterBE(index, coord);
        value = object->getParams().getExpressionValue(p, coord);
        for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
        {
            // X
            if ((p.getDirect() & DIR_X) == DIR_X || (object->getMesh().isPlate() && (p.getDirect() & DIR_Z) == DIR_Z)) // X или W - для пластины
                vertex[int(object->getMesh().getBE(index, j))].setX(float(value));
            // Y
            if ((p.getDirect() & DIR_Y) == DIR_Y)
                vertex[int(object->getMesh().getBE(index, j))].setY(float(value));
            // Z
            if ((p.getDirect() & DIR_Z) == DIR_Z)
                vertex[int(object->getMesh().getBE(index, j))].setZ(float(value));
        }
    }
    catch (ErrorCode err)
    {
        error = err;
        return;
    }
}
