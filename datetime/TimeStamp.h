#ifndef _TIME_STAMP_H_ 
#define _TIME_STAMP_H_

#include <stdint.h>
#include <string>

class TimeStamp
{
public:
	enum {
		MICRO_SECOND_PER_SECOND		= 1000 * 1000,
	};

public:
	TimeStamp();
	explicit TimeStamp(int64_t microSecondsSinceEpoch);

	void		Swap(TimeStamp& rStamp) {
		std::swap(rStamp.m_MicroSecondsSinceEpoch, m_MicroSecondsSinceEpoch);
	}

	std::string ToString() const;
	std::string ToFormattedString() const;

	int64_t		GetMicroSecondsSinceEpoch() {
		return m_MicroSecondsSinceEpoch;
	}

	bool		IsValid() const {
		return m_MicroSecondsSinceEpoch > 0;
	}

	static TimeStamp Now();
	static TimeStamp Invalid();

private:
	int64_t			m_MicroSecondsSinceEpoch;

};

inline bool operator<(TimeStamp lhs, TimeStamp rhs) {
	return lhs.GetMicroSecondsSinceEpoch() < rhs.GetMicroSecondsSinceEpoch();
}

inline bool operator==(TimeStamp lhs, TimeStamp rhs) {
	return lhs.GetMicroSecondsSinceEpoch() == rhs.GetMicroSecondsSinceEpoch();
}

inline double TimeDifference(TimeStamp high, TimeStamp low) {
	int64_t diff = high.GetMicroSecondsSinceEpoch() - low.GetMicroSecondsSinceEpoch();
	return static_cast<double>(diff) / TimeStamp::MICRO_SECOND_PER_SECOND;
}

inline TimeStamp AddTime(TimeStamp stamp, double dSeconds) {
	int64_t addVal = static_cast<int64_t>(dSeconds * TimeStamp::MICRO_SECOND_PER_SECOND);
	return TimeStamp(stamp.GetMicroSecondsSinceEpoch() + addVal);
}

#endif // _TIME_STAMP_H_
