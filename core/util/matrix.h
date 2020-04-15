#ifndef MATRIX_H
#define MATRIX_H

//#define DEBUG

#include <algorithm>
#include <functional>
#include <vector>
#include <iostream>
#include <cmath>

using namespace std;

template <typename T> vector<T> operator + (const vector<T>& left, const vector<T>& right)
{
    vector<T> r(left.size());

#ifdef DEBUG
    if (left.size() != right.size())
        cerr << "vector<T> operator + (const vector<T>& left, const vector<T>& right) " << left.size() << ' ' << right.size() << endl;
#endif

    transform(left.begin(), left.end(), right.begin(), r.begin(), std::plus<T>());
    return r;
}

template <typename T> void operator += (vector<T>& left, const vector<T>& right)
{
#ifdef DEBUG
    if (left.size() != right.size())
        cerr << "vector<T> operator += (const vector<T>& left, const vector<T>& right) " << left.size() << ' ' << right.size() << endl;
#endif
    transform(left.begin(), left.end(), right.begin(), left.begin(), plus<T>());
}

template <typename T> vector<T> operator * (const vector<T>& left, T right)
{
    vector<T> r = left;

    transform(r.begin(), r.end(), r.begin(), bind1st(multiplies<T>(),right));
    return r;
}

template <typename T> vector<T> operator *= (vector<T>& left, T right)
{
    transform(left.begin(), left.end(), left.begin(), bind1st(multiplies<T>(), right));
}

template <typename T> vector<T> operator * (T left, const vector<T>& right)
{
    return right*left;
}

template <typename T> T sum(const vector<T>& val)
{
    T sum_of_elems = 0;

    for_each(val.begin(), val.end(), [&] (T n) { sum_of_elems += n;});
    return sum_of_elems;
}

template <typename T> void norm3(vector<T>& res)
{
    T norm = sqrt(res[0] * res[0] + res[1] * res[1] + res[2] * res[2]);

    for (unsigned k = 0; k < 3; k++)
        res[k] /= norm;
}

template <typename T> vector<T> createVector3(T* xi, T* xj)
{
    vector<T> res(3);

    for (unsigned k = 0; k < 3; k++)
        res[k] = xj[k] - xi[k];
    norm3(res);
    return res;
}

template <typename T> vector<T> crossProduct3(vector<T>& a, vector<T>& b)
{
    vector<T> res(3);

    res[0] = a[1] * b[2] - a[2] * b[1];
    res[1] = a[2] * b[0] - a[0] * b[2];
    res[2] = a[0] * b[1] - a[1] * b[0];
    norm3(res);
    return res;
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
    matrix(const matrix<T>& r)
    {
        rows = r.rows;
        cols = r.cols;
        buffer = r.buffer;
    }
    matrix(initializer_list< initializer_list<T> > r)
    {
        rows = unsigned(r.size());
        cols = unsigned(r.begin()[0].size());
        for (auto i: r)
            for (auto j: i)
            {
#ifdef DEBUG
                if (i.size() != cols)
                    cerr << "initializer_list error" << endl;
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
    matrix operator = (const matrix& r)
    {
        rows = r.rows;
        cols = r.cols;
        buffer = r.buffer;
        return *this;
    }
    void operator += (const matrix& r)
    {
#ifdef DEBUG
        if (rows != r.rows && cols != r.cols)
            cerr << "void operator += (const matrix& r) " << rows << ' ' << r.rows << ' ' << cols << ' ' << r.cols << endl;
#endif
        buffer += r.buffer;
    }
    void operator *= (T right)
    {
        buffer *= right;
    }
    T* operator [] (unsigned i)
    {
#ifdef DEBUG
        if (i * cols >= rows * cols)
            cerr << "T* operator [] (unsigned i) " << i * cols << ' ' << rows * cols << endl;
#endif
        return buffer.data() + i * cols;
    }
    const T* operator [] (unsigned i) const
    {
#ifdef DEBUG
        if (i * cols >= rows * cols)
            cerr << "T* operator [] (unsigned i) " << i * cols << ' ' << rows * cols << endl;
#endif
        return buffer.data() + i * cols;
    }
    T& operator () (unsigned i, unsigned j)
    {
#ifdef DEBUG
        if (i * cols + j >= rows * cols)
            cerr << "T& operator () (unsigned i, unsigned j) " << i * cols + j << ' ' << rows * cols << endl;
#endif
        return (buffer.data() + i * cols)[j];
    }
    const T& operator () (unsigned i, unsigned j) const
    {
#ifdef DEBUG
        if (i * cols + j >= rows * cols)
            cerr << "T& operator () (unsigned i, unsigned j) " << i * cols + j << ' ' << rows * cols << endl;
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
    friend ostream& operator << (ostream& out, matrix& r)
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
    T* data(void)
    {
        return buffer.data();
    }
    vector<T>& asVector(void)
    {
        return buffer;
    }
};

template <typename T> matrix<T> transpose(const vector<T>& v)
{
    matrix<T> res(unsigned(v.size()), 1);

    for (unsigned i = 0; i < v.size(); i++)
        res[i][0] = v[i];
    return res;
}


template <typename T> matrix<T> operator + (const matrix<T>& left, const matrix<T>& right)
{
    matrix<T> res = left;

#ifdef DEBUG
        if (left.size1() != right.size1() && left.size2() != right.size2())
            cerr << "template <typename T> matrix<T> operator + (const matrix<T>& left, const matrix<T>& right " << left.size1() << ' ' << left.size2() << right.size1() << ' ' << right.size2() << endl;
#endif
    for (unsigned row = 0; row < left.size1(); row++)
        for (unsigned col = 0; col < left.size2(); col++)
            res[row][col] += right[row][col];
    return res;
}

template <typename T> matrix<T> operator * (const matrix<T>& left, const matrix<T>& right)
{
    matrix<T> res(left.size1(), right.size2());

#ifdef DEBUG
    if (left.size2() != right.size1())
        cerr << "template <typename T> matrix<T> operator * (const matrix<T>& left, const matrix<T>& right) " << left.size1() << ' ' << left.size2() << right.size1() << ' ' << right.size2() << endl;
#endif
    for (unsigned row = 0; row < left.size1(); row++)
        for (unsigned col = 0; col < right.size2(); col++)
            for (unsigned inner = 0; inner < left.size2(); inner++)
                res[row][col] += left[row][inner] * right[inner][col];
    return res;
}

template <typename T> matrix<T> operator * (const matrix<T>& left, const vector<T>& right)
{
    matrix<T> res;

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
    else
    {
#ifdef DEBUG
        cerr << "template <typename T> matrix<T> operator * (const matrix<T>& left, const vector<T>& right) " << left.size1() << ' ' << left.size2() << right.size() << endl;
#endif
    }



    return res;
}

template <typename T> matrix<T> operator * (const matrix<T>& left, T right)
{
    matrix<T> res = left;

    for (unsigned row = 0; row < left.size1(); row++)
        for (unsigned col = 0; col < left.size2(); col++)
            res[row][col] *= right;
    return res;
}

template <typename T> matrix<T> operator * (T left, const matrix<T>& right)
{
    matrix<T> res = right;

    return res * left;
}

template <typename T> matrix<T> operator / (const matrix<T>& left, T right)
{
    matrix<T> res = left;

    for (unsigned row = 0; row < left.size1(); row++)
        for (unsigned col = 0; col < left.size2(); col++)
            res[row][col] *= (1.0 / right);
    return res;
}

template <typename T> matrix<T> operator / (T left, const matrix<T>& right)
{
    matrix<T> res = right;

    return res / left;
}

template <typename T> matrix<T> transpose(const matrix<T>& m)
{
    matrix<T> res(m.size2(), m.size1());

    for (unsigned i = 0; i < m.size2(); i++)
        for (unsigned j = 0; j < m.size1(); j++)
            res(i, j) = m(j, i);
    return res;
}

template <typename T> T det2x2(const matrix<T>& m)
{
    return m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0);
}
template <typename T> T det3x3(const matrix<T>& m)
{
    return m(0, 0) * m(1, 1) * m(2, 2) + m(0, 1) * m(1, 2) * m(2, 0) + m(0, 2) * m(1, 0) * m(2, 1) -
           m(0, 2) * m(1, 1) * m(2, 0) - m(0, 0) * m(1, 2) * m(2, 1) - m(0, 1) * m(1, 0) * m(2, 2);
}
template <typename T> matrix<T> inv2x2(const matrix<T>& m)
{
    matrix<T> res(2, 2);

    res(0, 0) =  m(1, 1); res(0, 1) = -m(0, 1);
    res(1, 0) = -m(1, 0); res(1, 1) = m(0, 0);
    return res / det2x2(m);
}
template <typename T> matrix<T> inv3x3(const matrix<T>& m)
{
    matrix<T> res(3, 3);

    res(0, 0) = m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1); res(0, 1) = m(0, 2) * m(2, 1) - m(0, 1) * m(2, 2); res(0, 2) = m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1);
    res(1, 0) = m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2); res(1, 1) = m(0, 0) * m(2, 2) - m(0, 2) * m(2, 0); res(1, 2) = m(0, 2) * m(1, 0) - m(0, 0) * m(1, 2);
    res(2, 0) = m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0); res(2, 1) = m(0, 1) * m(2, 0) - m(0, 0) * m(2, 1); res(2, 2) = m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0);

    return res / det3x3(m);
}


#endif // MATRIX_H
