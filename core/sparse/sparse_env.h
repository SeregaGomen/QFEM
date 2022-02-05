#ifndef SPARSE_ENV_H
#define SPARSE_ENV_H

// #include <cassert>
#include <vector>

using namespace std;

// Хранение разреженной симметричной матрицы
// с использованием профильной схемы
class TEnvMatrix
{
private:
    // Кол-во узлов
    unsigned nvtxs = 0;
    // Кол-во степеней свободы
    unsigned blksze = 0;
    // Диагональные элементы
    vector<double> diag;
    // Профильные элементы
    vector<double> env;
    // Индексы начала каждой строки
    vector<unsigned> xenv;
    // Факторизация матрицы
    bool esfct(bool&);
public:
    TEnvMatrix() = default;
    ~TEnvMatrix() = default;
    unsigned size(void)
    {
        return nvtxs * blksze;
    }
    void setMatrix(vector<vector<unsigned>> &map, unsigned blksze)
    {
        auto size = 0u,
             count = 0u;

        nvtxs = (unsigned)map.size();
        this->blksze = blksze;
        xenv.resize(nvtxs * blksze + 1, 0);

        for (auto i = 0u; i < nvtxs; i++)
        {
            // Длина профиля для i-го узла
            auto len = blksze * blksze * (i - map[i][0]) + (blksze - 1) * blksze / 2;
            if (len == 0)
                continue;
            size += len;
            for (auto k = 0u; k < blksze; k++)
            {
                xenv[i * blksze + k] = count;
                count += blksze * (i - map[i][0]) + k;
            }
        }
        env.resize(size, 0.);
        diag.resize(nvtxs * blksze, 0.);
        xenv[xenv.size() - 1] = (unsigned)env.size();
    }
    void setValue(unsigned i, unsigned j, double value)
    {
        if (i >= j)
        {
            if (i == j)
                diag[i] = value;
            else
            {
                if (xenv[i + 1] - i + j >= xenv[i])
                    env[xenv[i + 1] - i + j] = value;
            }
        }
    }
    void addValue(unsigned i, unsigned j, double value)
    {
        if (i >= j)
        {
            if (i == j)
                diag[i] += value;
            else
            {
                // assert(xenv[i + 1] - i + j < env.size());
                env[xenv[i + 1] - i + j] += value;
            }
        }
    }
    double getValue(unsigned i, unsigned j)
    {
        // return i == j ? diag[i] : env[xenv[i + 1] - i + j];
        if (i >= j)
        {
            if (i == j)
                return diag[i];
            else
            {
                if (xenv[i + 1] - i + j >= xenv[i])
                    return env[xenv[i + 1] - i + j];
            }
        }
        return 0;

    }
    bool solve(vector<double>&, bool&);
};

#endif // SPARSE_ENV_H
