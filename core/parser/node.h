#ifndef NODE_H
#define NODE_H

// Абстрактный класс, описывающий узел дерева разбора
class Node
{
private:
    int use = 1; // Количество использований
public:
    Node(void) noexcept {}
    virtual ~Node(void) noexcept {}
    virtual double value(void) = 0;
    inline int inc(void) noexcept
    {
        return ++use;
    }
    inline int dec(void) noexcept
    {
        return --use;
    }
};

#endif // NODE_H
