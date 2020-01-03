#include "lcprocessor.h"
#include "lclist.h"
#include "object/plist.h"
#include "parser/parser.h"
#include "object/object.h"

extern TMessenger* msg;

TLCProcessor::TLCProcessor(TFEMObject* p) : QObject()
{
    object = p;
    lcVertex = new LimitList();
}

TLCProcessor::~TLCProcessor(void)
{
    delete lcVertex;
}

void TLCProcessor::start(void)
{
    processBoundaryVertex();
    emit finished();
}

void TLCProcessor::stop(void)
{
    isStoped = true;
}

void TLCProcessor::clear(void)
{
    lcVertex->clear();
}

void TLCProcessor::processBoundaryVertex(void)
{
    TParser parser;
    ParameterType type;
    vector<double> v(3);
    double value;
    int num = 0;
    unsigned direct;
    bool isOK;

    // Подсчет количества краевых условий
    for (auto it = object->getParams().plist.begin(); it != object->getParams().plist.end(); it++)
        num++;
    if (!num)
        return;

    lcVertex->clear();
    parser.set_variables(object->getParams().variables);
    isStoped = false;
    msg->setProcess(BC_CREATE_PROCESS, 1, int(object->getMesh().getNumBE()) * num);
    // Обработка граничных условий
    for (auto it = object->getParams().plist.begin(); it != object->getParams().plist.end(); it++)
    {
        type = it->getType();
        direct = unsigned(it->getDirect());

        for (unsigned i = 0; i < object->getMesh().getNumBE(); i++)
        {
            msg->addProgress();
            if (isStoped)
            {
                lcVertex->clear();
                cerr << sayError(ABORT_ERR) << endl;
                return;
            }

            if (type == PRESSURE_LOAD_PARAMETER)
            {
                if (it->getPredicate().length())
                {
                    isOK = true;
                    for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
                    {
                        for (unsigned k = 0; k < object->getMesh().getDimension(); k++)
                            parser.set_variable(object->getParams().names[k], object->getMesh().getX(object->getMesh().getBE(i, j), k));
                        parser.set_variable(object->getParams().names[3], 0); // t = 0
                        parser.set_expression(it->getPredicate());
                        if (parser.get_error() != NO_ERR)
                        {
                            lcVertex->clear();
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
                parser.set_expression(it->getExpression());
                if (parser.get_error() != NO_ERR)
                {
                    lcVertex->clear();
                    return;
                }
                value = parser.run();
                object->getMesh().normal(i, v);
                for (unsigned j = 0; j < object->getMesh().getBaseSizeBE(); j++)
                {
                    // X
                    lcVertex->add(object->getMesh().getBE(i, j), DIR_X, value * v[0], LoadValue);
                    // Y
                    if (object->getMesh().getFreedom() > 1)
                        lcVertex->add(object->getMesh().getBE(i, j), DIR_Y, value * v[1], LoadValue);
                    // Z
                    if (object->getMesh().getFreedom() > 2)
                        lcVertex->add(object->getMesh().getBE(i, j), DIR_Z, value * v[2], LoadValue);
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

                    if (it->getPredicate().length())
                    {
                        parser.set_expression(it->getPredicate());
                        if (parser.get_error() != NO_ERR)
                        {
                            lcVertex->clear();
                            return;
                        }
                        if (parser.run() == 0.0)
                            continue;
                    }
                    parser.set_expression(it->getExpression());
                    if (parser.get_error() != NO_ERR)
                    {
                        lcVertex->clear();
                        return;
                    }
                    value = parser.run();
                    lcVertex->add(object->getMesh().getBE(i, j), direct, value, (type == BOUNDARY_CONDITION_PARAMETER) ? LimitValue : LoadValue);
                }
        }
    }
    msg->stopProcess();
}
