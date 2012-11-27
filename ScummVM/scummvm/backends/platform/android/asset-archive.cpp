/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(__ANDROID__)

#include <jni.h>

#include <sys/types.h>
#include <unistd.h>

#include "common/str.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/archive.h"
#include "common/debug.h"
#include "common/textconsole.h"

#include "backends/platform/android/jni.h"
#include "backends/platform/android/asset-archive.h"

// Must match android.content.res.AssetManager.ACCESS_*
const jint ACCESS_UNKNOWN = 0;
const jint ACCESS_RANDOM = 1;

// This might be useful to someone else.  Assumes markSupported() == true.
class JavaInputStream : public Common::SeekableReadStream {
public:
	JavaInputStream(JNIEnv *env, jobject is);
	virtual ~JavaInputStream();

	virtual bool eos() const {
		return _eos;
	}

	virtual bool err() const {
		return _err;
	}

	virtual void clearErr() {
		_eos = _err = false;
	}

	virtual uint32 read(void *dataPtr, uint32 dataSize);

	virtual int32 pos() const {
		return _pos;
	}

	virtual int32 size() const {
		return _len;
	}

	virtual bool seek(int32 offset, int whence = SEEK_SET);

private:
	void close(JNIEnv *env);

	jmethodID MID_mark;
	jmethodID MID_available;
	jmethodID MID_close;
	jmethodID MID_read;
	jmethodID MID_reset;
	jmethodID MID_skip;

	jobject _input_stream;
	jsize _buflen;
	jbyteArray _buf;
	uint32 _pos;
	jint _len;
	bool _eos;
	bool _err;
};

JavaInputStream::JavaInputStream(JNIEnv *env, jobject is) :
	_eos(false),
	_err(false),
	_pos(0)
{
	_input_stream = env->NewGlobalRef(is);
	_buflen = 8192;
	_buf = (jbyteArray)env->NewGlobalRef(env->NewByteArray(_buflen));

	jclass cls = env->GetObjectClass(_input_stream);
	MID_mark = env->GetMethodID(cls, "mark", "(I)V");
	assert(MID_mark);
	MID_available = env->GetMethodID(cls, "available", "()I");
	assert(MID_available);
	MID_close = env->GetMethodID(cls, "close", "()V");
	assert(MID_close);
	MID_read = env->GetMethodID(cls, "read", "([BII)I");
	assert(MID_read);
	MID_reset = env->GetMethodID(cls, "reset", "()V");
	assert(MID_reset);
	MID_skip = env->GetMethodID(cls, "skip", "(J)J");
	assert(MID_skip);

	// Mark start of stream, so we can reset back to it.
	// readlimit is set to something bigger than anything we might
	// want to seek within.
	env->CallVoidMethod(_input_stream, MID_mark, 10 * 1024 * 1024);
	_len = env->CallIntMethod(_input_stream, MID_available);
}

JavaInputStream::~JavaInputStream() {
	JNIEnv *env = JNI::getEnv();
	close(env);

	env->DeleteGlobalRef(_buf);
	env->DeleteGlobalRef(_input_stream);
}

void JavaInputStream::close(JNIEnv *env) {
	env->CallVoidMethod(_input_stream, MID_close);

	if (env->ExceptionCheck())
		env->ExceptionClear();
}

uint32 JavaInputStream::read(void *dataPtr, uint32 dataSize) {
	JNIEnv *env = JNI::getEnv();

	if (_buflen < jint(dataSize)) {
		_buflen = dataSize;

		env->DeleteGlobalRef(_buf);
		_buf = static_cast<jbyteArray>(env->NewGlobalRef(env->NewByteArray(_buflen)));
	}

	jint ret = env->CallIntMethod(_input_stream, MID_read, _buf, 0, dataSize);

	if (env->ExceptionCheck()) {
		warning("Exception during JavaInputStream::read(%p, %d)",
				dataPtr, dataSize);

		env->ExceptionDescribe();
		env->ExceptionClear();

		_err = true;
		ret = -1;
	} else if (ret == -1) {
		_eos = true;
		ret = 0;
	} else {
		env->GetByteArrayRegion(_buf, 0, ret, static_cast<jbyte *>(dataPtr));
		_pos += ret;
	}

	return ret;
}

bool JavaInputStream::seek(int32 offset, int whence) {
	JNIEnv *env = JNI::getEnv();
	uint32 newpos;

	switch (whence) {
	case SEEK_SET:
		newpos = offset;
		break;
	case SEEK_CUR:
		newpos = _pos + offset;
		break;
	case SEEK_END:
		newpos = _len + offset;
		break;
	default:
		debug("Unknown 'whence' arg %d", whence);
		return false;
	}

	jlong skip_bytes;
	if (newpos > _pos) {
		skip_bytes = newpos - _pos;
	} else {
		// Can't skip backwards, so jump back to start and skip from there.
		env->CallVoidMethod(_input_stream, MID_reset);

		if (env->ExceptionCheck()) {
			warning("Failed to rewind to start of asset stream");

			env->ExceptionDescribe();
			env->ExceptionClear();

			return false;
		}

		_pos = 0;
		skip_bytes = newpos;
	}

	while (skip_bytes > 0) {
		jlong ret = env->CallLongMethod(_input_stream, MID_skip, skip_bytes);

		if (env->ExceptionCheck()) {
			warning("Failed to skip %ld bytes into asset stream",
					static_cast<long>(skip_bytes));

			env->ExceptionDescribe();
			env->ExceptionClear();

			return false;
		} else if (ret == 0) {
			warning("InputStream->skip(%ld) didn't skip any bytes. Aborting seek.",
					static_cast<long>(skip_bytes));

			// No point looping forever...
			return false;
		}

		_pos += ret;
		skip_bytes -= ret;
	}

	_eos = false;
	return true;
}

// Must match android.content.res.AssetFileDescriptor.UNKNOWN_LENGTH
const jlong UNKNOWN_LENGTH = -1;

// Reading directly from a fd is so much more efficient, that it is
// worth optimising for.
class AssetFdReadStream : public Common::SeekableReadStream {
public:
	AssetFdReadStream(JNIEnv *env, jobject assetfd);
	virtual ~AssetFdReadStream();

	virtual bool eos() const {
		return _eos;
	}

	virtual bool err() const {
		return _err;
	}

	virtual void clearErr() {
		_eos = _err = false;
	}

	virtual uint32 read(void *dataPtr, uint32 dataSize);

	virtual int32 pos() const {
		return _pos;
	}

	virtual int32 size() const {
		return _declared_len;
	}

	virtual bool seek(int32 offset, int whence = SEEK_SET);

private:
	void close(JNIEnv *env);

	int _fd;
	jmethodID MID_close;
	jobject _assetfd;
	jlong _start_off;
	jlong _declared_len;
	uint32 _pos;
	bool _eos;
	bool _err;
};

AssetFdReadStream::AssetFdReadStream(JNIEnv *env, jobject assetfd) :
	_eos(false),
	_err(false),
	_pos(0)
{
	_assetfd = env->NewGlobalRef(assetfd);

	jclass cls = env->GetObjectClass(_assetfd);
	MID_close = env->GetMethodID(cls, "close", "()V");
	assert(MID_close);

	jmethodID MID_getStartOffset =
		env->GetMethodID(cls, "getStartOffset", "()J");
	assert(MID_getStartOffset);
	_start_off = env->CallLongMethod(_assetfd, MID_getStartOffset);

	jmethodID MID_getDeclaredLength =
		env->GetMethodID(cls, "getDeclaredLength", "()J");
	assert(MID_getDeclaredLength);
	_declared_len = env->CallLongMethod(_assetfd, MID_getDeclaredLength);

	jmethodID MID_getFileDescriptor =
		env->GetMethodID(cls, "getFileDescriptor",
							"()Ljava/io/FileDescriptor;");
	assert(MID_getFileDescriptor);
	jobject javafd = env->CallObjectMethod(_assetfd, MID_getFileDescriptor);
	assert(javafd);

	jclass fd_cls = env->GetObjectClass(javafd);
	jfieldID FID_descriptor = env->GetFieldID(fd_cls, "descriptor", "I");
	assert(FID_descriptor);

	_fd = env->GetIntField(javafd, FID_descriptor);
}

AssetFdReadStream::~AssetFdReadStream() {
	JNIEnv *env = JNI::getEnv();
	env->CallVoidMethod(_assetfd, MID_close);

	if (env->ExceptionCheck())
		env->ExceptionClear();

	env->DeleteGlobalRef(_assetfd);
}

uint32 AssetFdReadStream::read(void *dataPtr, uint32 dataSize) {
	if (_declared_len != UNKNOWN_LENGTH) {
		jlong cap = _declared_len - _pos;
		if (dataSize > cap)
			dataSize = cap;
	}

	int ret = ::read(_fd, dataPtr, dataSize);

	if (ret == 0)
		_eos = true;
	else if (ret == -1)
		_err = true;
	else
		_pos += ret;

	return ret;
}

bool AssetFdReadStream::seek(int32 offset, int whence) {
	if (whence == SEEK_SET) {
		if (_declared_len != UNKNOWN_LENGTH && offset > _declared_len)
			offset = _declared_len;

		offset += _start_off;
	} else if (whence == SEEK_END && _declared_len != UNKNOWN_LENGTH) {
		whence = SEEK_SET;
		offset = _start_off + _declared_len + offset;
	}

	int ret = lseek(_fd, offset, whence);

	if (ret == -1)
		return false;

	_pos = ret - _start_off;
	_eos = false;

	return true;
}

AndroidAssetArchive::AndroidAssetArchive(jobject am) {
	JNIEnv *env = JNI::getEnv();
	_am = env->NewGlobalRef(am);

	jclass cls = env->GetObjectClass(_am);
	MID_open = env->GetMethodID(cls, "open",
								"(Ljava/lang/String;I)Ljava/io/InputStream;");
	assert(MID_open);

	MID_openFd = env->GetMethodID(cls, "openFd", "(Ljava/lang/String;)"
								"Landroid/content/res/AssetFileDescriptor;");
	assert(MID_openFd);

	MID_list = env->GetMethodID(cls, "list",
								"(Ljava/lang/String;)[Ljava/lang/String;");
	assert(MID_list);
}

AndroidAssetArchive::~AndroidAssetArchive() {
	JNIEnv *env = JNI::getEnv();
	env->DeleteGlobalRef(_am);
}

bool AndroidAssetArchive::hasFile(const Common::String &name) const {
	JNIEnv *env = JNI::getEnv();
	jstring path = env->NewStringUTF(name.c_str());
	jobject result = env->CallObjectMethod(_am, MID_open, path, ACCESS_UNKNOWN);
	if (env->ExceptionCheck()) {
		// Assume FileNotFoundException
		//warning("Error while calling AssetManager->open(%s)", name.c_str());
		//env->ExceptionDescribe();
		env->ExceptionClear();
		env->DeleteLocalRef(path);

		return false;
	}

	env->DeleteLocalRef(result);
	env->DeleteLocalRef(path);

	return true;
}

int AndroidAssetArchive::listMembers(Common::ArchiveMemberList &member_list) const {
	JNIEnv *env = JNI::getEnv();
	Common::List<Common::String> dirlist;
	dirlist.push_back("");

	int count = 0;
	while (!dirlist.empty()) {
		const Common::String dir = dirlist.back();
		dirlist.pop_back();

		jstring jpath = env->NewStringUTF(dir.c_str());
		jobjectArray jpathlist =
			(jobjectArray)env->CallObjectMethod(_am, MID_list, jpath);

		if (env->ExceptionCheck()) {
			warning("Error while calling AssetManager->list(%s). Ignoring.",
					dir.c_str());
			env->ExceptionDescribe();
			env->ExceptionClear();

			// May as well keep going ...
			continue;
		}

		env->DeleteLocalRef(jpath);

		for (jsize i = 0; i < env->GetArrayLength(jpathlist); ++i) {
			jstring elem = (jstring)env->GetObjectArrayElement(jpathlist, i);
			const char *p = env->GetStringUTFChars(elem, 0);

			if (strlen(p)) {
				Common::String thispath = dir;

				if (!thispath.empty())
					thispath += "/";

				thispath += p;

				// Assume files have a . in them, and directories don't
				if (strchr(p, '.')) {
					member_list.push_back(getMember(thispath));
					++count;
				} else {
					dirlist.push_back(thispath);
				}
			}

			env->ReleaseStringUTFChars(elem, p);
			env->DeleteLocalRef(elem);
		}

		env->DeleteLocalRef(jpathlist);
	}

	return count;
}

const Common::ArchiveMemberPtr AndroidAssetArchive::getMember(const Common::String &name) const {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *AndroidAssetArchive::createReadStreamForMember(const Common::String &path) const {
	JNIEnv *env = JNI::getEnv();
	jstring jpath = env->NewStringUTF(path.c_str());

	// Try openFd() first ...
	jobject afd = env->CallObjectMethod(_am, MID_openFd, jpath);

	if (env->ExceptionCheck())
		env->ExceptionClear();
	else if (afd != 0) {
		// success :)
		env->DeleteLocalRef(jpath);
		return new AssetFdReadStream(env, afd);
	}

	// ... and fallback to normal open() if that doesn't work
	jobject is = env->CallObjectMethod(_am, MID_open, jpath, ACCESS_RANDOM);

	if (env->ExceptionCheck()) {
		// Assume FileNotFoundException
		//warning("Error opening %s", path.c_str());
		//env->ExceptionDescribe();
		env->ExceptionClear();
		env->DeleteLocalRef(jpath);

		return 0;
	}

	return new JavaInputStream(env, is);
}

#endif
