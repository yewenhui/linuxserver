#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>

ssize_t Buffer::ReadFd(int nFd, int* pSavedErrno) {
	const size_t unWritable = WriteableBytes();
	char extraBuf[65536];
	struct iovec vec[2];
	vec[0].iov_base = BeginWrite();
	vec[0].iov_len = WriteableBytes();
	vec[1].iov_base = extraBuf;
	vec[1].iov_len = sizeof(extraBuf);
	const ssize_t n = readv(nFd, vec, 2);
	if (n < 0) {
		*pSavedErrno = errno;
	} else if (static_cast<size_t>(n) <= unWritable) {
		m_unWriterIndex += n;
	} else {
		m_unWriterIndex = m_vBuffer.size();
		Append(extraBuf, n - unWritable);
	}

	return n;
}
