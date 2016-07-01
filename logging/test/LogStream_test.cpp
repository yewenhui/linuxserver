#include "../LogStream.h"

#include <limits>
#include <stdint.h>

#define  BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(testLogStreamBoolean) {
	LogStream os;
	const LogStream::Buffer& buf = os.GetBuffer();
	BOOST_CHECK_EQUAL(buf.AsString(), string(""));
	os << true;
	BOOST_CHECK_EQUAL(buf.AsString(), string("1"));
	os << '\n';
	BOOST_CHECK_EQUAL(buf.AsString(), string("1\n"));
	os << false;
	BOOST_CHECK_EQUAL(buf.AsString(), string("1\n0"));
}

BOOST_AUTO_TEST_CASE(testLogStreamIntegers) {
	LogStream os;
	const LogStream::Buffer& buf = os.GetBuffer();
	BOOST_CHECK_EQUAL(buf.AsString(), string(""));
	os << 1;
	BOOST_CHECK_EQUAL(buf.AsString(), string("1"));
	os << 0;
	BOOST_CHECK_EQUAL(buf.AsString(), string("10"));
	os << -1;
	BOOST_CHECK_EQUAL(buf.AsString(), string("10-1"));
	os.ResetBuffer();
	os << 0 << " " << 123 << 'x' << 0x64;
	BOOST_CHECK_EQUAL(buf.AsString(), string("0 123x100"));
}

BOOST_AUTO_TEST_CASE(testLogStreamIntegerLimits) {
	LogStream os;
	const LogStream::Buffer& buf = os.GetBuffer();
	os << -2147483647;
	BOOST_CHECK_EQUAL(buf.AsString(), string("-2147483647"));
	os << (int)-2147483648;
	BOOST_CHECK_EQUAL(buf.AsString(), string("-2147483647-2147483648"));
	os << ' ';
	os << 2147483647;
	BOOST_CHECK_EQUAL(buf.AsString(), string("-2147483647-2147483648 2147483647"));
	os.ResetBuffer();

	os << std::numeric_limits<int16_t>::min();
	BOOST_CHECK_EQUAL(buf.AsString(), string("-32768"));
	os.ResetBuffer();

	os << std::numeric_limits<int16_t>::max();
	BOOST_CHECK_EQUAL(buf.AsString(), string("32767"));
	os.ResetBuffer();
	
	// ...
}
