#include "kimeraglobal.h"

QDataStream::ByteOrder sysByteOrder()
{
  int  wordSize;
  bool bigEndian;
  qSysInfo(&wordSize, &bigEndian);
  return bigEndian ? QDataStream::BigEndian : QDataStream::LittleEndian;
}

