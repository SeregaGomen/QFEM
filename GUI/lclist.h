#ifndef LCLIST_H
#define LCLIST_H

#include <list>
#include <iostream>
#include <QString>
#include <QDataStream>
#include "msg/msg.h"

using namespace std;

/************ Список краевых условий для GUI ************/

enum LimitType { NoValue = 0, LimitValue = 1, LoadValue = 2 };

// Описание краевого условия в GUI
class GCondition
{
private:
    LimitType type;   // Тип краевого условия (LimitValue - граничное условие или LoadValue - нагрузка)
    unsigned index;   // Номер узла в списке граничных элементов
    unsigned direct;  // Направление краевого условия: по Х, Y, ...
    double value;     // Значение краевого условия
public:
    GCondition(void)
    {
        clear();
    }
    GCondition operator = (GCondition& r)
    {
        type = r.type;
        index = r.index;
        direct = r.direct;
        return *this;
    }
    void clear(void)
    {
        type = NoValue;
        value = index = direct = 0;
    }
    void setType(LimitType p)
    {
        type = p;
    }
    LimitType getType(void) const
    {
        return type;
    }
    void setIndex(unsigned p)
    {
        index = p;
    }
    unsigned getIndex(void) const
    {
        return index;
    }
    void setDirect(unsigned p)
    {
        direct = p;
    }
    unsigned getDirect(void) const
    {
        return direct;
    }
    void setValue(double v)
    {
        value = v;
    }
    double getValue(void)
    {
        return value;
    }
    void write(QDataStream& file)
    {
        file << index;
        file << direct;
        file << value;
        file << qint8(type);
    }
    void read(QDataStream& file)
    {
        file >> index;
        file >> direct;
        file >> value;
        file >> reinterpret_cast<qint8&>(type);
    }
};


// Класс, реализующий список краевых условий для GUI
class LimitList : public list<GCondition>
{
public:
    LimitList(void) : list() {}
    ~LimitList(void) {}
    void add(unsigned index, unsigned direct, double value, LimitType type = LoadValue)
    {
        GCondition c;

        c.setIndex(index);
        c.setDirect(direct);
        c.setValue(value);
        c.setType(type);
        push_back(c);
    }
    void write(QDataStream& out)
    {
        out << unsigned(size());  // Кол-во условий
        for (auto it = begin(); it != end(); it++)
            it->write(out);
    }
    void read(QDataStream& in)
    {
        int num;
        GCondition c;

        clear();
        in >> num;
        for (int i = 0; i < num; i++)
        {
            c.read(in);
            push_back(c);
        }
    }
};
/***********************************************/


#endif // LCLIST_H
