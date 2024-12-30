# automatically generated by the FlatBuffers compiler, do not modify

# namespace: request

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Request(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Request()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsRequest(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    @classmethod
    def RequestBufferHasIdentifier(cls, buf, offset, size_prefixed=False):
        return flatbuffers.util.BufferHasIdentifier(buf, offset, b"\x4B\x56\x20\x20", size_prefixed=size_prefixed)

    # Request
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Request
    def Kv(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from ndb.request.KV import KV
            obj = KV()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # Request
    def KvLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # Request
    def KvIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

def RequestStart(builder):
    builder.StartObject(1)

def Start(builder):
    RequestStart(builder)

def RequestAddKv(builder, kv):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(kv), 0)

def AddKv(builder, kv):
    RequestAddKv(builder, kv)

def RequestStartKvVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartKvVector(builder, numElems):
    return RequestStartKvVector(builder, numElems)

def RequestEnd(builder):
    return builder.EndObject()

def End(builder):
    return RequestEnd(builder)
