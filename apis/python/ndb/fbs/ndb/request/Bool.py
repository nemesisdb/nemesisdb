# automatically generated by the FlatBuffers compiler, do not modify

# namespace: request

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Bool(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Bool()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsBool(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # Bool
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Bool
    def Val(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return bool(self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos))
        return False

def BoolStart(builder):
    builder.StartObject(1)

def Start(builder):
    BoolStart(builder)

def BoolAddVal(builder, val):
    builder.PrependBoolSlot(0, val, 0)

def AddVal(builder, val):
    BoolAddVal(builder, val)

def BoolEnd(builder):
    return builder.EndObject()

def End(builder):
    return BoolEnd(builder)
