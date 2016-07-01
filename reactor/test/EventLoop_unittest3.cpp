#include "../Channel.h"
#include "../EventLoop.h"

#include <stdio.h>
#include <sys/timerfd.h>

EventLoop* g_pEventLoop;

void timeout(TimeStamp receicveTime) {
	printf("%s Timeout!\n", receicveTime.ToFormattedString().c_str());
	g_pEventLoop->Quit();
}

int main() {
	printf("%s Started!\n", TimeStamp::Now().ToFormattedString().c_str());

	EventLoop loop;
	g_pEventLoop = &loop;

	int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	Channel channel(g_pEventLoop, timerfd);
	channel.SetReadCallBack(timeout);
	channel.EnableReading();

	struct itimerspec howlong;
	::bzero(&howlong, sizeof(howlong));
	howlong.it_value.tv_sec = 5;
	::timerfd_settime(timerfd, 0, &howlong, NULL);

	g_pEventLoop->Loop();
	::close(timerfd);
}
