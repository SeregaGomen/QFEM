#include "bcprocessor.h"
#include "object/plist.h"
#include "object/object.h"

extern TMessenger* msg;

void TBCProcessor::processVertex(void)
{
    unsigned size = object->getMesh().getNumBE(),
             step = size / numThread;
    ErrorCode error = ErrorCode::Undefined;
    vector<std::thread> thr(numThread);
    auto f_ptr = std::mem_fn(&TBCProcessor::calcParam);

    // Сброс данных
    vertex.clear();
    vertex.resize(int(object->getMesh().getNumVertex()));

    if (paramType == ParamType::PressureLoad or paramType == ParamType::SurfaceLoad)
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
    msg->setProcess(ProcessCode::GeneratingBoundaryCondition, 1, int(size), 5);
    // Обработка граничных условий
    for (unsigned i = 0; i < unsigned(numThread); i++)
        thr[i] = std::thread(f_ptr, this, i * step, (i == unsigned(numThread) - 1) ? size : (i + 1) * step, ref(error));
    for_each(thr.begin(), thr.end(), [](auto& t) { t.join(); });
//    calcConcentratedLoad(0, size, ref(error));
    msg->stopProcess();
    if (error not_eq ErrorCode::Undefined)
        cerr << endl << sayError(ErrorCode(error)) << endl;
}

void TBCProcessor::calcParam(unsigned begin, unsigned end, ErrorCode& error)
{
    vector<double> coord;

    try
    {
        for (unsigned i = begin; i < end; i++)
        {
            msg->addProgress();
            if (isStoped)
            {
                error = ErrorCode::EAbort;
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

void TBCProcessor::calcConcentratedLoad(unsigned begin, unsigned end, ErrorCode &error)
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
                error = ErrorCode::EAbort;
                return;
            }
            for (auto it: object->getParams().plist)
                if (it.getType() == ParamType::ConcentratedLoad)
                {
                    object->getMesh().getCoordVertex(i, coord);
                    if (it.getPredicate().length() and not object->getParams().getPredicateValue(it, coord))
                        continue;
                    value = float(object->getParams().getExpressionValue(it, coord));
                    if (contains(it.getDirect(), Direction::X)/* or (object->getMesh().isPlate() and contains(it.getDirect(), Direction::Z))*/) // X или W - для пластины
                        vertex[int(i)].setX(vertex[int(i)].x() + value);
                    if (contains(it.getDirect(), Direction::Y)) // Y
                        vertex[int(i)].setY(vertex[int(i)].y() + value);
                    if (contains(it.getDirect(), Direction::Z)) // Z
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

void TBCProcessor::calcPressureLoad(unsigned begin, unsigned end, ErrorCode &error)
{
    bool isTrue;
    matrix<double> coord;
    vector<double> c_coord,
                   v;
    double value;

    try
    {
        for (unsigned i = begin; i < end; i++)
        {
            msg->addProgress();
            if (isStoped)
            {
                error = ErrorCode::EAbort;
                return;
            }

            for (auto it: object->getParams().plist)
                if (it.getType() == ParamType::PressureLoad or it.getType() == ParamType::SurfaceLoad)
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
                    v = {1, 1, 1};
                    if (it.getType() == ParamType::PressureLoad)
                        v = object->getMesh().normal(i);
                    for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
                    {
                        if (contains(it.getDirect(), Direction::X) or it.getType() == ParamType::PressureLoad) // X
                            vertex[int(object->getMesh().getBE(i, j))].setX(vertex[int(object->getMesh().getBE(i, j))].x() + float(value * v[0]));
                        if (contains(it.getDirect(), Direction::Y) or it.getType() == ParamType::PressureLoad) // Y
                            vertex[int(object->getMesh().getBE(i, j))].setY(vertex[int(object->getMesh().getBE(i, j))].y() + float(value * v[1]));
                        if (contains(it.getDirect(), Direction::Z) or it.getType() == ParamType::PressureLoad) // Z или W - для пластины
                        {
                            if (object->getMesh().isPlate())
                                vertex[int(object->getMesh().getBE(i, j))].setZ(vertex[int(object->getMesh().getBE(i, j))].x() + float(value * v[2]));
                            else
                                vertex[int(object->getMesh().getBE(i, j))].setZ(vertex[int(object->getMesh().getBE(i, j))].z() + float(value * v[2]));
                        }
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

void TBCProcessor::calcVolumeLoad(unsigned begin, unsigned end, ErrorCode &error)
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
                error = ErrorCode::EAbort;
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
                        if (contains(it.getDirect(), Direction::X)) // X
                            vertex[int(object->getMesh().getFE(i, j))].setX(float(value));
                        // Y
                        if (contains(it.getDirect(), Direction::Y)) // Y
                            vertex[int(object->getMesh().getFE(i, j))].setY(float(value));
                        // Z
                        if (contains(it.getDirect(), Direction::Z)) // Z или W - для пластины
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
