#ifndef MATRIX_H
#define MATRIX_H

//#define DEBUG

#include <cassert>
#include <algorithm>
#include <functional>
#include <vector>
#include <iostream>
#include <cmath>

using namespace std;

template <typename T> vector<T> operator + (const vector<T> &left, const vector<T> &right)
{
    vector<T> r(left.size());

#ifdef DEBUG
    assert(left.size() == right.size());
#endif
    transform(left.begin(), left.end(), right.begin(), r.begin(), std::plus<T>());
    return r;
}

template <typename T> void operator += (vector<T> &left, const vector<T> &right)
{
#ifdef DEBUG
    assert(left.size() == right.size());
#endif
    transform(left.begin(), left.end(), right.begin(), left.begin(), plus<T>());
}

template <typename T> vector<T> operator * (const vector<T> &left, T right)
{
    vector<T> r{left};

//    transform(r.begin(), r.end(), r.begin(), bind1st(multiplies<T>(), right));
    transform(r.begin(), r.end(), r.begin(), [right](T &it) -> T { return (it *= right); });
    return r;
}

template <typename T> vector<T> operator *= (vector<T> &left, T right)
{
    transform(left.begin(), left.end(), left.begin(), bind1st(multiplies<T>(), right));
}

template <typename T> vector<T> operator * (T left, const vector<T> &right)
{
    return right * left;
}

template <typename T> T sum(const vector<T> &val)
{
    T sum_of_elems{0};

    for_each(val.begin(), val.end(), [&](T n) { sum_of_elems += n;});
    return sum_of_elems;
}

template <typename T> class matrix
{
private:
    vector<T> buffer;
    unsigned rows;
    unsigned cols;
public:
    matrix(void)
    {
        rows = cols = 0;
    }
    matrix(unsigned r, unsigned c)
    {
        rows = r;
        cols = c;
        buffer.resize(rows*cols);
        fill(0);
    }
    matrix(const matrix<T> &r)
    {
        rows = r.rows;
        cols = r.cols;
        buffer = r.buffer;
    }
    matrix(initializer_list<initializer_list<T>> r)
    {
        rows = unsigned(r.size());
        cols = unsigned(r.begin()[0].size());
        for (auto i: r)
            for (auto j: i)
            {
#ifdef DEBUG
                assert(i.size() == cols);
#endif
                buffer.push_back(j);
            }
    }
    ~matrix()
    {
        buffer.clear();
    }
    void resize(unsigned r, unsigned c)
    {
        rows = r;
        cols = c;
        buffer.resize(rows*cols);
        fill(0);
    }
    matrix operator = (const matrix &r)
    {
        rows = r.rows;
        cols = r.cols;
        buffer = r.buffer;
        return *this;
    }
    void operator += (const matrix &r)
    {
#ifdef DEBUG
        assert(rows == r.rows && cols == r.cols);
#endif
        buffer += r.buffer;
    }
    void operator *= (T right)
    {
        buffer *= right;
    }
    T *operator [] (unsigned i)
    {
#ifdef DEBUG
        assert(i * cols < rows * cols);
#endif
        return buffer.data() + i * cols;
    }
    const T *operator [] (unsigned i) const
    {
#ifdef DEBUG
        assert(i * cols < rows * cols);
#endif
        return buffer.data() + i * cols;
    }
    T &operator () (unsigned i, unsigned j)
    {
#ifdef DEBUG
        assert(i * cols + j < rows * cols);
#endif
        return (buffer.data() + i * cols)[j];
    }
    const T& operator () (unsigned i, unsigned j) const
    {
#ifdef DEBUG
        assert(i * cols + j < rows * cols);
#endif
        return (buffer.data() + i * cols)[j];
    }
    unsigned size1(void) const
    {
        return rows;
    }
    unsigned size2(void) const
    {
        return cols;
    }
    friend ostream &operator << (ostream &out, matrix &r)
    {
        out.setf( ios::fixed,  ios::floatfield );
        for (unsigned i = 0; i < r.size1(); i++)
        {
            for (unsigned j = 0; j < r.size2(); j++)
            {
                out.precision(5);
                out.width(15);
                out << r[i][j] << ' ';
            }
            out << endl;
        }
        return out;
    }
    void fill(T val)
    {
        std::fill(buffer.begin(), buffer.end(), val);
    }
    void clear(void)
    {
        rows = cols = 0;
        buffer.clear();
    }
    T *data(void)
    {
        return buffer.data();
    }
    vector<T> &asVector(void)
    {
        return buffer;
    }
};

template <typename T> matrix<T> transpose(const vector<T> &v)
{
    matrix<T> res(unsigned(v.size()), 1);

    for (unsigned i = 0; i < v.size(); i++)
        res[i][0] = v[i];
    return res;
}


template <typename T> matrix<T> operator + (const matrix<T> &left, const matrix<T> &right)
{
    matrix<T> res{left};

#ifdef DEBUG
    assert(left.size1() == right.size1() && left.size2() == right.size2());
#endif
    for (unsigned row = 0; row < left.size1(); row++)
        for (unsigned col = 0; col < left.size2(); col++)
            res[row][col] += right[row][col];
    return res;
}

template <typename T> matrix<T> operator * (const matrix<T> &left, const matrix<T> &right)
{
    matrix<T> res(left.size1(), right.size2());

#ifdef DEBUG
    assert(left.size2() == right.size1());
#endif
    for (unsigned row = 0; row < left.size1(); row++)
        for (unsigned col = 0; col < right.size2(); col++)
            for (unsigned inner = 0; inner < left.size2(); inner++)
                res[row][col] += left[row][inner] * right[inner][col];
    return res;
}

template <typename T> matrix<T> operator * (const matrix<T> &left, const vector<T> &right)
{
    matrix<T> res;

#ifdef DEBUG
    assert((left.size2() == right.size()) or (left.size2() == 1 && left.size1() == right.size()));
#endif
    if (left.size2() == right.size())
    {
        res.resize(left.size1(), 1);
        for (unsigned row = 0; row < left.size1(); row++)
            for (unsigned inner = 0; inner < left.size2(); inner++)
                res[row][0] += left[row][inner] * right[inner];
    }
    else if (left.size2() == 1 && left.size1() == right.size())
    {
        res.resize(left.size1(), left.size1());
        for (unsigned row = 0; row < left.size1(); row++)
            for (unsigned col = 0; col < right.size(); col++)
                res[row][col] += left[row][0] * right[col];
    }
    return res;
}

template <typename T> matrix<T> operator * (const matrix<T> &left, T right)
{
    matrix<T> res{left};

    for (unsigned row = 0; row < left.size1(); row++)
        for (unsigned col = 0; col < left.size2(); col++)
            res[row][col] *= right;
    return res;
}

template <typename T> matrix<T> operator * (T left, const matrix<T> &right)
{
    matrix<T> res{right};

    return res * left;
}

template <typename T> matrix<T> operator / (const matrix<T> &left, T right)
{
    matrix<T> res{left};

    for (unsigned row = 0; row < left.size1(); row++)
        for (unsigned col = 0; col < left.size2(); col++)
            res[row][col] *= (1.0 / right);
    return res;
}

template <typename T> matrix<T> operator / (T left, const matrix<T> &right)
{
    matrix<T> res{right};

    return res / left;
}

template <typename T> matrix<T> transpose(const matrix<T> &m)
{
    matrix<T> res(m.size2(), m.size1());

    for (unsigned i = 0; i < m.size2(); i++)
        for (unsigned j = 0; j < m.size1(); j++)
            res(i, j) = m(j, i);
    return res;
}

template <typename T> T det(const matrix<T> &m)
{
#ifdef DEBUG
    assert(m.size1() == m.size2() && m.size1() < 4);
#endif
    return m.size1() == 1 ? m[0][0] : m.size1() == 2 ? det2x2(m) : det3x3(m);
}


template <typename T> T det2x2(const matrix<T> &m)
{
    return m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0);
}

template <typename T> T det3x3(const matrix<T> &m)
{
    return m(0, 0) * m(1, 1) * m(2, 2) + m(0, 1) * m(1, 2) * m(2, 0) + m(0, 2) * m(1, 0) * m(2, 1) -
           m(0, 2) * m(1, 1) * m(2, 0) - m(0, 0) * m(1, 2) * m(2, 1) - m(0, 1) * m(1, 0) * m(2, 2);
}

template <typename T> matrix<T> inv(const matrix<T> &m)
{
    matrix<T> res(m.size1(), m.size2());

#ifdef DEBUG
    assert(m.size1() == m.size2() && m.size1() < 4);
#endif
    if (m.size1() == 1)
        res(0, 0) = 1.0 / m(0, 0);
    return m.size1() == 1 ? res : m.size1() == 2 ? inv2x2(m) : inv3x3(m);
}

template <typename T> matrix<T> inv2x2(const matrix<T> &m)
{
    matrix<T> res(2, 2);

    res(0, 0) =  m(1, 1); res(0, 1) = -m(0, 1);
    res(1, 0) = -m(1, 0); res(1, 1) = m(0, 0);
    return res / det2x2(m);
}

template <typename T> matrix<T> inv3x3(const matrix<T> &m)
{
    matrix<T> res(3, 3);

    res(0, 0) = m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1); res(0, 1) = m(0, 2) * m(2, 1) - m(0, 1) * m(2, 2); res(0, 2) = m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1);
    res(1, 0) = m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2); res(1, 1) = m(0, 0) * m(2, 2) - m(0, 2) * m(2, 0); res(1, 2) = m(0, 2) * m(1, 0) - m(0, 0) * m(1, 2);
    res(2, 0) = m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0); res(2, 1) = m(0, 1) * m(2, 0) - m(0, 0) * m(2, 1); res(2, 2) = m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0);
    return res / det3x3(m);
}

inline bool gauss_solve(matrix<double> &a, vector<double> &b, double eps = 1.0E-10)
{
    double k1,
           k2;

    for (auto i = 0u; i < a.size1() - 1; i++)
    {
        if (fabs(a[i][i]) < eps)
            for (auto j = i + 1u; j < a.size1(); j++)
            {
                if (fabs(a[j][i]) < eps)
                    continue;
                for (auto k = 0u; k < a.size1(); k++)
                    swap(a[j][k],a[i][k]);
                swap(b[j], b[i]);
            }
        k1 = a[i][i];
        for (auto j = i + 1; j < a.size1(); j++)
        {
            if (fabs(k2 = a[j][i]) < eps)
                continue;
            for (auto k = i; k < a.size1(); k++)
                a[j][k] -= k2 * a[i][k] / k1;
            b[j] -= k2 * b[i] / k1;
        }
    }
    if (fabs(a[a.size1() - 1][a.size1() - 1]) < eps)
        return false;
    b[a.size1() - 1] /= a[a.size1() - 1][a.size1() - 1];
    for (int i = (int)a.size1() - 2; i >= 0; i--)
    {
         for (int j = i + 1; j < (int)a.size1(); j++)
             b[i] -= b[j] * a[i][j];
         if (fabs(a[i][i]) < eps)
             return false;
         b[i] /= a[i][i];
    }
    return true;
}


#endif // MATRIX_H
