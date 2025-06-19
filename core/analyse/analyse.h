#ifndef ANALYSE_H
#define ANALYSE_H

#include <string>
#include <fstream>
#include <algorithm>
#include <vector>
#include <ctime>
#include <chrono>

using namespace std;

/***************************************************/
/*  Численный результат расчета для одной функции  */
/***************************************************/
class TResult
{
private:
    string name;
    vector<double> results;
    double time;
public:
    TResult(void) {}
    TResult(const TResult &r)
    {
        results = r.results;
        name = r.name;
        time = r.time;
    }
    TResult(vector<double> &res, string fname, double t = 0)
    {
        results = res;
        name = fname;
        time = t;
    }
    TResult(double *res, unsigned sz, string fname, double t = 0)
    {
        results.assign(res, res + sz);
        name = fname;
        time = t;
    }
    ~TResult(void) {}
    bool write(ofstream& out)
    {
        unsigned len = unsigned(results.size());

        out << name.c_str() << endl;
        out << time << endl;
        out << len << endl;
        if (out.fail())
            return false;
        for (unsigned i = 0; i < len; i++)
            out << results[i] << endl;
        if (out.fail())
            return false;
        return true;
    }
    bool read(ifstream &in)
    {
        string str;
        unsigned len;

        // Считываем имя функции
        in >> name;
        if (in.fail())
            return false;
        in >> time;

        in >> len;
        if (in.fail())
            return false;
        results.resize(len, 0);
        for (unsigned i = 0; i < len; i++)
            in >> results[i];
        if (in.fail())
            return false;
        return true;
    }
    vector<double> &getResults(void)
    {
        return results;
    }
    double &getResults(unsigned i)
    {
        return results[i];
    }
    TResult &operator = (const TResult &r)
    {
        name = r.name;
        results = r.results;
        time = r.time;
        return *this;
    }
    void add(const TResult &r)
    {
        results += r.results;
    }
    double min(void) const
    {
        return *min_element(results.begin(), results.end());
    }
    double max(void) const
    {
        return *max_element(results.begin(), results.end());
    }
    string getName(void)
    {
        return name;
    }
    double getTime(void)
    {
        return time;
    }
};
/***************************************************/
/*           Список результатов расчета            */
/***************************************************/
class TResults
{
private:
    time_t sdt;
    vector<TResult> result;
public:
    TResults(void) {}
    ~TResults(void) {}
    void setSolutionTime(time_t &t)
    {
        sdt = t;
    }
    void setCurrentSolutionTime(void)
    {
        sdt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    }
    time_t& getSolutionTime(void)
    {
        return sdt;
    }
    void setResult(vector<double> &res, string n, double t = 0)
    {
        TResult c(res, n, t);

        for (auto &it: result)
            if (n == it.getName())
            {
                it = c;
                return;
            }
        result.push_back(c);
    }
    void addResult(vector<double> &res, string n)
    {
        TResult c(res, n);

        for (auto &it: result)
            if (n == it.getName())
            {
                it.add(c);
                return;
            }
        result.push_back(c);
    }
    void setResult(double *res, unsigned sz, string n, double t = 0)
    {
        TResult c(res, sz, n, t);

        for (auto &it: result)
            if (n == it.getName())
            {
                it = c;
                return;
            }
        result.push_back(c);
    }
    void addResult(double *res, unsigned sz, string n)
    {
        TResult c(res, sz, n);

        for (auto &it: result)
            if (n == it.getName())
            {
                it.add(c);
                return;
            }
        result.push_back(c);
    }
    TResult &operator [] (unsigned i)
    {
        return result[i];
    }
    bool write(ofstream &out)
    {
//        out <<  localtime(&sdt)->tm_mday << ' ' << localtime(&sdt)->tm_mon << ' ' << localtime(&sdt)->tm_year << ' ' << localtime(&sdt)->tm_hour << ' ' << localtime(&sdt)->tm_min << ' ' << localtime(&sdt)->tm_sec << endl;
        out << "Results" << endl;
        out << sdt << endl;
        out << result.size() << endl;
        if (out.fail())
            return false;
        for (auto it: result)
            if (!it.write(out))
                return false;
        return !out.fail();
    }
    bool read(ifstream &in)
    {
        string str;
        int num;
        TResult c;

        result.clear();
        in >> str;
        in >> sdt;
        in >> num;
        if (in.fail())
            return false;
        for (int i = 0; i < num; i++)
        {
            if (!c.read(in))
                return false;
            result.push_back(c);
        }
        in.putback('\n');
        return !in.fail();
    }
    unsigned size(void)
    {
        return unsigned(result.size());
    }
    void clear(void)
    {
        result.clear();
    }
    int index(string n)
    {
        auto res = find_if(result.begin(), result.end(), [=](auto& it) -> bool { return it.getName() == n; });

        return (res == result.end()) ? -1 : int(res - result.begin());
    }
};

#endif //__ANALYSE_H
