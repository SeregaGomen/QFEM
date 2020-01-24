#include <mutex>
#include "lcprocessor.h"
#include "object/plist.h"
#include "parser/parser.h"
#include "object/object.h"

extern TMessenger* msg;

void TLCProcessor::processBoundaryVertex(void)
{
    int num = 0,
        numThread = std::thread::hardware_concurrency(),
        step = object->getMesh().getNumBE() / numThread;
    int error = NO_ERR;
    vector<std::thread> thr(numThread);

    // Подсчет количества краевых условий
    for (auto it = object->getParams().plist.begin(); it != object->getParams().plist.end(); it++)
        num++;
    if (!num)
        return;

    lcVertex.clear();
    isStoped = false;
    msg->setProcess(BC_CREATE_PROCESS, 1, int(object->getMesh().getNumBE()) * num, 5);
    // Обработка граничных условий
    for (auto it = object->getParams().plist.begin(); it != object->getParams().plist.end(); it++)
    {
//        for (int i = 0; i < numThread; i++)
//            thr[i] = std::thread(&TLCProcessor::calc, this, i * step, (i == numThread - 1) ? object->getMesh().getNumBE() : (i + 1) * step, it->getType(), unsigned(it->getDirect()), it->getPredicate(), it->getExpression(), ref(error));
//        for_each(thr.begin(), thr.end(), [](std::thread& t) { t.join(); });
        calc(0, object->getMesh().getNumBE(), it->getType(), it->getDirect(), it->getPredicate(), it->getExpression(), ref(error));
        if (error)
            break;
    }
    if (error)
        cerr << endl << sayError(ErrorCode(error)) << endl;
    else
        msg->stopProcess();
}

void TLCProcessor::calc(unsigned begin, unsigned end, int type, int direct, string predicate, string expression, int& error)
{
    bool isOK;
    double value;
    vector<double> v(3);
    TParser parser;
    mutex mtx;

    parser.set_variables(object->getParams().variables);
    for (unsigned i = begin; i < end; i++)
    {
        msg->addProgress();
        if (isStoped)
        {
            error = ABORT_ERR;
            return;
        }
        if (type == PRESSURE_LOAD_PARAMETER)
        {
            if (predicate.length())
            {
                isOK = true;
                for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
                {
                    for (unsigned k = 0; k < object->getMesh().getDimension(); k++)
                        parser.set_variable(object->getParams().names[k], object->getMesh().getX(object->getMesh().getBE(i, j), k));
                    parser.set_variable(object->getParams().names[3], 0); // t = 0
                    try
                    {
                        parser.set_expression(predicate);
                    }
                    catch (ErrorCode err)
                    {
                        error = err;
                        return;
                    }
                    if (parser.run() == 0.0)
                    {
                        isOK = false;
                        break;
                    }
                }
                if (!isOK)
                    continue;
            }
            try
            {
                parser.set_expression(expression);
            }
            catch (ErrorCode err)
            {
                error = err;
                return;
            }
            value = parser.run();
            object->getMesh().normal(i, v);
            for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
            {
                lock_guard<mutex> guard(mtx);

                // X
                lcVertex.add(object->getMesh().getBE(i, j), DIR_X, value * v[0], LoadValue);
                // Y
                if (object->getMesh().getFreedom() > 1)
                    lcVertex.add(object->getMesh().getBE(i, j), DIR_Y, value * v[1], LoadValue);
                // Z
                if (object->getMesh().getFreedom() > 2)
                    lcVertex.add(object->getMesh().getBE(i, j), DIR_Z, value * v[2], LoadValue);
            }
        }
        else if (type != EMPTY_PARAMETER && type != INITIAL_CONDITION_PARAMETER)
            for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
            {
                if (!direct)
                    continue;
                for (unsigned k = 0; k < object->getMesh().getDimension(); k++)
                    parser.set_variable(object->getParams().names[k], object->getMesh().getX(object->getMesh().getBE(i, j), k));
                parser.set_variable(object->getParams().names[3], 0); // t = 0

                if (predicate.length())
                {
                    try
                    {
                        parser.set_expression(predicate);
                    }
                    catch (ErrorCode err)
                    {
                        error = err;
                        return;
                    }
                    if (parser.run() == 0.0)
                        continue;
                }
                try
                {
                    parser.set_expression(expression);
                }
                catch (ErrorCode err)
                {
                    error = err;
                    return;
                }
                value = parser.run();

                lock_guard<mutex> guard(mtx);

                lcVertex.add(object->getMesh().getBE(i, j), direct, value, (type == BOUNDARY_CONDITION_PARAMETER) ? LimitValue : LoadValue);
            }
    }

}
