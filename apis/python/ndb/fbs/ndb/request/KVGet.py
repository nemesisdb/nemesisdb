# automatically generated by the FlatBuffers compiler, do not modify

# namespace: request

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class KVGet(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = KVGet()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsKVGet(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # KVGet
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # KVGet
    def Keys(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.String(a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return ""

    # KVGet
    def KeysLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # KVGet
    def KeysIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

def KVGetStart(builder):
    builder.StartObject(1)

def Start(builder):
    KVGetStart(builder)

def KVGetAddKeys(builder, keys):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(keys), 0)

def AddKeys(builder, keys):
    KVGetAddKeys(builder, keys)

def KVGetStartKeysVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartKeysVector(builder, numElems):
    return KVGetStartKeysVector(builder, numElems)

def KVGetEnd(builder):
    return builder.EndObject()

def End(builder):
    return KVGetEnd(builder)