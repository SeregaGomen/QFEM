#ifndef ANALYSE_H
#define ANALYSE_H

#include <list>
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>
#include <ctime>


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
    TResult(const TResult& r)
    {
        results = r.results;
        name = r.name;
        time = r.time;
    }
    TResult(vector<double>& res, string fname, double t = 0)
    {
        results = res;
        name = fname;
        time = t;
    }
    TResult(double* res, unsigned sz, string fname, double t = 0)
    {
        results.assign(res,res+sz);
        name = fname;
        time = t;
    }
   ~TResult(void)
    {
        clear();
    }
    void clear(void)
    {
        results.clear();
        name.clear();
    }
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
    bool read(ifstream& in)
    {
        string str;
        unsigned len;

        // Считываем имя функции
        in >> name;
        getline(in,str);
        if (in.fail())
            return false;
        in >> time;
        getline(in,str);

        in >> len;
        getline(in,str);
        if (in.fail())
            return false;
        results.resize(len, 0);
        for (unsigned i = 0; i < len; i++)
        {
            in >> results[i];
            getline(in,str);
        }
        if (in.fail())
            return false;
        return (in.fail()) ? false: true;
    }
    vector<double>& getResults(void)
    {
        return results;
    }
    double& getResults(unsigned i)
    {
        return results[i];
    }
    TResult& operator = (const TResult& r)
    {
        name = r.name;
        results = r.results;
        time = r.time;
        return *this;
    }
    void add(const TResult& r)
    {
        for (unsigned i = 0; i < results.size(); i++)
            results[i] += r.results[i];
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
class TResultList
{
private:
    tm sdt;
    vector<TResult> result;
public:
    TResultList(void) {}
   ~TResultList(void) {}
    void setSolutionTime(tm& t)
    {
        sdt = t;
    }
    void setCurrentSolutionTime(void)
    {
        time_t now = time(nullptr);

        sdt = *localtime(&now);
    }
    tm getSolutionTime(void)
    {
        return sdt;
    }
    void setResult(vector<double>& res, string n, double t = 0)
    {
        TResult c(res,n,t);

        for (unsigned i = 0; i < result.size(); i++)
            if (!n.compare(result[i].getName()))
            {
                result[i] = c;
                return;
            }
        result.push_back(c);
    }
    void addResult(vector<double>& res, string n)
    {
        TResult c(res,n);

        for (unsigned i = 0; i < result.size(); i++)
            if (!n.compare(result[i].getName()))
            {
                result[i].add(c);
                return;
            }
        result.push_back(c);
    }
    void setResult(double* res, unsigned sz, string n, double t = 0)
    {
        TResult c(res,sz,n,t);

        for (unsigned i = 0; i < result.size(); i++)
            if (!n.compare(result[i].getName()))
            {
                result[i] = c;
                return;
            }
        result.push_back(c);
    }
    void addResult(double* res, unsigned sz, string n)
    {
        TResult c(res,sz,n);

        for (unsigned i = 0; i < result.size(); i++)
            if (!n.compare(result[i].getName()))
            {
                result[i].add(c);
                return;
            }
        result.push_back(c);
    }
    TResult& operator [] (unsigned i)
    {
        return result[i];
    }
    bool write(ofstream& out)
    {
        out << sdt.tm_mday << ' ' << sdt.tm_mon << ' ' << sdt.tm_year << ' ' << sdt.tm_hour << ' ' << sdt.tm_min << ' ' << sdt.tm_sec << endl;
        out << result.size() << endl;
        if (out.fail())
            return false;
        for (unsigned i = 0; i < result.size(); i++)
            if (!result[i].write(out))
                return false;
        return !out.fail();
    }
    bool read(ifstream& in)
    {
        string str;
        int num;
        TResult c;

        result.clear();
        in >> sdt.tm_mday >> sdt.tm_mon >> sdt.tm_year >> sdt.tm_hour >> sdt.tm_min >> sdt.tm_sec;
        getline(in, str);
        in >> num;
        getline(in, str);
        if (in.fail())
            return false;
        for (int i = 0; i < num; i++)
        {
            if (!c.read(in))
                return false;
            result.push_back(c);
        }
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
        int num = 0;
        bool isFind = false;

        for (unsigned i = 0; i < result.size(); i++)
            if (!n.compare(result[i].getName()))
            {
                num = int(i);
                isFind = true;
                break;
            }
        return (isFind) ? num : -1;
    }
};

#endif //__ANALYSE_H
