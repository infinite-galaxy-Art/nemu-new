SECTIONS
{
	. = 0x8048000;
	.text :
	{
		obj/testcase/start.o(.text)
		*(.text)
	}
}
