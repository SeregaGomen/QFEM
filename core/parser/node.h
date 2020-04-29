#ifndef NODE_H
#define NODE_H

// Абстрактный класс, описывающий узел дерева разбора
class Node
{
private:
    int use = 1; // Количество использований
public:
    Node(void) {}
    virtual ~Node(void) {}
    virtual double value(void) = 0;
    inline const int inc(void)
    {
        return ++use;
    }
    inline const int dec(void)
    {
        return --use;
    }
};

#endif // NODE_H
