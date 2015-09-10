#ifndef STATISTIC_H
#define STATISTIC_H

#include <math.h>
#include <iostream>
#include <fstream>
#include <boost/unordered_map.hpp>
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/math/distributions.hpp>

class GammaGenerator;

extern boost::mt19937 RNG;
extern GammaGenerator GAMMA;

template <typename Generator>
inline void DumpDistribution(Generator gen, int samples, const char *file_name, bool append) {
    const int scale = 100;

    std::map<int, int> samples_count;

    for (int i = 0; i < samples; ++i) {
        samples_count[rint(gen() * scale)] += 1;
    }

    std::ofstream fout;
    if (append) {
    	fout.open(file_name, std::ios_base::out | std::ios_base::app);
    }
    else {
    	fout.open(file_name, std::ios_base::out | std::ios_base::trunc);
    }

    if (fout.good()) {
        for (std::map<int, int>::iterator it = samples_count.begin(); it != samples_count.end(); ++it) {
        	double p = it->first / double(scale);

        	if (p > -50 && p < 50) {
        		fout << p << " " << it->second << std::endl;
        	}
        }

        fout.close();
    }
}

template<typename _Tp>
inline const _Tp&
Max(const _Tp& x, const _Tp& y)
{
    return std::max(x, y);
}

template<typename _Tp>
inline const _Tp&
Min(const _Tp& x, const _Tp& y)
{
    return std::min(x, y);
}

template<typename _Tp>
inline const _Tp&
MinMax(const _Tp& min, const _Tp& x, const _Tp& max)
{
    return Min(Max(min, x), max);
}


template<class COUNT>
class VALUE
{
public:
	VALUE() {
		Count = 0;
		Total = 0;
	}

    void Set(double count, double value)
    {
        Count = count;
        Total = value * count;
    }

    void Add(double x)
    {
        Count += 1.0;
        Total += x;
    }

    void Add(double x, COUNT weight)
    {
        Count += weight;
        Total += x * weight;
    }

    double GetValue() const
    {
        return Count == 0 ? Total : Total / double(Count);
    }

    COUNT GetCount() const
    {
        return Count;
    }

private:

    COUNT Count;
    double Total;
};


class STATISTIC
{
public:

    STATISTIC();

    ~STATISTIC() {

    }

    int GetCount() const;
    double GetValue() const; //merge from VALUE
    void Set(int count, double value); //merge from VALUE

    void Initialise();
    double GetTotal() const;
    double GetMean() const;
    double GetVariance() const;
    double GetStdDev() const; //标准差
    double GetStdErr() const; //标准误差（平均值的标准差）
    double GetMax() const;
    double GetMin() const;

    void SetMin(double min) {
    	Min = min;
    }

    void SetMax(double max) {
    	Max = max;
    }

    void AdjustRange(double min, double max) {
    	Min = std::min(min, Min);
    	Max = std::max(max, Max);
    }

    void Print(const std::string& name, std::ostream& ostr) const;
    void Add(double val);

private:
    double Count;
    double Mean; //平均值
    double Variance; //方差
    double Min, Max;
};

inline STATISTIC::STATISTIC()
{
	Initialise();
}

inline void STATISTIC::Set(int count, double value)
{
	Initialise();

	Count = count;
	Mean = value;
}

inline void STATISTIC::Add(double val)
{
    double meanOld = Mean;
    int countOld = Count;

    ++Count;
    assert(Count > 0); // overflow
    Mean += (val - Mean) / Count;
    Variance = (countOld * (Variance + meanOld * meanOld) + val * val) / Count - Mean * Mean;

    if (Variance < 0.0)
    	Variance = 0.0;
    if (val > Max)
        Max = val;
    if (val < Min)
        Min = val;
}

inline void STATISTIC::Initialise()
{
    Count = 0;
    Mean = 0;
    Variance = 0;
    Min = +10e6;
    Max = -10e6;
}

inline int STATISTIC::GetCount() const
{
    return Count;
}

inline double STATISTIC::GetTotal() const
{
    return Mean * Count;
}

inline double STATISTIC::GetValue() const
{
	return GetMean();
}

inline double STATISTIC::GetMean() const
{
    return Mean;
}

inline double STATISTIC::GetVariance() const
{
	return Variance;
}

inline double STATISTIC::GetStdDev() const
{
    return sqrt(Variance);
}

inline double STATISTIC::GetStdErr() const
{
    return sqrt(Variance / Count);
}

inline double STATISTIC::GetMax() const
{
    return Max;
}

inline double STATISTIC::GetMin() const
{
    return Min;
}
    
inline void STATISTIC::Print(const std::string& name, std::ostream& ostr) const
{
    ostr << name
    		<< ": " << Mean
    		<< " (" << GetCount()
    		<< ") [" << Min
    		<< ", " << Max
    		<< "] +- " << GetStdErr()
    		<< ", sigma=" << GetStdDev()
    		<< std::endl;
}

class UniformGenerator {
public:
	UniformGenerator(double low, double high): mLow(low), mHigh(high) {

	}

	double operator()() {
		return mLow + drand48() * (mHigh - mLow);
	}

	double mLow;
	double mHigh;
};

class NormalGammaGenerator {
public:
	NormalGammaGenerator(double mu, double lambda, double alpha, double beta): Mu(mu), Lambda(lambda), Alpha(alpha), Beta(beta) {

	}

	double operator()() {
		const double t = boost::gamma_distribution<>(Alpha, 1.0 / Beta)(RNG);
		const double p = std::max(Lambda * t, 1.0e-6);
		const double m = boost::normal_distribution<>(Mu, sqrt(1.0 / p))(RNG);

		return m;
	}

private:
	const double Mu;
	const double Lambda;
	const double Alpha;
	const double Beta;
};

class NormalGammaInfo {
public:
	NormalGammaInfo():
		Mu(0.0),
		Lambda(0.0),
		Alpha(ALPHA),
		Beta(BETA) {
		Initialise();
	}

	~NormalGammaInfo() {

	}

	void Initialise() {
		Mu = 0.0;
		Lambda = 0.0;
		Alpha = ALPHA;
		Beta = BETA;
	}

	double GetValue() const {
		return Mu;
	}

	double GetCount() const {
		return Lambda;
	}

	double GetAlpha() const {
		return Alpha;
	}

	double GetBeta() const {
		return Beta;
	}

	double GetExpectation() const {
		return ThompsonSampling(false);
	}

	void Set(int count, double value) {
		Mu = value;
		Lambda = count;
		Alpha = ALPHA;
		Beta = BETA;
	}

	void Add(const std::vector<double>& values) { //add a new sample
		STATISTIC samples;
		for (uint i = 0; i < values.size(); ++i) {
			samples.Add(values[i]);
		}

		double n = samples.GetCount();
		double m = samples.GetMean();
		double s = samples.GetVariance();

		double mu = (Lambda * Mu + n * m) / (Lambda + n);
		double lambda = Lambda + n;
		double alpha = Alpha + 0.5 * n;
		double beta = Beta + 0.5 * n * s + 0.5 * (Lambda * n / (Lambda + n)) * (m - Mu) * (m - Mu) ;

		Mu = mu;
		Lambda = lambda;
		Alpha = alpha;
		Beta = beta;
	}

	double ThompsonSampling(bool sampling = true) const { //Two Step: 采样一个模型参数，并计算出该模型参数对应的期望收益
		return sampling? NormalGammaGenerator(Mu, Lambda, Alpha, Beta)(): Mu;
	}

	void Print(const std::string& name, std::ostream& ostr) const
	{
		ostr << name << ":"
			<< " mu=" << Mu
			<< " lambda=" << Lambda
			<< " alpha=" << Alpha
			<< " beta=" << Beta
			<< " error=" << sqrt(Beta / (Lambda * (Alpha - 1)))
			<< " sigma=" << sqrt(Beta / (Alpha - 1))
			<< std::endl;
	}

	static void SetALPHA(double alpha) {
		ALPHA = alpha;
	}

	static void SetBETA(double beta) {
		BETA = beta;
	}

private:
	double Mu;
	double Lambda;
	double Alpha;
	double Beta;

	static double ALPHA;
	static double BETA;
};


template <class T, class H>
class DirichletInfo {
public:
    const DirichletInfo<T, H> &operator=(const DirichletInfo<T, H> &o) {
        Alpha = o.Alpha;

        return *this;
    }

	const std::vector<std::pair<T, double> > &GetExpectation() const {
		return ThompsonSampling(false);
	}

	void Initial(const T &s) {
		Alpha[s] = 0.5; //XXX
	}

	void Clear() {
		Alpha.clear();
	}

	void Add(const T &s) {
		Alpha[s] += 1.0;
	}

	const std::vector<std::pair<T, double> > &ThompsonSampling(bool sampling = true) const { //Two Step: 采样一个模型参数，并计算出该模型参数对应的期望收益
		outcomes_.clear();

		double sum = 0.0;
        for(typename H::iterator it = Alpha.begin(); it != Alpha.end(); ++it ) {
        	outcomes_.push_back(std::make_pair(it->first, 0));
        	outcomes_.back().second = sampling? boost::gamma_distribution<>(it->second)(RNG): it->second;
        	sum += outcomes_.back().second;
        }

        for (typename std::vector<std::pair<T, double> >::iterator it = outcomes_.begin(); it != outcomes_.end(); ++it) {
        	it->second /= sum;
        }

		return outcomes_;
	}

	void Print(const std::string& name, std::ostream& ostr) const
	{
		const std::vector<std::pair<T, double> > &outcomes = GetExpectation();

		ostr << name << ": Alpha=(";
		for (typename std::vector<std::pair<T, double> >::const_iterator it = outcomes.begin(); it != outcomes.end(); ++it) {
			ostr << "(" << it->second << "), ";
		}
		ostr << ")" << std::endl;
	}

private:
	mutable H Alpha;
	mutable std::vector<std::pair<T, double> > outcomes_;
};

#endif // STATISTIC
