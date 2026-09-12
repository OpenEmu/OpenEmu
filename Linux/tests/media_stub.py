"""Keep desktop unit tests independent of host audio services.

Real device/audio-stream validation runs separately in the live smoke check.
"""
from PySide6.QtCore import QObject, Signal


class NoAudioDevice:
    def isNull(self): return True


class TestMediaDevices(QObject):
    audioOutputsChanged = Signal()

    @staticmethod
    def audioOutputs(): return []

    @staticmethod
    def defaultAudioOutput(): return NoAudioDevice()


def isolate_audio():
    from openemu_linux import player, settings
    player.QMediaDevices = TestMediaDevices
    settings.QMediaDevices = TestMediaDevices
