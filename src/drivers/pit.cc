#include <drivers/pit.h>

using namespace os;
using namespace os::common;
using namespace os::drivers;
using namespace os::hardwarecommunication;

void printf(char *);

namespace os
{
	namespace drivers
	{
		volatile uint32_t pit_tick_counter = 0;
	}
}

PIT::PIT(InterruptManager *manager)

	: InterruptHandler(manager->HardwareInterruptOffset(), manager),
	  channel0(0x40),
	  channel1(0x41),
	  channel2(0x42),
	  commandPort(0x43),
	  PIC(0x20)
{

	// configure channel 0 once: channel 0, lobyte/hibyte,
	// mode 3 (square wave), binary counting -> 0x36.
	// reload = 1193182 / 100 = 11932 -> IRQ-0 fires at 100 Hz (10 ms tick).
	// 100 Hz keeps multitasking smooth while slashing v86 WASM IRQ
	// overhead 10x vs the 1000 Hz setting.
	commandPort.Write(0x36);
	uint16_t reload = 1193182 / 1000;
	channel0.Write((uint8_t)(reload & 0xff));
	channel0.Write((uint8_t)((reload >> 8) & 0xff));

	pit_tick_counter = 0;
}

PIT::~PIT()
{
}
void PIT::sleep(uint32_t ms)

{

	asm("cli");
	uint32_t start = pit_tick_counter;
	asm("sti");

	while ((pit_tick_counter - start) < ms)
	{
		asm("hlt");
	}
}

uint32_t PIT::readCount()
{

	uint32_t count = 0;

	asm("cli");

	commandPort.Write(0b0000000);

	count = channel0.Read();
	count |= channel0.Read() << 8;

	asm("sti");
	return count;
}

void PIT::setCount(uint32_t count)
{

	asm("cli");

	channel0.Write(count);
	channel0.Write(count >> 8);

	asm("sti");
}

uint32_t PIT::HandleInterrupt(uint32_t esp)
{

	// advance the global wall-clock tick counter
	pit_tick_counter++;

	// send EOI to the PIC
	PIC.Write(0x20);

	return esp;
}
