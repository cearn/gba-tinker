
#include <tonc.h>

void test_speeds();

int main()
{
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);

	test_speeds();

	while(1)
		VBlankIntrWait();
}

// EOF
