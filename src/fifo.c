#include "fifo.h"

void FifoInit(FIFO *buf)
{
	buf->Head = 0x00;
	buf->Tail = 0x00;
}

void Push(FIFO *buf, TypeFifo data)
{
	if (GetSize(buf) == 0xFF)
	{
		buf->Head = 0x00;
		buf->Data[buf->Head++] = data;
	}
	else
	{
		buf->Data[buf->Head++] = data;
	}
}

TypeFifo Pull(FIFO *buf)
{
	TypeFifo data = buf->Data[buf->Tail++];
	if(buf->Head == buf->Tail){
		 Clear(buf);
	}
	return data;
}

uint16_t GetSize(FIFO *buf)
{
	if (buf->Head > buf->Tail)
	{
		return buf->Head - buf->Tail;
	}
	else
	{
		return 0;
	}
}


void Clear(FIFO *buf)
{
	buf->Head = 0x00;
	buf->Tail = 0x00;
}