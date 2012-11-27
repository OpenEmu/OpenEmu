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

#include "common/util.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "audio/mixer_intern.h"
#include "audio/rate.h"
#include "audio/audiostream.h"
#include "audio/timestamp.h"


namespace Audio {

#pragma mark -
#pragma mark --- Channel classes ---
#pragma mark -


/**
 * Channel used by the default Mixer implementation.
 */
class Channel {
public:
	Channel(Mixer *mixer, Mixer::SoundType type, AudioStream *stream, DisposeAfterUse::Flag autofreeStream, bool reverseStereo, int id, bool permanent);
	~Channel();

	/**
	 * Mixes the channel's samples into the given buffer.
	 *
	 * @param data buffer where to mix the data
	 * @param len  number of sample *pairs*. So a value of
	 *             10 means that the buffer contains twice 10 sample, each
	 *             16 bits, for a total of 40 bytes.
	 * @return number of sample pairs processed (which can still be silence!)
	 */
	int mix(int16 *data, uint len);

	/**
	 * Queries whether the channel is still playing or not.
	 */
	bool isFinished() const { return _stream->endOfStream(); }

	/**
	 * Queries whether the channel is a permanent channel.
	 * A permanent channel is not affected by a Mixer::stopAll
	 * call.
	 */
	bool isPermanent() const { return _permanent; }

	/**
	 * Returns the id of the channel.
	 */
	int getId() const { return _id; }

	/**
	 * Pauses or unpaused the channel in a recursive fashion.
	 *
	 * @param paused true, when the channel should be paused.
	 *               false when it should be unpaused.
	 */
	void pause(bool paused);

	/**
	 * Queries whether the channel is currently paused.
	 */
	bool isPaused() const { return (_pauseLevel != 0); }

	/**
	 * Sets the channel's own volume.
	 *
	 * @param volume new volume
	 */
	void setVolume(const byte volume);

	/**
	 * Gets the channel's own volume.
	 *
	 * @return volume
	 */
	byte getVolume();

	/**
	 * Sets the channel's balance setting.
	 *
	 * @param balance new balance
	 */
	void setBalance(const int8 balance);

	/**
	 * Gets the channel's balance setting.
	 *
	 * @return balance
	 */
	int8 getBalance();

	/**
	 * Notifies the channel that the global sound type
	 * volume settings changed.
	 */
	void notifyGlobalVolChange() { updateChannelVolumes(); }

	/**
	 * Queries how long the channel has been playing.
	 */
	Timestamp getElapsedTime();

	/**
	 * Queries the channel's sound type.
	 */
	Mixer::SoundType getType() const { return _type; }

	/**
	 * Sets the channel's sound handle.
	 *
	 * @param handle new handle
	 */
	void setHandle(const SoundHandle handle) { _handle = handle; }

	/**
	 * Queries the channel's sound handle.
	 */
	SoundHandle getHandle() const { return _handle; }

private:
	const Mixer::SoundType _type;
	SoundHandle _handle;
	bool _permanent;
	int _pauseLevel;
	int _id;

	byte _volume;
	int8 _balance;

	void updateChannelVolumes();
	st_volume_t _volL, _volR;

	Mixer *_mixer;

	uint32 _samplesConsumed;
	uint32 _samplesDecoded;
	uint32 _mixerTimeStamp;
	uint32 _pauseStartTime;
	uint32 _pauseTime;

	RateConverter *_converter;
	Common::DisposablePtr<AudioStream> _stream;
};

#pragma mark -
#pragma mark --- Mixer ---
#pragma mark -


MixerImpl::MixerImpl(OSystem *system, uint sampleRate)
	: _syst(system), _mutex(), _sampleRate(sampleRate), _mixerReady(false), _handleSeed(0), _soundTypeSettings() {

	assert(sampleRate > 0);

	for (int i = 0; i != NUM_CHANNELS; i++)
		_channels[i] = 0;
}

MixerImpl::~MixerImpl() {
	for (int i = 0; i != NUM_CHANNELS; i++)
		delete _channels[i];
}

void MixerImpl::setReady(bool ready) {
	_mixerReady = ready;
}

uint MixerImpl::getOutputRate() const {
	return _sampleRate;
}

void MixerImpl::insertChannel(SoundHandle *handle, Channel *chan) {
	int index = -1;
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] == 0) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		warning("MixerImpl::out of mixer slots");
		delete chan;
		return;
	}

	_channels[index] = chan;

	SoundHandle chanHandle;
	chanHandle._val = index + (_handleSeed * NUM_CHANNELS);

	chan->setHandle(chanHandle);
	_handleSeed++;
	if (handle)
		*handle = chanHandle;
}

void MixerImpl::playStream(
			SoundType type,
			SoundHandle *handle,
			AudioStream *stream,
			int id, byte volume, int8 balance,
			DisposeAfterUse::Flag autofreeStream,
			bool permanent,
			bool reverseStereo) {
	Common::StackLock lock(_mutex);

	if (stream == 0) {
		warning("stream is 0");
		return;
	}


	assert(_mixerReady);

	// Prevent duplicate sounds
	if (id != -1) {
		for (int i = 0; i != NUM_CHANNELS; i++)
			if (_channels[i] != 0 && _channels[i]->getId() == id) {
				// Delete the stream if were asked to auto-dispose it.
				// Note: This could cause trouble if the client code does not
				// yet expect the stream to be gone. The primary example to
				// keep in mind here is QueuingAudioStream.
				// Thus, as a quick rule of thumb, you should never, ever,
				// try to play QueuingAudioStreams with a sound id.
				if (autofreeStream == DisposeAfterUse::YES)
					delete stream;
				return;
			}
	}

#ifdef AUDIO_REVERSE_STEREO
	reverseStereo = !reverseStereo;
#endif

	// Create the channel
	Channel *chan = new Channel(this, type, stream, autofreeStream, reverseStereo, id, permanent);
	chan->setVolume(volume);
	chan->setBalance(balance);
	insertChannel(handle, chan);
}

int MixerImpl::mixCallback(byte *samples, uint len) {
	assert(samples);

	Common::StackLock lock(_mutex);

	int16 *buf = (int16 *)samples;
	// we store stereo, 16-bit samples
	assert(len % 4 == 0);
	len >>= 2;

	// Since the mixer callback has been called, the mixer must be ready...
	_mixerReady = true;

	//  zero the buf
	memset(buf, 0, 2 * len * sizeof(int16));

	// mix all channels
	int res = 0, tmp;
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i]) {
			if (_channels[i]->isFinished()) {
				delete _channels[i];
				_channels[i] = 0;
			} else if (!_channels[i]->isPaused()) {
				tmp = _channels[i]->mix(buf, len);

				if (tmp > res)
					res = tmp;
			}
		}

	return res;
}

void MixerImpl::stopAll() {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != 0 && !_channels[i]->isPermanent()) {
			delete _channels[i];
			_channels[i] = 0;
		}
	}
}

void MixerImpl::stopID(int id) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != 0 && _channels[i]->getId() == id) {
			delete _channels[i];
			_channels[i] = 0;
		}
	}
}

void MixerImpl::stopHandle(SoundHandle handle) {
	Common::StackLock lock(_mutex);

	// Simply ignore stop requests for handles of sounds that already terminated
	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->getHandle()._val != handle._val)
		return;

	delete _channels[index];
	_channels[index] = 0;
}

void MixerImpl::muteSoundType(SoundType type, bool mute) {
	assert(0 <= type && type < ARRAYSIZE(_soundTypeSettings));
	_soundTypeSettings[type].mute = mute;

	for (int i = 0; i != NUM_CHANNELS; ++i) {
		if (_channels[i] && _channels[i]->getType() == type)
			_channels[i]->notifyGlobalVolChange();
	}
}

bool MixerImpl::isSoundTypeMuted(SoundType type) const {
	assert(0 <= type && type < ARRAYSIZE(_soundTypeSettings));
	return _soundTypeSettings[type].mute;
}

void MixerImpl::setChannelVolume(SoundHandle handle, byte volume) {
	Common::StackLock lock(_mutex);

	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->getHandle()._val != handle._val)
		return;

	_channels[index]->setVolume(volume);
}

byte MixerImpl::getChannelVolume(SoundHandle handle) {
	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->getHandle()._val != handle._val)
		return 0;

	return _channels[index]->getVolume();
}

void MixerImpl::setChannelBalance(SoundHandle handle, int8 balance) {
	Common::StackLock lock(_mutex);

	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->getHandle()._val != handle._val)
		return;

	_channels[index]->setBalance(balance);
}

int8 MixerImpl::getChannelBalance(SoundHandle handle) {
	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->getHandle()._val != handle._val)
		return 0;

	return _channels[index]->getBalance();
}

uint32 MixerImpl::getSoundElapsedTime(SoundHandle handle) {
	return getElapsedTime(handle).msecs();
}

Timestamp MixerImpl::getElapsedTime(SoundHandle handle) {
	Common::StackLock lock(_mutex);

	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->getHandle()._val != handle._val)
		return Timestamp(0, _sampleRate);

	return _channels[index]->getElapsedTime();
}

void MixerImpl::pauseAll(bool paused) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != 0) {
			_channels[i]->pause(paused);
		}
	}
}

void MixerImpl::pauseID(int id, bool paused) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++) {
		if (_channels[i] != 0 && _channels[i]->getId() == id) {
			_channels[i]->pause(paused);
			return;
		}
	}
}

void MixerImpl::pauseHandle(SoundHandle handle, bool paused) {
	Common::StackLock lock(_mutex);

	// Simply ignore (un)pause requests for sounds that already terminated
	const int index = handle._val % NUM_CHANNELS;
	if (!_channels[index] || _channels[index]->getHandle()._val != handle._val)
		return;

	_channels[index]->pause(paused);
}

bool MixerImpl::isSoundIDActive(int id) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i] && _channels[i]->getId() == id)
			return true;
	return false;
}

int MixerImpl::getSoundID(SoundHandle handle) {
	Common::StackLock lock(_mutex);
	const int index = handle._val % NUM_CHANNELS;
	if (_channels[index] && _channels[index]->getHandle()._val == handle._val)
		return _channels[index]->getId();
	return 0;
}

bool MixerImpl::isSoundHandleActive(SoundHandle handle) {
	Common::StackLock lock(_mutex);
	const int index = handle._val % NUM_CHANNELS;
	return _channels[index] && _channels[index]->getHandle()._val == handle._val;
}

bool MixerImpl::hasActiveChannelOfType(SoundType type) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i != NUM_CHANNELS; i++)
		if (_channels[i] && _channels[i]->getType() == type)
			return true;
	return false;
}

void MixerImpl::setVolumeForSoundType(SoundType type, int volume) {
	assert(0 <= type && type < ARRAYSIZE(_soundTypeSettings));

	// Check range
	if (volume > kMaxMixerVolume)
		volume = kMaxMixerVolume;
	else if (volume < 0)
		volume = 0;

	// TODO: Maybe we should do logarithmic (not linear) volume
	// scaling? See also Player_V2::setMasterVolume

	Common::StackLock lock(_mutex);
	_soundTypeSettings[type].volume = volume;

	for (int i = 0; i != NUM_CHANNELS; ++i) {
		if (_channels[i] && _channels[i]->getType() == type)
			_channels[i]->notifyGlobalVolChange();
	}
}

int MixerImpl::getVolumeForSoundType(SoundType type) const {
	assert(0 <= type && type < ARRAYSIZE(_soundTypeSettings));

	return _soundTypeSettings[type].volume;
}


#pragma mark -
#pragma mark --- Channel implementations ---
#pragma mark -

Channel::Channel(Mixer *mixer, Mixer::SoundType type, AudioStream *stream,
                 DisposeAfterUse::Flag autofreeStream, bool reverseStereo, int id, bool permanent)
    : _type(type), _mixer(mixer), _id(id), _permanent(permanent), _volume(Mixer::kMaxChannelVolume),
      _balance(0), _pauseLevel(0), _samplesConsumed(0), _samplesDecoded(0), _mixerTimeStamp(0),
      _pauseStartTime(0), _pauseTime(0), _converter(0),
      _stream(stream, autofreeStream) {
	assert(mixer);
	assert(stream);

	// Get a rate converter instance
	_converter = makeRateConverter(_stream->getRate(), mixer->getOutputRate(), _stream->isStereo(), reverseStereo);
}

Channel::~Channel() {
	delete _converter;
}

void Channel::setVolume(const byte volume) {
	_volume = volume;
	updateChannelVolumes();
}

byte Channel::getVolume() {
	return _volume;
}

void Channel::setBalance(const int8 balance) {
	_balance = balance;
	updateChannelVolumes();
}

int8 Channel::getBalance() {
	return _balance;
}

void Channel::updateChannelVolumes() {
	// From the channel balance/volume and the global volume, we compute
	// the effective volume for the left and right channel. Note the
	// slightly odd divisor: the 255 reflects the fact that the maximal
	// value for _volume is 255, while the 127 is there because the
	// balance value ranges from -127 to 127.  The mixer (music/sound)
	// volume is in the range 0 - kMaxMixerVolume.
	// Hence, the vol_l/vol_r values will be in that range, too

	if (!_mixer->isSoundTypeMuted(_type)) {
		int vol = _mixer->getVolumeForSoundType(_type) * _volume;

		if (_balance == 0) {
			_volL = vol / Mixer::kMaxChannelVolume;
			_volR = vol / Mixer::kMaxChannelVolume;
		} else if (_balance < 0) {
			_volL = vol / Mixer::kMaxChannelVolume;
			_volR = ((127 + _balance) * vol) / (Mixer::kMaxChannelVolume * 127);
		} else {
			_volL = ((127 - _balance) * vol) / (Mixer::kMaxChannelVolume * 127);
			_volR = vol / Mixer::kMaxChannelVolume;
		}
	} else {
		_volL = _volR = 0;
	}
}

void Channel::pause(bool paused) {
	//assert((paused && _pauseLevel >= 0) || (!paused && _pauseLevel));

	if (paused) {
		_pauseLevel++;

		if (_pauseLevel == 1)
			_pauseStartTime = g_system->getMillis();
	} else if (_pauseLevel > 0) {
		_pauseLevel--;

		if (!_pauseLevel) {
			_pauseTime = (g_system->getMillis() - _pauseStartTime);
			_pauseStartTime = 0;
		}
	}
}

Timestamp Channel::getElapsedTime() {
	const uint32 rate = _mixer->getOutputRate();
	uint32 delta = 0;

	Audio::Timestamp ts(0, rate);

	if (_mixerTimeStamp == 0)
		return ts;

	if (isPaused())
		delta = _pauseStartTime - _mixerTimeStamp;
	else
		delta = g_system->getMillis() - _mixerTimeStamp - _pauseTime;

	// Convert the number of samples into a time duration.

	ts = ts.addFrames(_samplesConsumed);
	ts = ts.addMsecs(delta);

	// In theory it would seem like a good idea to limit the approximation
	// so that it never exceeds the theoretical upper bound set by
	// _samplesDecoded. Meanwhile, back in the real world, doing so makes
	// the Broken Sword cutscenes noticeably jerkier. I guess the mixer
	// isn't invoked at the regular intervals that I first imagined.

	return ts;
}

int Channel::mix(int16 *data, uint len) {
	assert(_stream);

	int res = 0;

	if (_stream->endOfData()) {
		// TODO: call drain method
	} else {
		assert(_converter);
		_samplesConsumed = _samplesDecoded;
		_mixerTimeStamp = g_system->getMillis();
		_pauseTime = 0;
		res = _converter->flow(*_stream, data, len, _volL, _volR);
		_samplesDecoded += res;
	}

	return res;
}

} // End of namespace Audio
